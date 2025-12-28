#include "deadlineparser.h"
#include <QJsonDocument>
#include <QRegularExpression>
#include <QDebug>

/* ================= FSM ================= */

ParserStateMachine::ParserStateMachine()
    : m_state(ParserState::INIT)
{
    m_graph[ParserState::INIT][UserAction::RAW_TXT] = ParserState::FILTER_TEXT;
    m_graph[ParserState::FILTER_TEXT][UserAction::VALID] = ParserState::SPLIT_TEXT;
    m_graph[ParserState::SPLIT_TEXT][UserAction::VALID] = ParserState::JSON_TEXT;

    // VALID JSON → FINISH
    m_graph[ParserState::JSON_TEXT][UserAction::JSON_TXT] = ParserState::FINISH;

    // Direct JSON input
    m_graph[ParserState::INIT][UserAction::JSON_TXT] = ParserState::FINISH;

    // Error paths
    m_graph[ParserState::FILTER_TEXT][UserAction::INVALID] = ParserState::PROMPT_USER;
    m_graph[ParserState::SPLIT_TEXT][UserAction::INVALID] = ParserState::PROMPT_USER;
}

void ParserStateMachine::update(UserAction action)
{
    if (m_graph[m_state].contains(action))
        m_state = m_graph[m_state][action];
    else
        qWarning() << "[FSM] Invalid transition";
}

ParserState ParserStateMachine::state() const
{
    return m_state;
}

/* ================= Parser ================= */

QDate DeadlineParser::parseDate(const QString& dateString)
{
    QString cleanDate = dateString.trimmed();
    if (cleanDate.endsWith(',')) cleanDate.chop(1);

    static const QStringList formats = {
        "MM/dd/yyyy", "MM/dd/yy",
        "MMMM d yyyy", "MMMM d, yyyy",
        "d MMMM yyyy", "MMM d, yyyy"
    };

    for (const auto& fmt : formats) {
        QDate d = QDate::fromString(cleanDate, fmt);
        if (d.isValid()) return d;
    }
    return {};
}

QString DeadlineParser::filterLinesWithDates(const QString& text,
                                             const QRegularExpression& pattern)
{
    QStringList lines = text.split('\n');
    QStringList result;

    for (const auto& line : lines) {
        if (pattern.match(line).hasMatch())
            result << line.trimmed();
    }
    return result.join('\n');
}

QStringList DeadlineParser::splitTextByDate(const QString& text,
                                            const QRegularExpression& pattern)
{
    QString normalized = text;

    // Normalize whitespace + NBSP
    normalized.replace(QChar(0x00A0), ' ');
    normalized.replace(QRegularExpression("\\s+"), " ");

    QStringList result;
    int last = 0;

    auto it = pattern.globalMatch(normalized);
    while (it.hasNext()) {
        auto match = it.next();
        if (match.capturedStart() > last)
            result << normalized.mid(last, match.capturedStart() - last).trimmed();
        result << match.captured().trimmed();
        last = match.capturedEnd();
    }

    if (last < normalized.length())
        result << normalized.mid(last).trimmed();

    result.removeAll("");
    return result;
}

QJsonArray DeadlineParser::serializeText(const QStringList& parts)
{
    QJsonArray array;

    for (int i = 0; i + 1 < parts.size(); ++i) {
        QDate d1 = parseDate(parts[i]);
        QDate d2 = parseDate(parts[i + 1]);

        if (d1.isValid()) {
            array.append(QJsonObject{
                {"date", d1.toString(Qt::ISODate)},
                {"event", parts[i + 1]}
            });
            ++i;
        }
        else if (d2.isValid()) {
            array.append(QJsonObject{
                {"date", d2.toString(Qt::ISODate)},
                {"event", parts[i]}
            });
            ++i;
        }
    }
    return array;
}

/* ================= Action Resolver ================= */

UserAction DeadlineParser::getAction(const QString& text, ParserState state)
{
    QJsonParseError err;
    QJsonDocument::fromJson(text.toUtf8(), &err);
    if (err.error == QJsonParseError::NoError)
        return UserAction::JSON_TXT;

    if (state == ParserState::INIT)
        return UserAction::RAW_TXT;

    if (state == ParserState::FILTER_TEXT)
        return text.length() > 1 ? UserAction::VALID : UserAction::INVALID;

    return UserAction::INVALID;
}

UserAction DeadlineParser::getAction(const QStringList& text, ParserState state)
{
    if (state == ParserState::SPLIT_TEXT)
        return (text.size() % 2 == 0) ? UserAction::VALID : UserAction::INVALID;
    return UserAction::INVALID;
}

/* ================= Public API ================= */

QPair<QString, ParserState> DeadlineParser::parseDeadlines(const QString& input)
{
    QString text = input;
    QString original = input;

    // Normalize NBSP early
    text.replace(QChar(0x00A0), ' ');

    ParserStateMachine fsm;

    // ✅ FIXED REGEX (correct precedence)
    QRegularExpression datePattern(
        R"(\b(?:\d{1,2}/\d{1,2}/\d{2,4}|\d{1,2}\s\w+\s\d{4}|\w+\s\d{1,2},?\s\d{4})\b)"
        );

    while (fsm.state() != ParserState::FINISH) {

        if (fsm.state() == ParserState::SPLIT_TEXT) {
            QStringList parts = splitTextByDate(text, datePattern);
            fsm.update(getAction(parts, fsm.state()));

            if (fsm.state() == ParserState::JSON_TEXT) {
                QJsonArray arr = serializeText(parts);
                text = QJsonDocument(arr).toJson(QJsonDocument::Compact);
                fsm.update(UserAction::JSON_TXT);
            }
        }
        else {
            fsm.update(getAction(text, fsm.state()));
        }

        switch (fsm.state()) {
        case ParserState::FILTER_TEXT:
            text = filterLinesWithDates(text, datePattern);
            break;

        case ParserState::PROMPT_USER:
            return { "[INVALID TEXT]\n\n" + original, fsm.state() };

        case ParserState::FINISH:
            return { text, fsm.state() };

        default:
            break;
        }
    }

    return { text, fsm.state() };
}

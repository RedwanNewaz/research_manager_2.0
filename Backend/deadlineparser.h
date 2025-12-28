#ifndef DEADLINEPARSER_H
#define DEADLINEPARSER_H
#include <QString>
#include <QStringList>
#include <QJsonArray>
#include <QJsonObject>
#include <QDate>
#include <QHash>

enum class ParserState {
    INIT,
    FILTER_TEXT,
    SPLIT_TEXT,
    JSON_TEXT,
    PROMPT_USER,
    FINISH
};

enum class UserAction {
    RAW_TXT,
    SPLIT_TXT,
    JSON_TXT,
    EMPTY_TXT,
    INVALID,
    VALID
};

class ParserStateMachine
{
public:
    ParserStateMachine();

    void update(UserAction action);
    ParserState state() const;

private:
    ParserState m_state;
    QHash<ParserState, QHash<UserAction, ParserState>> m_graph;
};

class DeadlineParser
{
public:
    static QPair<QString, ParserState> parseDeadlines(const QString& text);

private:
    static QDate parseDate(const QString& dateString);
    static QString filterLinesWithDates(const QString& text, const QRegularExpression& pattern);
    static QStringList splitTextByDate(const QString& text, const QRegularExpression& pattern);
    static QJsonArray serializeText(const QStringList& parts);
    static UserAction getAction(const QString& text, ParserState state);
    static UserAction getAction(const QStringList& text, ParserState state);
};


#endif // DEADLINEPARSER_H

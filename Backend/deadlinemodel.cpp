#include "deadlinemodel.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
using namespace project;
DeadlineModel::DeadlineModel(DbmPtr db, QObject *parent)
    : QAbstractTableModel{parent}, db_(db)
{
    m_projectId = -1;
}

void DeadlineModel::projectIdChanged(int id)
{
    // qInfo() << "[DeadlineModel] projectIdChanged " << id;
    m_projectId = id;
    emit layoutChanged();
}

void DeadlineModel::deleteRow(int id)
{
    if(!event_map_.contains(id))
        return;

    int id_ = event_map_[id].id;
    QString sqlCmd= QString("DELETE FROM calendars WHERE id = %1").arg(id_);
    if(db_->updateDB(sqlCmd))
    {
        qInfo() << "[DeadlineModel] success ";
    }
    else{
        qInfo() << sqlCmd;
    }
    emit layoutChanged();
}

QString DeadlineModel::getEventCountdown(int indx)
{
    if(!event_map_.contains(indx))
        return "event not found";
    auto event = event_map_[indx];

    QDate currentDate = QDate::currentDate();
    auto eventDate = event.date;
    int daysDifference = currentDate.daysTo(eventDate);

    if (daysDifference < 0)
        return  QString("[Event] %1 happened %2 days ago")
                .arg(event.name)
                .arg(abs(daysDifference));

    return QString("[Event] %1 in %2 days")
        .arg(event.name)
        .arg(daysDifference);

}

int DeadlineModel::rowCount(const QModelIndex &parent) const
{
    if(m_projectId < 0) return 0;

    QString sqlCmd= QString("SELECT id, timestamp, event FROM calendars WHERE project_id = %1").arg(m_projectId);
    auto results = db_->queryRow(sqlCmd);
    event_map_.clear();
    int index = 0;
    for(int i =0; i < results.size(); i+=3)
    {
        int j = i + 1;
        int k = i + 2;

        EventData event;
        event.index = index;
        event.id = results[i].toInt();
        event.date = QDate::fromString(results[j], Qt::ISODate);
        event.name = results[k];
        event_map_[index] = event;

        index++;
    }
    // qInfo() << "[DeadlineModel] data " << data_;
    return event_map_.size();
}

int DeadlineModel::columnCount(const QModelIndex &parent) const
{
    return (event_map_.isEmpty()) ? 0 : 2;
}

QVariant DeadlineModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || m_projectId < 0)
        return QVariant();

    int row = index.row();
    int col = index.column();
    if(role != Qt::DisplayRole || !event_map_.contains(row))
        return QVariant();

    auto event = event_map_[row];

    switch (col) {
    case 1:
        return event.name;
    default:
        return event.date.toString("MM/dd/yyyy");
    }


    return QVariant();
}

QHash<int, QByteArray> DeadlineModel::roleNames() const
{
    return{
        {Qt::DisplayRole, "display"}
    };
}

QString DeadlineModel::deadlineTxt() const
{
    return m_deadlineTxt;
}

void DeadlineModel::setDeadlineTxt(const QString &newDeadlineTxt)
{
    if (m_deadlineTxt == newDeadlineTxt || newDeadlineTxt.isEmpty())
        return;
    auto result = DeadlineParser::parseDeadlines(newDeadlineTxt);
    m_deadlineTxt = result.first;


    if (result.second != ParserState::PROMPT_USER)
    {
        qInfo() << "[DeadlineModel] inserting to database " << result.first;
        QJsonParseError error;
        QJsonDocument doc =
            QJsonDocument::fromJson(result.first.toUtf8(), &error);

        if (error.error != QJsonParseError::NoError) {
            qWarning() << "JSON parse error:" << error.errorString();
            return;
        }

        if (!doc.isArray()) {
            qWarning() << "Expected JSON array";
            return;
        }

        QJsonArray array = doc.array();
        for (const QJsonValue &val : array) {
            QJsonObject obj = val.toObject();
            QString dateTxt  = obj["date"].toString();
            QString event = obj["event"].toString();

            QDate date = QDate::fromString(dateTxt, Qt::ISODate);
            if (!date.isValid()) {
                qWarning() << "[DeadlineModel] Invalid date:" << dateTxt;
                continue;
            }
            QString isoString = date.toString(Qt::ISODate);

            qDebug() << "[DeadlineModel]: Date:" << isoString << "Event:" << event;
            QString sqlCmd = QString("INSERT INTO calendars (timestamp, event, project_id) VALUES ('%1', '%2', %3)")
                             .arg(isoString)
                             .arg(event)
                                 .arg(m_projectId);

            db_->updateDB(sqlCmd);
        }
        m_deadlineTxt = "";
    }

    emit deadlineTxtChanged();
    emit layoutChanged();
}

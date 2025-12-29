#include "taskmanger.h"
#include <QDateTime>
namespace project{

TaskManger::TaskManger(DbmPtr db, QObject *parent): db_(db)
{
    m_projectId = -1;
}

int TaskManger::rowCount(const QModelIndex &parent) const
{
    if(m_projectId < 0) return 0;


    QString sqlCmd= QString("SELECT id, title FROM tasks WHERE project_id = %1 ORDER BY timestamp DESC").arg(m_projectId);
    auto id_data = db_->queryRow(sqlCmd);

    record_map_.clear();
    int index = 0;
    for(int i = 0; i < id_data.size(); i+=2)
    {
        int j = i + 1;
        TaskRecord record;
        record.index = index;
        record.id = id_data.at(i).toInt();
        record.data = id_data.at(j);
        record.checked = false;
        record_map_[index] = record;
        ++index;
    }
    return record_map_.size();

}

QVariant TaskManger::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    // qInfo() << " data row " << row;
    if (!index.isValid() || m_projectId < 0)
        return QVariant();
    if(record_map_.find(row) == record_map_.end())
        return QVariant();
    if(role == CheckBoxRole)
        return record_map_[row].checked;
    return record_map_[row].data;
}

QHash<int, QByteArray> TaskManger::roleNames() const
{
    return {
        { TitleRole,    "title" },
        { TimestampRole,"time" },
        { CheckBoxRole, "checked"}
    };
}

void TaskManger::addTask(const QString &text)
{
    // qInfo() << "[TaskManger]: addTask " << text;
    // auto escapeAndQuote= [](const QString &data) {
    //     // 1. Escape: Replace single quotes with two single quotes for SQL safety
    //     QString escaped = data;
    //     escaped.replace("'", "''");

    //     // 2. Quote: Enclose the escaped string in single quotes
    //     return QString("'%1'").arg(escaped);
    // };


    // --- 1. Prepare Data for Insertion ---

    // a. Title (Quoted and escaped)
    // QString safeTitle = escapeAndQuote(text);

    // b. Description (NULL/empty string, quoted)
    // QString safeDescription = "''"; // Represents an empty string for the TEXT column

    // c. Timestamp (Format as ISO string, quoted)
    // The format must be compatible with the DATETIME column type.
    QString timestampStr = QDateTime::currentDateTime().toString(Qt::ISODate);

    // d. Pending and Project ID (Integers, no quotes needed)
    int pending = 1;
    int projectId = m_projectId; // Assuming m_projectId is a valid integer member variable

    // --- 2. Construct the SQL Command using QString::arg() ---

    // QString sqlCmd = QString(
    //                      "INSERT INTO tasks (title, description, timestamp, pending, project_id) "
    //                      "VALUES (%1, %2, %3, %4, %5)"
    //                      )
    //                      .arg(safeTitle)         // %1: title (quoted and escaped)
    //                      .arg(safeDescription)   // %2: description (quoted and escaped string)
    //                      .arg(timestampStr)      // %3: timestamp (quoted ISO date string)
    //                      .arg(pending)           // %4: pending (integer)
    //                      .arg(projectId);        // %5: project_id (integer)

    // if(db_->updateDB(sqlCmd))
    // {
    //     qInfo() << "[TaskManger] success ";
    // }
    // else{
    //     qInfo() << sqlCmd;
    // }

    auto query = db_->getBinder(
        "INSERT INTO tasks (title, description, timestamp, pending, project_id) "
        "VALUES (:title, :desc, :time, :pending, :pid)"
        );

    query.bindValue(":title", text);
    query.bindValue(":desc", text);
    query.bindValue(":time", timestampStr);
    query.bindValue(":pending", pending);
    query.bindValue(":pid", projectId);
    query.exec();


    emit layoutChanged();
}

void TaskManger::editTask(int index, const QString& title, const QString& description)
{
    if(record_map_.find(index) == record_map_.end()) return;



    // 1. Prepare the statement with placeholders
    auto query = db_->getBinder("UPDATE tasks SET title = :title, description = :desc WHERE id = :id");

    // 2. Bind the actual values
    query.bindValue(":title", title);
    query.bindValue(":desc", description);
    query.bindValue(":id", record_map_[index].id);

    // 3. Execute the query
    if(query.exec())
    {
        setTaskDescription(description);
        qInfo() << "[TaskManger] editTask success to update database " << title;
    }
    else
    {
        qWarning() << "[TaskManger] editTask failed: " << query.lastError().text();
    }

    emit layoutChanged();
}

void TaskManger::deleteTasks()
{
    for(const auto& it: record_map_)
    {
        auto record = it.second;
        if(!record.checked)
            continue;
        qInfo() << "[TaskManger]: deleteTask " << record.index;
        QString sqlCmd = QString("DELETE FROM tasks WHERE id = %1").arg(record.id);
        db_->deleteItem(sqlCmd);
    }

    emit layoutChanged();
}

void TaskManger::updateCheckedBox(int index, bool value)
{
    record_map_[index].checked = value;
    for(const auto& it: record_map_)
        if(it.second.checked)
            qInfo() << "[TaskManger]: checked box index = " << it.second.index;
}

void TaskManger::projectIdChanged(int id)
{
    m_projectId = id;
    // qInfo() << QString("project id updated = %1").arg(id);
    emit layoutChanged();
}

int TaskManger::taskIndex() const
{
    return m_taskIndex;
}

void TaskManger::setTaskIndex(int newTaskIndex)
{
    if (record_map_.find(newTaskIndex) == record_map_.end())
        return;
    m_taskIndex = newTaskIndex;

    qInfo() << "[TaskManger] set task index " << m_taskIndex << " record id = " << record_map_[m_taskIndex].id;

    // read project description and update it
    setTaskTitle(record_map_[m_taskIndex].data);
    QString sqlCmd= QString("SELECT description FROM tasks WHERE id = %1").arg(record_map_[m_taskIndex].id);
    for(const auto& des: db_->queryRow(sqlCmd))
    {

        setTaskDescription(des);
    }

    emit taskIndexChanged();
}

QString TaskManger::taskDescription() const
{
    return m_taskDescription;
}

void TaskManger::setTaskDescription(const QString &newTaskDescription)
{
    if (m_taskDescription == newTaskDescription)
        return;
    m_taskDescription = newTaskDescription;
    emit taskDescriptionChanged();
    qInfo() << "[TaskManger] des " << newTaskDescription;
}

QString TaskManger::taskTitle() const
{
    return m_taskTitle;
}

void TaskManger::setTaskTitle(const QString &newTaskTitle)
{
    if (m_taskTitle == newTaskTitle)
        return;
    m_taskTitle = newTaskTitle;
    emit taskTitleChanged();
}

}

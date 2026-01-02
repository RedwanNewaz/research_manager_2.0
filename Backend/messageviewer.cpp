#include "messageviewer.h"

namespace collab {
MessageViewer::MessageViewer(DbmPtr db, QObject *parent)
    : QAbstractListModel{parent}, db_(db)
{}

int MessageViewer::rowCount(const QModelIndex &parent) const
{
    return task_map_.size();
}

QVariant MessageViewer::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if(m_projectID < 0 || !index.isValid() || !task_map_.contains(row))
        return QVariant();

    auto msg = task_map_[row];
    switch (role) {
    case Subject:
        return msg.title;
    case Description:
        return msg.desc;

    default:
        return msg.tag;
    }

}

QHash<int, QByteArray> MessageViewer::roleNames() const
{
    return {
        {Subject, "subject"},
        {Tag, "tag"},
        {Description, "desc"}
    };
}

QString MessageViewer::currentName() const
{
    return m_currentName;
}

void MessageViewer::setCurrentName(const QString &newCurrentName)
{
    if (m_currentName == newCurrentName)
        return;
    m_currentName = newCurrentName;
    //query from database
    auto sqlCmd = QString(R"(
        SELECT title, description, tag_name FROM collaborators AS c
        INNER JOIN tasks AS t ON c.project_id = t.project_id
        WHERE c.name = '%1' AND c.project_id = %2
          AND t.title LIKE  c.tag_name || '%' ;
    )").arg(newCurrentName)
        .arg(m_projectID);

    auto results = db_->queryRow(sqlCmd);

    int index = 0;
    task_map_.clear();
    for(int i = 0; i < results.size(); i += 3)
    {
        int j = i + 1;
        int k = i + 2;
        MsgData msg;
        msg.title = results[i];
        msg.desc = results[j];

        qInfo() << "[MessageViewer]: " << msg.title;
        msg.tag = results[k];
        task_map_[index] = msg;
        ++index;
    }

    emit currentNameChanged();
    emit layoutChanged();
}

void MessageViewer::projectIdChanged(int newId)
{
    m_projectID = newId;
}


}

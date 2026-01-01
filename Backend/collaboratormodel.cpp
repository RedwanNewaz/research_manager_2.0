#include "collaboratormodel.h"
using namespace project;

CollaboratorModel::CollaboratorModel(DbmPtr db,QObject *parent)
    : QAbstractListModel{parent}, db_(db)
{
    m_projectID = -1;
}

void CollaboratorModel::projectIdChanged(int newId)
{
    m_projectID = newId;
    qInfo() << "[CollaboratorModel] m_projectID = " << m_projectID;
    
    // Clear the old data when switching projects
    beginResetModel();
    col_map_.clear();
    task_map_.clear();
    setMsgDescription("");
    setCurrentName("");
    setCurrentTag("");
    setMsgComboList(QVariant());
    endResetModel();
}

int CollaboratorModel::rowCount(const QModelIndex &parent) const
{
    if(m_projectID < 0)
        return 0;
    // create a table if it does not exist with following columns
    // id, name, tag_name, project_id

    auto tableStr = QString(R"(
        CREATE TABLE IF NOT EXISTS "collaborators" (
            "id" INTEGER PRIMARY KEY AUTOINCREMENT,
            "name" TEXT NOT NULL,
            "tag_name" TEXT,
            "photo" TEXT,
            "project_id" INTEGER NOT NULL,
            UNIQUE("name", "project_id"),
            FOREIGN KEY("project_id") REFERENCES "projects"("id") ON DELETE CASCADE
        )
    )");
    db_->createTable(tableStr);

    // Clear existing data before repopulating
    col_map_.clear();

    
    auto query = QString("SELECT id, name, tag_name, photo FROM collaborators WHERE project_id = %1").arg(m_projectID);
    auto results = db_->queryRow(query);

    int index = 0;
    for(int i = 0; i < results.size(); i += 4)
    {
        int j = i + 1;
        int k = i + 2;
        int l = i + 3;
        ColData col;
        col.id = results[i].toInt();
        col.name = results[j];
        col.tag_name = results[k];
        col.photo = results[l];
        col_map_[index] = col;
        ++index;
    }

    return col_map_.size();
}

QVariant CollaboratorModel::data(const QModelIndex &index, int role) const
{

    int row = index.row();
    if(m_projectID < 0 || !index.isValid() || !col_map_.contains(row))
        return QVariant();

    auto col = col_map_[row];
    switch(role)
    {
        case Name: return col.name;
        case Tag: return col.tag_name;
        case Photo: return col.photo;
    }

    return QVariant();
}

void CollaboratorModel::addCollaborator(const QString &name, const QString& photo)
{
    qInfo() << QString("[CollaboratorModel] %1 added to project = %2")
                   .arg(name)
                   .arg(m_projectID);


    // Check if collaborator already exists in this project (database check)
    auto checkQuery = db_->getBinder("SELECT COUNT(*) FROM collaborators WHERE name = :name AND project_id = :id");
    checkQuery.bindValue(":name", name);
    checkQuery.bindValue(":id", m_projectID);
    if(checkQuery.exec() && checkQuery.next())
    {
        int count = checkQuery.value(0).toInt();
        if(count > 0)
        {
            qWarning() << "[CollaboratorModel] collaborator already exists in this project:" << name;
            return;
        }
    }

    auto tagName = QString("[%1]");
    QString initials;
    for(const auto& part: name.split(" "))
    {
        if(part.isEmpty())
            continue;
        initials += part[0].toUpper();
    }
    tagName = tagName.arg(initials);

    auto query = db_->getBinder("INSERT OR IGNORE INTO collaborators (name, tag_name, photo, project_id)"
                                "VALUES (:name, :tag, :photo, :id)"
                                );
    query.bindValue(":name", name);
    query.bindValue(":tag", tagName);
    query.bindValue(":photo", photo);
    query.bindValue(":id", m_projectID);
    if(!query.exec())
    {
        qWarning() << "[CollaboratorModel] " << query.lastError();
        return;
    }

    // Check if the insert actually added a row
    if(query.numRowsAffected() == 0)
    {
        qWarning() << "[CollaboratorModel] collaborator already exists (ignored):" << name;
        return;
    }

    emit layoutChanged();

}

void CollaboratorModel::deleteCollaborator(int index)
{
    if(!col_map_.contains(index))
        return;
    
    beginRemoveRows(QModelIndex(), index, index);
    auto collaboratorId = col_map_[index].id;
    auto query = QString("DELETE FROM collaborators WHERE id = %1").arg(collaboratorId);
    db_->deleteItem(query);
    
    // Remove from map and reindex remaining items
    col_map_.remove(index);
    QMap<int, ColData> newMap;
    int newIndex = 0;
    for (auto it = col_map_.begin(); it != col_map_.end(); ++it) {
        newMap[newIndex++] = it.value();
    }
    col_map_ = newMap;
    
    endRemoveRows();
}

void CollaboratorModel::updateTagName(int index, const QString &tag)
{
    if(!col_map_.contains(index))
        return;
    auto col = col_map_[index];
    auto query = db_->getBinder("UPDATE collaborators SET tag_name = :tag WHERE id = :id");
    query.bindValue(":tag", tag);
    query.bindValue(":id", col.id);
    query.exec();
    col_map_[index].tag_name = tag;
}

QStringList CollaboratorModel::getTaskTitles() const
{
    QStringList titles;
    for(int i = 0; i < task_map_.size(); ++i) {
        if(task_map_.contains(i)) {
            titles.append(task_map_[i].title);
        }
    }
    qInfo() << "[titles] " << titles;
    qInfo() << task_map_.size();
    return titles;
}

void CollaboratorModel::setTaskDescription(int index)
{
    if(!task_map_.contains(index))
        return;
    setMsgDescription(task_map_[index].desc);
}



QHash<int, QByteArray> CollaboratorModel::roleNames() const
{
    return {
        {Name, "name"},
        {Tag, "tag"},
        {Photo, "photo"}
    };
}

QString CollaboratorModel::currentName() const
{
    return m_currentName;
}

void CollaboratorModel::setCurrentName(const QString &newCurrentName)
{
    if (m_currentName == newCurrentName)
        return;
    m_currentName = newCurrentName;

    int selectedIndex = -1;
    for (auto const& [key, data] : col_map_.asKeyValueRange()) {
        if(data.name == newCurrentName)
        {
            selectedIndex = key;
            break;
        }
    }

    qInfo() << "[CollaboratorModel] selected Index " << selectedIndex;
    emit currentNameChanged();
    if(selectedIndex < 0)
        return;


    auto query = db_->getBinder(
        "SELECT title, description FROM tasks "
        "WHERE project_id = :id AND title LIKE :tag "
        "ORDER BY timestamp DESC"
        );

    // 2. Bind the ID normally
    query.bindValue(":id", m_projectID);

    // 3. Prep the tag with the % wildcard BEFORE binding
    // This creates a "starts with" search if you use "tag%"
    // or "ends with" if you use "%tag"
    QString tagSearch = "%" + col_map_[selectedIndex].tag_name + "%";
    query.bindValue(":tag", tagSearch);

    setCurrentTag(col_map_[selectedIndex].tag_name);

    task_map_.clear();
    int index = 0;
    QStringList taskList;
    if(query.exec())
    {
        while (query.next()) {

            TaskData task;
            task.title = query.value("title").toString();
            task.desc = query.value("description").toString();
            task_map_[index] = task;
            index++;
            taskList << task.title;

            qInfo() << "[CollaboratorModel]" << task.title;
        }
    }

    // emit layoutChanged();
    setMsgComboList(taskList);
    setTaskDescription(0);

}

QVariant CollaboratorModel::msgComboList() const
{
    return m_msgComboList;
}

void CollaboratorModel::setMsgComboList(const QVariant &newMsgComboList)
{
    if (m_msgComboList == newMsgComboList)
        return;
    m_msgComboList = newMsgComboList;
    emit msgComboListChanged();
}

QString CollaboratorModel::msgDescription() const
{
    return m_msgDescription;
}

void CollaboratorModel::setMsgDescription(const QString &newMsgDescription)
{
    if (m_msgDescription == newMsgDescription)
        return;
    m_msgDescription = newMsgDescription;
    emit msgDescriptionChanged();
}

QString CollaboratorModel::currentTag() const
{
    return m_currentTag;
}

void CollaboratorModel::setCurrentTag(const QString &newCurrentTag)
{
    if (m_currentTag == newCurrentTag)
        return;

    for(int i = 0; i < col_map_.size(); ++i )
    {
        if(col_map_[i].tag_name == m_currentTag)
        {
            updateTagName(i, newCurrentTag);
            break;
        }
    }

    m_currentTag = newCurrentTag;

    emit currentTagChanged();
}

#include "createproject.h"
using namespace homepage;

CreateProject::CreateProject(DbmPtr db, QObject *parent)
    : QAbstractListModel{parent}, db_(db), m_WsName(""), m_WsIcon("")
{
    // m_root_dir = "C:/Users/robor/OneDrive - University of New Orleans/Research/Year 2025/ResearchWorkspace";

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(setRoot()));
    m_timer->setInterval(250);
    m_timer->start();
}

int CreateProject::selectTemplate(const QString &name)
{
    if(name.isEmpty()){
        emit setRootDir(m_root_dir);
        return -1;
    }
    QString sqlCmd= QString("SELECT items FROM %1").arg(name);
    template_ =  db_->queryRow(sqlCmd);
    qInfo() << "[CreateProject] template := " << template_;
    emit layoutChanged();

    sqlCmd= QString("SELECT category_id FROM %1").arg(name);
    for(const auto& cat: db_->queryRow(sqlCmd))
        return cat.toInt();
    return -1;
}

void CreateProject::createProject(const QString &project_name, const QString &template_name)
{
    auto cat = selectTemplate(template_name);
    // populate database
    QStringList info;
    info << project_name << QString::number(cat);
    emit setProjectInfo(info);

        // populate workspace
    QDir baseDir(m_root_dir + "/" + project_name);

    for (const QString &relativePath : template_) {
        const QString fullPath = baseDir.filePath(relativePath);

        if (!baseDir.mkpath(fullPath)) {
            qWarning() << "Failed to create:" << fullPath;

        }
    }

    emit layoutChanged();
}

bool CreateProject::deleteProject(const QString &project_name)
{
    QString dirPath = m_root_dir + "/" + project_name;
    QDir dir(dirPath);
    if (!dir.exists()) {
        qWarning() << "Directory does not exist:" << dirPath;
        return false;
    }

    if (!dir.removeRecursively()) {
        qWarning() << "Failed to delete directory:" << dirPath;
        return false;
    }

    qInfo() << "Directory deleted:" << dirPath;
    return true;
}

void CreateProject::setRoot()
{
    QString sqlCmd= "SELECT name, database, workspace, icon FROM Workspace";
    auto response = db_->queryRow(sqlCmd);
    int k = 0;
    for(int i = 0; i < response.size(); i+=4)
    {
        QStringList data;
        for(int j = i; j < i + 4; ++j)
        {
            data << response[j];
        }
        workspace_map_[++k] = data;
        qInfo() << data;
    }
    setWsIndex(1);

    m_timer->stop();
}

int CreateProject::rowCount(const QModelIndex &parent) const
{

    QString sqlCmd= "SELECT items FROM template";
    data_ = db_->queryRow(sqlCmd);
    return data_.size();
}

QVariant CreateProject::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
        return QVariant();
    int row = index.row();
    switch (role) {
    case IdRole:
        return row;
    default:
        return data_[row];
    }

}

QHash<int, QByteArray> CreateProject::roleNames() const
{
    return {
        {NameRole, "name"},
        {IdRole, "id"}
    };
}


int CreateProject::WsIndex() const
{
    return m_WsIndex;
}

void CreateProject::setWsIndex(int newWsIndex)
{
    qInfo() << "[CreateProject]: set ws index = " << newWsIndex;
    if (m_WsIndex == newWsIndex || newWsIndex < 1 || newWsIndex > workspace_map_.size())
        return;
    m_WsIndex = newWsIndex;
    auto data = workspace_map_[newWsIndex];
    qInfo() << data;
    // name, database, workspace, icon

    auto name = data[0];
    auto icon = QString("images/%1").arg(data[3]);
    setWsIcon(icon);
    setWsName(name);


    m_root_dir = data[2];
    setWsPath(m_root_dir);
    emit setRootDir(m_root_dir);
    auto db_path = data[1];
    emit setReserachDB(db_path);
    emit WsIndexChanged();
}

QString CreateProject::WsName() const
{
    return m_WsName;
}

void CreateProject::setWsName(const QString &newWsName)
{
    if (m_WsName == newWsName)
        return;
    m_WsName = newWsName;
    emit WsNameChanged();
}

QString CreateProject::WsIcon() const
{
    return m_WsIcon;
}

void CreateProject::setWsIcon(const QString &newWsIcon)
{
    if (m_WsIcon == newWsIcon)
        return;
    m_WsIcon = newWsIcon;
    emit WsIconChanged();
}

QString CreateProject::WsPath() const
{
    return m_WsPath;
}

void CreateProject::setWsPath(const QString &newWsPath)
{
    if (m_WsPath == newWsPath)
        return;
    m_WsPath = newWsPath;
    emit WsPathChanged();
    emit setWsPathRoot(newWsPath);
}

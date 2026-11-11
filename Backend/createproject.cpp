#include "createproject.h"
#include <QRegularExpression>
#include <utility>
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

QString CreateProject::selectTemplate(const QString &name)
{
    template_.clear();

    if(name.isEmpty()){
        emit setRootDir(m_root_dir);
        return categoryrepair::canonical::researchProjects();
    }

    // Template names may contain spaces ("Invited Talk"), and each template has
    // a table of its own - the name has to be quoted or the query is a syntax
    // error and the project ends up with no folders and no category.
    const QString sqlCmd = QString("SELECT items FROM \"%1\"").arg(name);
    template_ = db_->queryRow(sqlCmd);
    qInfo() << "[CreateProject] template" << name << ":=" << template_;
    emit layoutChanged();

    // The category comes from the Template registry, not from the first folder
    // of the template: the folders carry their own ids and they disagree.
    return categoryrepair::categoryForTemplate(*db_, name);
}

bool CreateProject::createProject(const QString &project_name, const QString &template_name)
{
    const QString name = project_name.trimmed();

    if (name.isEmpty()) {
        emit projectCreationFailed(tr("Please enter a project name."));
        return false;
    }

    // The name becomes a folder name, so reject what the filesystem will not take.
    static const QRegularExpression illegal(R"([\\/:*?"<>|])");
    if (name.contains(illegal)) {
        emit projectCreationFailed(tr(R"(A project name cannot contain \ / : * ? " < > |)"));
        return false;
    }

    if (m_root_dir.isEmpty()) {
        emit projectCreationFailed(tr("No workspace is selected."));
        return false;
    }

    QDir workspaceDir(m_root_dir);
    if (!workspaceDir.exists() && !workspaceDir.mkpath(QStringLiteral("."))) {
        emit projectCreationFailed(tr("The workspace folder %1 does not exist and could not be created.")
                                       .arg(m_root_dir));
        return false;
    }

    const QString category = selectTemplate(template_name);

    if (template_.isEmpty()) {
        qWarning() << "[CreateProject] Template" << template_name << "has no folders";
    }

    // Add the project to the research database. ProjectPage owns that
    // connection, so the insert happens in its slot.
    QStringList info;
    info << name << category;
    emit setProjectInfo(info);

    // Create the template's folders inside the project directory.
    QDir baseDir(workspaceDir.filePath(name));
    if (!baseDir.exists() && !baseDir.mkpath(QStringLiteral("."))) {
        emit projectCreationFailed(tr("Could not create the folder %1.").arg(baseDir.path()));
        return false;
    }

    for (const QString &relativePath : std::as_const(template_)) {
        const QString fullPath = baseDir.filePath(relativePath);
        if (!baseDir.mkpath(fullPath))
            qWarning() << "[CreateProject] Failed to create:" << fullPath;
    }

    qInfo() << "[CreateProject] Created" << name << "from" << template_name
            << "under" << category;

    emit layoutChanged();
    emit projectCreated(name);
    return true;
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
    // Honour the order the user set by dragging rows in Settings > Manage
    // Workspace, so the first workspace shown here is their highest priority.
    const bool hasSortOrder = db_->getHeaderList("Workspace")
                                  .contains(QStringLiteral("sort_order"), Qt::CaseInsensitive);
    QString sqlCmd = QStringLiteral("SELECT name, database, workspace, icon FROM Workspace ")
                     + (hasSortOrder ? QStringLiteral("ORDER BY COALESCE(sort_order, rowid), rowid")
                                     : QStringLiteral("ORDER BY rowid"));
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

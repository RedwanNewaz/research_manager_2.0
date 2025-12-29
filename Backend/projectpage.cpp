#include "projectpage.h"
using namespace project;

ProjectPage::ProjectPage(DbmPtr db, QObject *parent)
    : QObject{parent}, db_(db)
{
    // m_linkText = root_dir_ = "C:/Users/robor/OneDrive - University of New Orleans/Research/Year 2025/ResearchWorkspace";
}

QString ProjectPage::projectName() const
{
    return m_projectName;
}

void ProjectPage::setProjectName(const QString &newProjectName)
{
    if (m_projectName == newProjectName)
        return;
    m_projectName = newProjectName;
    emit projectNameChanged();
    QString sqlCmd = QString("SELECT id FROM projects WHERE name = '%1'").arg(m_projectName);
    qInfo() << sqlCmd;
    for(const auto& result: db_->queryRow(sqlCmd))
    {
        int id_ = result.toInt();
        emit projectIdChanged(id_);
        qInfo() << "project id = " << id_;

    }

    sqlCmd = QString("SELECT description FROM projects WHERE name = '%1'").arg(m_projectName);

    QString description = db_->queryRow(sqlCmd).front();
    if(description.isEmpty())
        setProjectDescription(newProjectName);
    else
        setProjectDescription(description);

    // update project link directory
    setLinkText(QString("%1/%2").arg(m_projectRoot, newProjectName));


}

QString ProjectPage::projectDescription() const
{
    return m_projectDescription;
}

void ProjectPage::setProjectDescription(const QString &newProjectDescription)
{
    if (m_projectDescription == newProjectDescription || newProjectDescription.isEmpty())
        return;

    qInfo() << "[ProjectPage]: update database " << m_projectDescription;
    // QString sqlCmd = QString("UPDATE projects SET description = '%2' WHERE name = '%1'").arg(m_projectName, newProjectDescription);
    // db_->updateDB(sqlCmd);

    auto query = db_->getBinder("UPDATE projects SET description = :desc WHERE name = :name");
    query.bindValue(":desc", m_projectName);
    query.bindValue(":name", newProjectDescription);
    query.exec();

    m_projectDescription = newProjectDescription;
    emit projectDescriptionChanged();
}

QString ProjectPage::linkText() const
{
    return m_linkText;
}

void ProjectPage::setLinkText(const QString &newLinkText)
{
    if (m_linkText == newLinkText || newLinkText.isEmpty())
        return;

    QDir dir(newLinkText);
    if(!dir.exists())
    {
        setSearchButton(true);
        emit setDownloadDirectory(m_linkText);

        return;
    }

    m_linkText = newLinkText;
    emit linkTextChanged();
    // emit projectRootDir(m_linkText);
    setSearchButton(false);

}

void ProjectPage::setWsPathRoot(const QString &wsPath)
{
    m_wsPath = wsPath;
    qInfo() << "[ProjectPage] set workspace = " << wsPath;
}

void ProjectPage::setRootDir(const QString &root)
{
    if(m_wsPath.isEmpty() || root == m_wsPath || !root.startsWith(m_wsPath))
        return;

    //TODO: don't load entire worksapace, only load a project folder
    qInfo() << "[ProjectPage] set root = " << root;
    // setLinkText(root);
    m_projectRoot = root;
    emit projectRootDir(m_projectRoot);
}

void ProjectPage::setProjectInfo(const QStringList& pInfo)
{
    qInfo() <<"[ProjectPage]: set project info " << pInfo;
    auto projectName = pInfo[0];
    auto cat = pInfo[1].toInt();
    // QString sqlCmd = QString("INSERT INTO projects (name, description, category_id) VALUES ('%1', '%1', %2)")
    //                 .arg(projectName)
    //                 .arg(cat);
    // db_->updateDB(sqlCmd);

    auto query = db_->getBinder("INSERT INTO projects (name, description, category_id) VALUES (:name, :desc, :cat)");
    query.bindValue(":name", projectName);
    query.bindValue(":desc", projectName);
    query.bindValue(":cat", cat);
    query.exec();
}

QString ProjectPage::projectRoot() const
{
    return m_projectRoot;
}

void ProjectPage::setProjectRoot(const QString &newProjectRoot)
{
    if (m_projectRoot == newProjectRoot )
        return;
    m_projectRoot = newProjectRoot;
    emit projectRootChanged();
}

bool ProjectPage::searchButton() const
{
    return m_searchButton;
}

void ProjectPage::setSearchButton(bool newSearchButton)
{
    if (m_searchButton == newSearchButton)
        return;
    m_searchButton = newSearchButton;
    emit searchButtonChanged();
}

void ProjectPage::downloadFile(const QString &url)
{
    emit setDownloadLink(url);
    emit linkTextChanged();
    setSearchButton(false);
}

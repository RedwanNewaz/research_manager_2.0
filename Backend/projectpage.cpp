#include "projectpage.h"
#include "categoryrepair.h"
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

    // A project that is not in the database yet returns nothing here.
    const QStringList descriptions = db_->queryRow(sqlCmd);
    const QString description = descriptions.isEmpty() ? QString() : descriptions.constFirst();
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
    query.bindValue(":desc", newProjectDescription);
    query.bindValue(":name", m_projectName);
    if (!query.exec())
        qWarning() << "[ProjectPage] Failed to update description:" << query.lastError().text();

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

    if (pInfo.size() < 2) {
        qWarning() << "[ProjectPage] Incomplete project info:" << pInfo;
        return;
    }

    const QString projectName = pInfo.at(0);
    const QString categoryName = pInfo.at(1);

    if(db_->initializeDatabase())
    {
        qInfo() << "[ProjectPage] initializing the database for this workspace";
    }

    // Workspaces disagree on category ids - "Grants & Funding" is id 2 in one
    // database and id 4 in another - so file the project by name and let the
    // category be created here if this workspace does not have it yet.
    const int cat = categoryrepair::resolveCategoryId(*db_, categoryName);
    if (cat <= 0)
        qWarning() << "[ProjectPage] Could not resolve category" << categoryName;

    // Re-creating an existing project should re-file it, not add a second row.
    auto existing = db_->getBinder("SELECT COUNT(*) FROM projects WHERE name = :name");
    existing.bindValue(":name", projectName);
    bool isKnown = false;
    if (existing.exec() && existing.next())
        isKnown = existing.value(0).toInt() > 0;

    QSqlQuery query = isKnown
        ? db_->getBinder("UPDATE projects SET category_id = :cat WHERE name = :name")
        : db_->getBinder("INSERT INTO projects (name, description, category_id) "
                         "VALUES (:name, :desc, :cat)");

    query.bindValue(":name", projectName);
    if (!isKnown)
        query.bindValue(":desc", projectName);
    query.bindValue(":cat", cat);

    if (!query.exec()) {
        qWarning() << "[ProjectPage] Error: Failed to execute query:" << query.lastError().text();
        return;
    }

    qInfo() << "[ProjectPage]" << (isKnown ? "re-filed" : "added") << projectName
            << "under category" << categoryName << "(id" << cat << ")";
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

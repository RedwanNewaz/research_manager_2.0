#include "homepage.h"
#include "categoryrepair.h"
#include <algorithm>
#include <QDebug>

namespace homepage{
    ProjectView::ProjectView(DbmPtr db, QObject *parent)
    : QAbstractListModel{parent}, db_(db)
    {


    }

int ProjectView::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    QString sql = "SELECT name, id FROM categories";
    auto cat_response = db_->queryRow(sql);

    QMap<int, QString> catMap;
    for(int i =0; i < cat_response.size(); i+= 2)
    {
        int j = i + 1;
        catMap[cat_response[j].toInt()] = cat_response[i];
    }

    data_.clear();
    sql = "SELECT name, category_id FROM projects";
    auto project_response = db_->queryRow(sql);
    for(int i =0; i < project_response.size(); i+=2)
    {
        int j = i + 1;
        auto cat = catMap.value(project_response[j].toInt());
        // A project whose category_id points nowhere used to land in a group
        // with no name at all, which read as a missing label on the page.
        if (cat.trimmed().isEmpty())
            cat = QStringLiteral("Uncategorized");
        data_[cat] << project_response[i];
    }

    // qInfo() << "Projects have following data \n" << data_;
    // The groups are listed in this order, so sort them by name instead of
    // leaving it to the order the container happened to hand back.
    order_ = data_.keys();
    std::sort(order_.begin(), order_.end(), [](const QString &a, const QString &b) {
        const int byName = a.compare(b, Qt::CaseInsensitive);
        return byName != 0 ? byName < 0 : a < b;
    });

    return data_.size();
}

    QVariant ProjectView::data(const QModelIndex &index, int role) const
    {
        const int row = index.row();
        if (!index.isValid() || row < 0 || row >= order_.size())
            return QVariant();

        const QString folderName = order_.at(row);
        const QStringList category = data_.value(folderName);

        // qInfo() << "[FolderName]: " << folderName;
        switch (role) {
            case NameRole: return folderName;
            case CategoryRole:   return category;
            default: return QVariant();
        }
    }

    QHash<int, QByteArray> ProjectView::roleNames() const
    {
        return {
            { NameRole, "name" },
            { CategoryRole,   "category" }
        };
    }

    void ProjectView::deleteProject(const QString &projectName)
    {
        qInfo() << "[ProjectView]: deleting  project = " << projectName;
        QString sqlCmd= QString("DELETE FROM projects WHERE name = '%1'").arg(projectName);
        if(db_->deleteItem(sqlCmd))
        {
            qInfo() << "[DeadlineModel] success ";
        }
        else{
            qInfo() << sqlCmd;
        }

        emit layoutChanged();
    }

    QStringList ProjectView::availableCategories() const
    {
        QStringList names;
        for (const QString &name : db_->queryRow("SELECT name FROM categories ORDER BY name")) {
            if (!name.trimmed().isEmpty() && !names.contains(name, Qt::CaseInsensitive))
                names << name;
        }
        return names;
    }

    QString ProjectView::categoryOfProject(const QString &projectName) const
    {
        auto query = db_->getBinder("SELECT c.name FROM projects p "
                                    "LEFT JOIN categories c ON c.id = p.category_id "
                                    "WHERE p.name = :name");
        query.bindValue(":name", projectName);
        if (query.exec() && query.next())
            return query.value(0).toString();
        return QString();
    }

    bool ProjectView::setProjectCategory(const QString &projectName, const QString &categoryName)
    {
        if (projectName.trimmed().isEmpty() || categoryName.trimmed().isEmpty()) {
            qWarning() << "[ProjectView] setProjectCategory needs a project and a category";
            return false;
        }

        const int categoryId = categoryrepair::resolveCategoryId(*db_, categoryName);
        if (categoryId <= 0) {
            qWarning() << "[ProjectView] Could not resolve category" << categoryName;
            return false;
        }

        auto query = db_->getBinder("UPDATE projects SET category_id = :cat WHERE name = :name");
        query.bindValue(":cat", categoryId);
        query.bindValue(":name", projectName);
        if (!query.exec()) {
            qWarning() << "[ProjectView] Could not move" << projectName << ":"
                       << query.lastError().text();
            return false;
        }

        if (query.numRowsAffected() == 0) {
            qWarning() << "[ProjectView] No project named" << projectName;
            return false;
        }

        qInfo() << "[ProjectView] Moved" << projectName << "to" << categoryName
                << "(id" << categoryId << ")";
        reload();
        return true;
    }

    void ProjectView::reload()
    {
        beginResetModel();
        endResetModel();
        updateProjectsList();
        emit layoutChanged();
    }

    void ProjectView::setReserachDB(const QString &db_path)
    {
        qInfo() << "[ProjectView]: setReserachDB " << db_path;
        db_->connect(db_path);
        ensureDefaultCategories();
        updateProjectsList();
        emit layoutChanged();
    }

    void ProjectView::ensureDefaultCategories()
    {
        // Check if categories already exist
        QString checkSql = "SELECT COUNT(*) FROM categories";
        auto result = db_->queryRow(checkSql);
        if (!result.isEmpty() && result[0].toInt() > 0) {
            qInfo() << "[ProjectView]: Categories already exist, skipping initialization";
            return;
        }

        // Create default categories for existing databases
        QStringList defaultCategories = {"Research Projects", "Publications", "Presentations", "Grants & Funding", "Teaching", "Service"};

        for (int i = 0; i < defaultCategories.size(); ++i) {
            auto query = db_->getBinder(
                "INSERT INTO categories (id, name, year_id) VALUES (:id, :name, :year)"
            );
            query.bindValue(":id", i + 1);
            query.bindValue(":name", defaultCategories[i]);
            query.bindValue(":year", 0); // year_id can be 0 for general categories

            if (!query.exec()) {
                qWarning() << "[ProjectView] Failed to insert default category:" << defaultCategories[i]
                          << query.lastError().text();
            } else {
                qInfo() << "[ProjectView] Added default category:" << defaultCategories[i];
            }
        }

        qInfo() << "[ProjectView] Initialized" << defaultCategories.size() << "default categories for existing database";
    }

    void ProjectView::searchProjects(const QString &searchText)
    {
        m_searchSuggestions.clear();
        
        if (searchText.isEmpty()) {
            emit searchSuggestionsChanged();
            return;
        }
        
        // Get all project names from data_
        QStringList allProjectNames = getAllProjectNames();
        
        // Filter projects that contain the search text (case-insensitive)
        QString lowerSearchText = searchText.toLower();
        for (const QString &projectName : allProjectNames) {
            if (projectName.toLower().contains(lowerSearchText)) {
                m_searchSuggestions.append(projectName);
            }
        }
        
        // Sort suggestions: exact matches first, then prefix matches, then contains
        std::sort(m_searchSuggestions.begin(), m_searchSuggestions.end(),
                  [&lowerSearchText](const QString &a, const QString &b) {
            QString lowerA = a.toLower();
            QString lowerB = b.toLower();
            
            // Exact match
            bool aExact = (lowerA == lowerSearchText);
            bool bExact = (lowerB == lowerSearchText);
            if (aExact != bExact) return aExact;
            
            // Starts with
            bool aStarts = lowerA.startsWith(lowerSearchText);
            bool bStarts = lowerB.startsWith(lowerSearchText);
            if (aStarts != bStarts) return aStarts;
            
            // Alphabetical
            return a < b;
        });
        
        // Limit to top 10 suggestions
        if (m_searchSuggestions.size() > 10) {
            m_searchSuggestions = m_searchSuggestions.mid(0, 10);
        }
        
        qInfo() << "[ProjectView]: Search for '" << searchText << "' found" << m_searchSuggestions.size() << "matches";
        emit searchSuggestionsChanged();
    }
    
    QStringList ProjectView::getAllProjectNames() const
    {
        QStringList allNames;
        
        // Iterate through all categories and collect project names
        for (const QString &category : data_.keys()) {
            const QStringList &projects = data_.value(category);
            for (const QString &projectName : projects) {
                if (!projectName.isEmpty()) {
                    allNames.append(projectName);
                }
            }
        }
        
        return allNames;
    }
    
    void ProjectView::clearSearch()
    {
        m_searchSuggestions.clear();
        emit searchSuggestionsChanged();
    }

    bool ProjectView::isValidProject(const QString &folder)
    {
        auto allProjects = getAllProjectNames();
        qInfo() << allProjects;
        return allProjects.contains(folder, Qt::CaseInsensitive);
    }
    
    void ProjectView::updateProjectsList()
    {
        m_allProjects = getAllProjectNames();
        m_allProjects.sort();
        emit allProjectsChanged();
    }
}

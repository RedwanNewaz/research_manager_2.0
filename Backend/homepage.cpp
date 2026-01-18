#include "homepage.h"
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
        auto cat = catMap[project_response[j].toInt()];
        data_[cat] << project_response[i];
    }

    // qInfo() << "Projects have following data \n" << data_;
    return data_.size();
}

    QVariant ProjectView::data(const QModelIndex &index, int role) const
    {
        int row = index.row();
        if (!index.isValid() || row > data_.size())
            return QVariant();


        int i = 0;
        QVariant folderName, category;
        for(const auto& item: data_.keys())
        {
            if(i == row)
            {
                folderName = item;
                category = data_.value(item);
                break;
            }
            ++i;
        }

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

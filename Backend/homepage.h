#ifndef HOMEPAGE_H
#define HOMEPAGE_H

#include <QObject>
#include <QHash>
#include <QMap>
#include <QAbstractListModel>
#include "database.h"

namespace homepage{

class ProjectView : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QStringList searchSuggestions READ searchSuggestions NOTIFY searchSuggestionsChanged)
    Q_PROPERTY(QStringList allProjects READ allProjects NOTIFY allProjectsChanged)
    
public:
    explicit ProjectView(DbmPtr db, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void deleteProject(const QString& projectName);

    Q_INVOKABLE void setReserachDB(const QString& db_path);

    /**
     * @brief Re-read the projects from the database and refresh the page.
     *
     * Called after a project is created or deleted; without it a new project
     * only shows up after a restart.
     */
    Q_INVOKABLE void reload();

    /**
     * @brief Category names this workspace offers, in display order.
     *
     * Used to build the "Move to category" menu on a project tile.
     */
    Q_INVOKABLE QStringList availableCategories() const;

    /** @brief The category @p projectName is currently filed under. */
    Q_INVOKABLE QString categoryOfProject(const QString& projectName) const;

    /**
     * @brief Re-file @p projectName under @p categoryName.
     *
     * The category is matched by name and created in this workspace if it does
     * not exist yet, so the same menu works in every workspace regardless of
     * how its categories are numbered.
     *
     * @return true when the project was moved.
     */
    Q_INVOKABLE bool setProjectCategory(const QString& projectName, const QString& categoryName);
    
    // Autocomplete search methods
    Q_INVOKABLE void searchProjects(const QString& searchText);
    Q_INVOKABLE QStringList getAllProjectNames() const;
    Q_INVOKABLE void clearSearch();
    Q_INVOKABLE bool isValidProject(const QString& folder);
    
    QStringList searchSuggestions() const { return m_searchSuggestions; }
    QStringList allProjects() const { return m_allProjects; }



signals:
    void searchSuggestionsChanged();
    void allProjectsChanged();

private:
    enum ProjectRoles {
        NameRole = Qt::UserRole + 1,
        CategoryRole
    };

    /**
     * @brief Projects grouped by category name.
     *
     * A QMap, not a QHash: the page lists the groups in key order, and a hash
     * would put them in an arbitrary order that changes between runs.
     */
    mutable QMap<QString, QStringList> data_;

    /** @brief Category names in the order the page shows them (alphabetical). */
    mutable QStringList order_;
    DbmPtr db_;
    QStringList m_searchSuggestions;
    QStringList m_allProjects;

    void updateProjectsList();
    void ensureDefaultCategories();

};


}

#endif // HOMEPAGE_H

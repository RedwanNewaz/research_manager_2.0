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

    mutable QHash<QString, QStringList> data_;
    DbmPtr db_;
    QStringList m_searchSuggestions;
    QStringList m_allProjects;
    
    void updateProjectsList();

};


}

#endif // HOMEPAGE_H

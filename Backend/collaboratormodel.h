#ifndef COLLABORATORMODEL_H
#define COLLABORATORMODEL_H

#include <QObject>
#include <QDebug>
#include <QAbstractListModel>
#include "database.h"
#include <QMap>
namespace project{
class CollaboratorModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit CollaboratorModel(DbmPtr db, QObject *parent = nullptr);

    Q_INVOKABLE void projectIdChanged(int newId);
signals:

    // QAbstractItemModel interface
public:
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    Q_INVOKABLE void addCollaborator(const QString& name, const QString& photo="");
    Q_INVOKABLE void deleteCollaborator(int index);
    Q_INVOKABLE void updateTagName(int index, const QString& tag);

private:
    DbmPtr db_;
    int m_projectID;
    struct ColData{
        int id;
        QString name;
        QString tag_name;
        QString photo;
    };

    mutable QMap<int, ColData> col_map_;
    enum CollabRoles {
        Name = Qt::UserRole + 1,
        Tag,
        Photo,
    };

    // QAbstractItemModel interface
public:
    QHash<int, QByteArray> roleNames() const override;
};

}
#endif // COLLABORATORMODEL_H

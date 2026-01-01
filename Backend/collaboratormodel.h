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
    Q_PROPERTY(QString currentTag READ currentTag WRITE setCurrentTag NOTIFY currentTagChanged FINAL)
    Q_PROPERTY(QString currentName READ currentName WRITE setCurrentName NOTIFY currentNameChanged FINAL)
    Q_PROPERTY(QVariant msgComboList READ msgComboList WRITE setMsgComboList NOTIFY msgComboListChanged FINAL)
    Q_PROPERTY(QString msgDescription READ msgDescription WRITE setMsgDescription NOTIFY msgDescriptionChanged FINAL)
public:
    explicit CollaboratorModel(DbmPtr db, QObject *parent = nullptr);

    Q_INVOKABLE void projectIdChanged(int newId);
signals:

    // QAbstractItemModel interface
    void currentNameChanged();

    void msgComboListChanged();

    void msgDescriptionChanged();

    void currentTagChanged();

public:
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    Q_INVOKABLE void addCollaborator(const QString& name, const QString& photo="");
    Q_INVOKABLE void deleteCollaborator(int index);
    Q_INVOKABLE void updateTagName(int index, const QString& tag);
    Q_INVOKABLE QStringList getTaskTitles() const;
    Q_INVOKABLE void setTaskDescription(int index);

private:
    DbmPtr db_;
    int m_projectID;

    struct ColData{
        int id;
        QString name;
        QString tag_name;
        QString photo;
    };

    struct TaskData{
        QString title;
        QString desc;
    };

    mutable QMap<int, ColData> col_map_;
    QMap<int, TaskData> task_map_;
    enum CollabRoles {
        Name = Qt::UserRole + 1,
        Tag,
        Photo,
    };

    // QAbstractItemModel interface
    QString m_currentName;

    QVariant m_msgComboList;

    QString m_msgDescription;

    QString m_currentTag;

public:
    QHash<int, QByteArray> roleNames() const override;
    QString currentName() const;
    void setCurrentName(const QString &newCurrentName);
    QVariant msgComboList() const;
    void setMsgComboList(const QVariant &newMsgComboList);
    QString msgDescription() const;
    void setMsgDescription(const QString &newMsgDescription);
    QString currentTag() const;
    void setCurrentTag(const QString &newCurrentTag);
};

}
#endif // COLLABORATORMODEL_H

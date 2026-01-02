#ifndef MESSAGEVIEWER_H
#define MESSAGEVIEWER_H

#include <QObject>
#include <QAbstractListModel>
#include "database.h"

namespace collab {
class MessageViewer : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString currentName READ currentName WRITE setCurrentName NOTIFY currentNameChanged FINAL)
public:
    explicit MessageViewer(DbmPtr db, QObject *parent = nullptr);

signals:

    void currentNameChanged();

private:
    DbmPtr db_;
    struct MsgData{
        QString title;
        QString desc;
        QString tag;
    };
    QMap<int, MsgData> task_map_;

    // QAbstractItemModel interface
    QString m_currentName;

    enum MsgRoles {
        Subject = Qt::UserRole + 1,
        Tag,
        Description,
    };

    int m_projectID = -1;

public:
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    QString currentName() const;
    void setCurrentName(const QString &newCurrentName);
    Q_INVOKABLE void projectIdChanged(int newId);
};
}

#endif // MESSAGEVIEWER_H

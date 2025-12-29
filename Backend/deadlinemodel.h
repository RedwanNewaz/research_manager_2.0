#ifndef DEADLINEMODEL_H
#define DEADLINEMODEL_H

#include <QObject>
#include <QAbstractTableModel>
#include <QVector>
#include <QDebug>
#include <QHash>
#include <QDateTime>
#include "database.h"
#include "deadlineparser.h"

namespace project{
    class DeadlineModel : public QAbstractTableModel
    {
        Q_OBJECT
        Q_PROPERTY(QString deadlineTxt READ deadlineTxt WRITE setDeadlineTxt NOTIFY deadlineTxtChanged FINAL)
    public:
        explicit DeadlineModel(DbmPtr db, QObject *parent = nullptr);
        Q_INVOKABLE void projectIdChanged(int id);
        Q_INVOKABLE void deleteRow(int id);
        Q_INVOKABLE QString getEventCountdown(int indx);

    signals:

        void deadlineTxtChanged();

    private:
        DbmPtr db_;
        int m_projectId;

        struct EventData{
            int index;
            int id;
            QDate date;
            QString name;
        };

        mutable QMap<int, EventData> event_map_;


        // QAbstractItemModel interface
        QString m_deadlineTxt;

    public:
        int rowCount(const QModelIndex &parent) const;
        int columnCount(const QModelIndex &parent) const;
        QVariant data(const QModelIndex &index, int role) const;
        QHash<int, QByteArray> roleNames() const;
        QString deadlineTxt() const;
        void setDeadlineTxt(const QString &newDeadlineTxt);
    };
}

#endif // DEADLINEMODEL_H

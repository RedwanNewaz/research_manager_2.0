#ifndef TASKMANGER_H
#define TASKMANGER_H

#include <QObject>
#include <QDebug>
#include <map>
#include <QAbstractListModel>
#include "database.h"


namespace project{
class TaskManger : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int taskIndex READ taskIndex WRITE setTaskIndex NOTIFY taskIndexChanged FINAL)
    Q_PROPERTY(QString taskDescription READ taskDescription WRITE setTaskDescription NOTIFY taskDescriptionChanged FINAL)
    Q_PROPERTY(QString taskTitle READ taskTitle WRITE setTaskTitle NOTIFY taskTitleChanged FINAL)
public:
    explicit TaskManger(DbmPtr db, QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void addTask(const QString& text);
    Q_INVOKABLE void editTask(int index, const QString& title, const QString& description);
    Q_INVOKABLE void deleteTasks();
    Q_INVOKABLE void updateCheckedBox(int index, bool value);
    Q_INVOKABLE void moveItem(int from, int to);

    int taskIndex() const;
    void setTaskIndex(int newTaskIndex);

    QString taskDescription() const;
    void setTaskDescription(const QString &newTaskDescription);

    QString taskTitle() const;
    void setTaskTitle(const QString &newTaskTitle);

public slots:
    void projectIdChanged(int);

signals:
    void taskIndexChanged();

    void taskDescriptionChanged();

    void taskTitleChanged();

private:
    DbmPtr db_;
    int m_projectId;

    struct TaskRecord{
        int index;
        int id;
        QString data;
        QDateTime timestamp;
        bool checked;
    };

    mutable std::map<int, TaskRecord> record_map_;

    void updateTimestamp(int id, const QDateTime& timestamp) const;



    enum TaskRoles {
        TitleRole = Qt::UserRole + 1,
        TimestampRole,
        CheckBoxRole
    };
    int m_taskIndex;
    QString m_taskDescription;
    QString m_taskTitle;
};
}

#endif // TASKMANGER_H

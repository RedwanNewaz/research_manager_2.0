#ifndef WORKSPACEMODEL_H
#define WORKSPACEMODEL_H

#include <QObject>
#include <QDir>
#include <QDebug>
#include <QAbstractTableModel>
#include <QVariantMap>
#include "database.h"


class WorkspaceModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit WorkspaceModel(DbmPtr db, QObject *parent = nullptr);

signals:

    // QAbstractItemModel interface
public:
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    // Add these declarations
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Q_INVOKABLE bool createWorkspace(const QVariantMap &data);
    Q_INVOKABLE bool updateWorkspace(const QVariantMap &data);
    Q_INVOKABLE bool deleteWorkspace(int row);
private:
    DbmPtr db_;
    mutable QStringList headers_;
    mutable QList<QStringList> tableData_; // Cache for row/column data
          // Helper to load data from DB
};

#endif // WORKSPACEMODEL_H

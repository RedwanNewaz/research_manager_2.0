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

    /**
     * @brief Move the workspace at @p from to position @p to and persist the order.
     *
     * Used by the drag-to-reorder interaction in WorkspaceManager.qml. The new
     * order is stored in the hidden "sort_order" column, so it survives restarts.
     */
    Q_INVOKABLE bool moveWorkspace(int from, int to);

private:
    /** @brief Columns that back the ordering but are never shown in the table. */
    static bool isHiddenColumn(const QString &column);

    /** @brief Add the "sort_order" column on first use and seed it from the current order. */
    bool ensureSortOrderColumn();

    DbmPtr db_;
    mutable QStringList headers_;
    mutable QList<QStringList> tableData_; // Cache for row/column data
          // Helper to load data from DB
};

#endif // WORKSPACEMODEL_H

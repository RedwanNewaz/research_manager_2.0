#include "workspacemodel.h"

WorkspaceModel::WorkspaceModel(DbmPtr db, QObject *parent)
    : QAbstractTableModel{parent}, db_(db)
{

}

int WorkspaceModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    // Initialize headers and data

    // Assuming a specific table name for this model, e.g., "Workspace"
    headers_ = db_->getHeaderList("Workspace");

    // qInfo() << "[WorkspaceModel]: " << headers_;

    tableData_.clear();


    int cols = headers_.count();
    QString sqlCmd = "SELECT * FROM Workspace";
    auto response = db_->queryRow(sqlCmd);

    for(int i =0; i < response.size(); i+=cols)
    {
        QStringList temp;
        for(int j = i; j < i+cols; ++j)
            temp << response[j];
        tableData_.append(temp);
    }



    return tableData_.count();
}

int WorkspaceModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return headers_.count();
}

QVariant WorkspaceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        // Retrieve the string from our cached list of rows
        return tableData_.at(index.row()).at(index.column());
    }

    return QVariant();
}

QVariant WorkspaceModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            if (section >= 0 && section < headers_.size()) {
                return headers_.at(section);
            }
        } else if (orientation == Qt::Vertical) {
            // Optional: return row numbers
            return section + 1;
        }
    }
    return QVariant();
}


Qt::ItemFlags WorkspaceModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

bool WorkspaceModel::setData(const QModelIndex &index, const QVariant &value, int role)
{

    // qInfo() << "[WorkspaceModel] setData called for row:" << index.row() << "col:" << index.column() << "value:" << value.toString();

    if (!index.isValid() || role != Qt::EditRole) {
        qInfo() << "[WorkspaceModel] Invalid index or role";
        return false;
    }

    if (index.row() >= tableData_.count() ||
        index.column() >= tableData_.at(index.row()).count()) {
        qInfo() << "[WorkspaceModel] Out of bounds";
        return false;
    }

    // Update the local data
    QString oldValue = tableData_[index.row()][index.column()];


    // Update the database
    QString columnName = headers_.at(index.column());
    QString tableName = "Workspace";

    // You'll need a way to identify the row - assuming first column is ID
    QString idColumn = headers_.at(0);
    QString idValue = tableData_.at(index.row()).at(0);

    // QString sqlCmd = QString("UPDATE %1 SET %2 = '%3' WHERE %4 = '%5'")
    //                      .arg(tableName)
    //                      .arg(columnName)
    //                      .arg(value.toString().replace("'", "''"))  // Escape single quotes
    //                      .arg(idColumn)
    //                      .arg(idValue);

    // // qInfo() << "[WorkspaceModel] SQL:" << sqlCmd;

    // bool success = db_->updateDB(sqlCmd);

    auto cmd = QString("UPDATE %1 SET %2 = :val WHERE %3 = :id")
               .arg(tableName)
               .arg(columnName)
               .arg(idColumn);

    auto query = db_->getBinder(cmd);
    query.bindValue(":val", value.toString());
    query.bindValue(":id", idValue);
    bool success = query.exec();


    tableData_[index.row()][index.column()] = value.toString();

    if (success) {
        // qInfo() << "[WorkspaceModel] Database updated successfully";
        emit dataChanged(index, index, {role});
        emit layoutChanged();
        return true;
    } else {
        qInfo() << "[WorkspaceModel] Database update FAILED - reverting local data";
        // Revert local data if DB update failed
        tableData_[index.row()][index.column()] = oldValue;
        return false;
    }
}

bool WorkspaceModel::createWorkspace(const QVariantMap &data)
{
    QString name = data.value("Name").toString();
    QString database = data.value("Database").toString();
    int year = data.value("Year").toInt();
    QString workspace = data.value("Workspace").toString();
    QString icon = data.value("Icon").toString();

    // check workspace dir exists or not
    QDir wsDir(workspace);

    if(wsDir.exists())
    {
        qWarning() << "[WorkspaceModel] No workspace created. It already exists";
        return false;
    }

    if (!wsDir.mkpath(workspace)) {
        qWarning() << "[WorkspaceModel] Failed to create: " << workspace;
        return false;
    }

    // create database
    if(!initializeDatabase(database))
    {
        qWarning() << "[WorkspaceModel] Failed to database: " << database;
        return false;
    }

    // Create default categories for new workspace
    DatabaseManager researchDb("tempResearch", database);
    if (!researchDb.connect(database)) {
        qWarning() << "[WorkspaceModel] Failed to connect to research database for category initialization";
        return false;
    }

    // Insert default categories
    QStringList defaultCategories = {"Research Projects", "Publications", "Presentations", "Grants & Funding", "Teaching", "Service"};
    for (int i = 0; i < defaultCategories.size(); ++i) {
        auto categoryQuery = researchDb.getBinder(
            "INSERT INTO categories (id, name, year_id) VALUES (:id, :name, :year)"
        );
        categoryQuery.bindValue(":id", i + 1);
        categoryQuery.bindValue(":name", defaultCategories[i]);
        categoryQuery.bindValue(":year", 0); // year_id can be 0 for general categories
        if (!categoryQuery.exec()) {
            qWarning() << "[WorkspaceModel] Failed to insert default category:" << defaultCategories[i]
                      << categoryQuery.lastError().text();
        }
    }

    qInfo() << "[WorkspaceModel] Created" << defaultCategories.size() << "default categories for workspace";

    // --- 2. Construct the SQL Command using QString::arg() ---

    // QString sqlCmd = QString(
    //                      "INSERT INTO Workspace (name, database, year, workspace, icon) "
    //                      "VALUES ('%1', '%2', %3, '%4', '%5')"
    //                      )
    //                      .arg(name)
    //                      .arg(database)
    //                      .arg(year)
    //                      .arg(workspace)
    //                      .arg(icon);

    // if(db_->updateDB(sqlCmd))
    // {
    //     qInfo() << "[WorkspaceModel] create Workspace success ";
    //     return true;
    // }

    // qInfo() << sqlCmd;
    // return false;

    auto query = db_->getBinder(
        "INSERT INTO Workspace (name, database, year, workspace, icon) "
        "VALUES (:name, :database, :year, :workspace, :icon)"
        );

    query.bindValue(":name", name);
    query.bindValue(":database", database);
    query.bindValue(":year", year);
    query.bindValue(":workspace", workspace);
    query.bindValue(":icon", icon);

    return query.exec();

}

bool WorkspaceModel::updateWorkspace(const QVariantMap &data)
{
    QString name = data.value("Name").toString();
    QString database = data.value("Database").toString();
    int year = data.value("Year").toInt();
    QString workspace = data.value("Workspace").toString();
    QString icon = data.value("Icon").toString();

    // Check if workspace exists by querying
    QString checkCmd = QString("SELECT COUNT(*) FROM Workspace WHERE name = '%1'").arg(name);
    auto result = db_->queryRow(checkCmd);

    if(!result.isEmpty() && result[0].toInt() > 0)
    {
        // // Workspace exists, update it
        // QString sqlCmd = QString(
        //                      "UPDATE Workspace SET database = '%1', year = %2, workspace = '%3', icon = '%4' "
        //                      "WHERE name = '%5'"
        //                      )
        //                      .arg(database)
        //                      .arg(year)
        //                      .arg(workspace)
        //                      .arg(icon)
        //                      .arg(name);

        // if(db_->updateDB(sqlCmd))
        // {
        //     qInfo() << "[WorkspaceModel] Workspace updated successfully";
        //     return true;
        // }
        // else
        // {
        //     qWarning() << "[WorkspaceModel] Failed to update workspace";
        //     return false;
        // }

        auto query = db_->getBinder(
            "UPDATE Workspace SET (name, database, year, workspace, icon) "
            "VALUES (:name, :database, :year, :workspace, :icon)"
            );

        query.bindValue(":name", name);
        query.bindValue(":database", database);
        query.bindValue(":year", year);
        query.bindValue(":workspace", workspace);
        query.bindValue(":icon", icon);
        return query.exec();
    }
    else
    {
        // Workspace does not exist, insert it
        auto query = db_->getBinder(
            "INSERT INTO Workspace (name, database, year, workspace, icon) "
            "VALUES (:name, :database, :year, :workspace, :icon)"
            );

        query.bindValue(":name", name);
        query.bindValue(":database", database);
        query.bindValue(":year", year);
        query.bindValue(":workspace", workspace);
        query.bindValue(":icon", icon);
        return query.exec();
    }
}

bool WorkspaceModel::deleteWorkspace(int row)
{
    if (row < 0 || row >= tableData_.count()) {
        qWarning() << "[WorkspaceModel] Invalid row for deletion:" << row;
        return false;
    }

    // Get the ID of the workspace to delete (assuming first column is ID)
    QString idColumn = headers_.at(0);
    QString idValue = tableData_.at(row).at(0);
    QString workspacePath = tableData_.at(row).at(4); // workspace column

    QString sqlCmd = QString("DELETE FROM Workspace WHERE %1 = '%2'")
                         .arg(idColumn)
                         .arg(idValue);

    qInfo() << "[WorkspaceModel] Deleting workspace with SQL:" << sqlCmd;

    beginRemoveRows(QModelIndex(), row, row);
    bool success = db_->deleteItem(sqlCmd);

    if (success) {
        tableData_.removeAt(row);
        qInfo() << "[WorkspaceModel] Workspace deleted successfully";
    } else {
        qWarning() << "[WorkspaceModel] Failed to delete workspace from database";
    }
    endRemoveRows();

    return success;
}


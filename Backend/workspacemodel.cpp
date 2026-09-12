#include "workspacemodel.h"
#include <utility>

WorkspaceModel::WorkspaceModel(DbmPtr db, QObject *parent)
    : QAbstractTableModel{parent}, db_(db)
{

}

bool WorkspaceModel::isHiddenColumn(const QString &column)
{
    // "sort_order" only exists to remember the user's drag-and-drop ordering.
    return column.compare(QStringLiteral("sort_order"), Qt::CaseInsensitive) == 0;
}

int WorkspaceModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;

    // Assuming a specific table name for this model, e.g., "Workspace"
    const QStringList allColumns = db_->getHeaderList("Workspace");

    headers_.clear();
    for (const QString &column : allColumns) {
        if (!isHiddenColumn(column))
            headers_ << column;
    }

    tableData_.clear();
    if (headers_.isEmpty())
        return 0;

    QStringList quoted;
    for (const QString &column : std::as_const(headers_))
        quoted << QStringLiteral("\"%1\"").arg(column);

    // Respect the user's manual ordering when the column is present; fall back
    // to insertion order otherwise.
    const bool hasSortOrder = allColumns.contains(QStringLiteral("sort_order"), Qt::CaseInsensitive);
    const QString orderBy = hasSortOrder
        ? QStringLiteral(" ORDER BY COALESCE(sort_order, rowid), rowid")
        : QStringLiteral(" ORDER BY rowid");

    const QString sqlCmd = "SELECT " + quoted.join(", ") + " FROM Workspace" + orderBy;
    const auto response = db_->queryRow(sqlCmd);

    const int cols = headers_.count();
    for (int i = 0; i + cols <= response.size(); i += cols)
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
    Q_UNUSED(tableData_.at(row).value(headers_.indexOf("workspace"))); // workspace column

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


bool WorkspaceModel::ensureSortOrderColumn()
{
    const QStringList columns = db_->getHeaderList("Workspace");
    if (columns.isEmpty()) {
        qWarning() << "[WorkspaceModel] Workspace table is unavailable";
        return false;
    }

    if (!columns.contains(QStringLiteral("sort_order"), Qt::CaseInsensitive)) {
        if (!db_->createTable("ALTER TABLE Workspace ADD COLUMN sort_order INTEGER")) {
            qWarning() << "[WorkspaceModel] Could not add sort_order column";
            return false;
        }
        qInfo() << "[WorkspaceModel] Added sort_order column to Workspace table";
    }

    // Rows added before this feature (or by createWorkspace) have no order yet;
    // seed them from their insertion order so nothing jumps around.
    auto seed = db_->getBinder("UPDATE Workspace SET sort_order = rowid WHERE sort_order IS NULL");
    if (!seed.exec())
        qWarning() << "[WorkspaceModel] Could not seed sort_order:" << seed.lastError().text();

    return true;
}

bool WorkspaceModel::moveWorkspace(int from, int to)
{
    const int count = tableData_.count();

    if (from == to)
        return false;
    if (from < 0 || from >= count || to < 0 || to >= count) {
        qWarning() << "[WorkspaceModel] moveWorkspace out of range:" << from << "->" << to
                   << "(rows:" << count << ")";
        return false;
    }

    if (!ensureSortOrderColumn())
        return false;

    // Column 0 is the workspace name, which identifies the row in the database.
    QStringList names;
    names.reserve(count);
    for (const QStringList &row : std::as_const(tableData_))
        names << row.value(0);

    names.move(from, to);

    // beginMoveRows wants the destination *before* the row is removed
    const int destination = (to > from) ? to + 1 : to;
    if (!beginMoveRows(QModelIndex(), from, from, QModelIndex(), destination)) {
        qWarning() << "[WorkspaceModel] beginMoveRows rejected" << from << "->" << to;
        return false;
    }

    tableData_.move(from, to);

    bool success = true;
    for (int i = 0; i < names.size(); ++i) {
        auto query = db_->getBinder("UPDATE Workspace SET sort_order = :order WHERE name = :name");
        query.bindValue(":order", i);
        query.bindValue(":name", names.at(i));
        if (!query.exec()) {
            success = false;
            qWarning() << "[WorkspaceModel] Failed to persist order for" << names.at(i)
                       << query.lastError().text();
        }
    }

    endMoveRows();

    if (success)
        qInfo() << "[WorkspaceModel] Moved workspace" << from << "->" << to;

    return success;
}

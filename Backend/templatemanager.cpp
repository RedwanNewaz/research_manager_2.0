#include "templatemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

TemplateModel::TemplateModel(DbmPtr dbManager, QObject *parent)
    : QAbstractListModel(parent)
    , m_dbManager(dbManager)
{
}

int TemplateModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_templates.count();
}

QVariant TemplateModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_templates.count())
        return QVariant();

    const TemplateItem &item = m_templates[index.row()];

    switch (role) {
    case IdRole:
        return item.id;
    case ItemsRole:
        return item.items;
    case CategoryIdRole:
        return item.categoryId;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> TemplateModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[ItemsRole] = "items";
    roles[CategoryIdRole] = "categoryId";
    return roles;
}

void TemplateModel::setCurrentTemplate(const QString &templateName)
{
    if (m_currentTemplate != templateName) {
        m_currentTemplate = templateName;
        emit currentTemplateChanged();
        emit templateDataChanged();
    }
}

bool TemplateModel::loadTemplates()
{
    if (!m_dbManager || !m_dbManager->database().isOpen()) {
        qWarning() << "Database not connected";
        return false;
    }

    beginResetModel();
    m_templates.clear();

    QSqlQuery query(m_dbManager->database());
    QString selectSql = "SELECT id, items, category_id FROM Template ORDER BY items";
    
    if (!query.exec(selectSql)) {
        // Check if table exists first
        if (!tableExists("Template")) {
            qDebug() << "Template table does not exist - creating it";
            if (!query.exec("CREATE TABLE IF NOT EXISTS Template ("
                           "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                           "items TEXT NOT NULL,"
                           "category_id INTEGER"
                           ")")) {
                qWarning() << "Failed to create Template table:" << query.lastError().text();
                endResetModel();
                return false;
            }
        } else {
            qWarning() << "Failed to query Template table:" << query.lastError().text();
            endResetModel();
            return false;
        }
    } else {
        // Successfully executed, load the data
        while (query.next()) {
            TemplateItem item;
            item.id = query.value(0).toInt();
            item.items = query.value(1).toString();
            item.categoryId = query.value(2).toInt();
            m_templates.append(item);
        }
    }

    endResetModel();
    emit templatesChanged();
    qDebug() << "Loaded" << m_templates.count() << "templates";
    return true;
}

QVariantList TemplateModel::loadTemplateData(const QString &templateName)
{
    QVariantList result;

    if (!m_dbManager || !m_dbManager->database().isOpen()) {
        qWarning() << "Database not connected";
        return result;
    }

    if (templateName.isEmpty()) {
        qWarning() << "Template name is empty";
        return result;
    }

    // Query the template-specific table (e.g., EPA, etc.)
    QSqlQuery query(m_dbManager->database());
    QString selectSql = QString("SELECT id, items, category_id FROM \"%1\" ORDER BY id").arg(templateName);
    
    if (!query.exec(selectSql)) {
        qWarning() << "Failed to query" << templateName << "table:" << query.lastError().text();
        return result;
    }

    while (query.next()) {
        QVariantMap item;
        item["id"] = query.value(0).toInt();
        item["items"] = query.value(1).toString();
        item["categoryId"] = query.value(2).toInt();
        result.append(item);
    }

    qDebug() << "Loaded" << result.count() << "items from" << templateName << "table";
    return result;
}

QStringList TemplateModel::getTemplateNames() const
{
    QStringList names;
    for (const TemplateItem &item : m_templates) {
        names.append(item.items);
    }
    return names;
}

// ===================== Template CRUD Operations =====================

bool TemplateModel::tableExists(const QString &tableName)
{
    if (!m_dbManager || !m_dbManager->database().isOpen()) {
        return false;
    }
    
    QSqlQuery query(m_dbManager->database());
    query.prepare("SELECT name FROM sqlite_master WHERE type='table' AND name=?");
    query.addBindValue(tableName);
    
    if (query.exec() && query.next()) {
        return true;
    }
    return false;
}

bool TemplateModel::createTemplate(const QString &templateName, int categoryId)
{
    if (!m_dbManager || !m_dbManager->database().isOpen()) {
        emit operationCompleted(false, "Database not connected");
        return false;
    }

    if (templateName.isEmpty()) {
        emit operationCompleted(false, "Template name cannot be empty");
        return false;
    }

    // Check if template already exists
    if (tableExists(templateName)) {
        emit operationCompleted(false, "Template '" + templateName + "' already exists");
        return false;
    }

    QSqlQuery query(m_dbManager->database());
    
    // Start transaction
    m_dbManager->database().transaction();

    // Insert into Template table
    query.prepare("INSERT INTO Template (items, category_id) VALUES (?, ?)");
    query.addBindValue(templateName);
    query.addBindValue(categoryId);
    
    if (!query.exec()) {
        m_dbManager->database().rollback();
        QString error = "Failed to add template entry: " + query.lastError().text();
        qWarning() << error;
        emit operationCompleted(false, error);
        return false;
    }

    // Create the new template table with same structure
    QString createTableSql = QString(
        "CREATE TABLE \"%1\" ("
        "\"id\" INTEGER PRIMARY KEY, "
        "\"items\" TEXT, "
        "\"category_id\" INTEGER)"
    ).arg(templateName);
    
    if (!query.exec(createTableSql)) {
        m_dbManager->database().rollback();
        QString error = "Failed to create template table: " + query.lastError().text();
        qWarning() << error;
        emit operationCompleted(false, error);
        return false;
    }

    m_dbManager->database().commit();
    
    // Reload templates
    loadTemplates();
    
    emit operationCompleted(true, "Template '" + templateName + "' created successfully");
    qDebug() << "Created template:" << templateName;
    return true;
}

bool TemplateModel::deleteTemplate(const QString &templateName)
{
    if (!m_dbManager || !m_dbManager->database().isOpen()) {
        emit operationCompleted(false, "Database not connected");
        return false;
    }

    if (templateName.isEmpty()) {
        emit operationCompleted(false, "Template name cannot be empty");
        return false;
    }

    QSqlQuery query(m_dbManager->database());
    
    // Start transaction
    m_dbManager->database().transaction();

    // Delete from Template table
    query.prepare("DELETE FROM Template WHERE items = ?");
    query.addBindValue(templateName);
    
    if (!query.exec()) {
        m_dbManager->database().rollback();
        QString error = "Failed to delete template entry: " + query.lastError().text();
        qWarning() << error;
        emit operationCompleted(false, error);
        return false;
    }

    // Drop the template table
    QString dropTableSql = QString("DROP TABLE IF EXISTS \"%1\"").arg(templateName);
    
    if (!query.exec(dropTableSql)) {
        m_dbManager->database().rollback();
        QString error = "Failed to drop template table: " + query.lastError().text();
        qWarning() << error;
        emit operationCompleted(false, error);
        return false;
    }

    m_dbManager->database().commit();
    
    // Clear current template if it was the deleted one
    if (m_currentTemplate == templateName) {
        m_currentTemplate.clear();
        emit currentTemplateChanged();
    }
    
    // Reload templates
    loadTemplates();
    
    emit operationCompleted(true, "Template '" + templateName + "' deleted successfully");
    qDebug() << "Deleted template:" << templateName;
    return true;
}

bool TemplateModel::renameTemplate(const QString &oldName, const QString &newName)
{
    if (!m_dbManager || !m_dbManager->database().isOpen()) {
        emit operationCompleted(false, "Database not connected");
        return false;
    }

    if (oldName.isEmpty() || newName.isEmpty()) {
        emit operationCompleted(false, "Template names cannot be empty");
        return false;
    }

    if (oldName == newName) {
        emit operationCompleted(true, "No changes needed");
        return true;
    }

    // Check if new name already exists
    if (tableExists(newName)) {
        emit operationCompleted(false, "Template '" + newName + "' already exists");
        return false;
    }

    QSqlQuery query(m_dbManager->database());
    
    // Start transaction
    m_dbManager->database().transaction();

    // Update Template table
    query.prepare("UPDATE Template SET items = ? WHERE items = ?");
    query.addBindValue(newName);
    query.addBindValue(oldName);
    
    if (!query.exec()) {
        m_dbManager->database().rollback();
        QString error = "Failed to update template entry: " + query.lastError().text();
        qWarning() << error;
        emit operationCompleted(false, error);
        return false;
    }

    // Rename the table
    QString renameSql = QString("ALTER TABLE \"%1\" RENAME TO \"%2\"").arg(oldName, newName);
    
    if (!query.exec(renameSql)) {
        m_dbManager->database().rollback();
        QString error = "Failed to rename template table: " + query.lastError().text();
        qWarning() << error;
        emit operationCompleted(false, error);
        return false;
    }

    m_dbManager->database().commit();
    
    // Update current template if it was renamed
    if (m_currentTemplate == oldName) {
        m_currentTemplate = newName;
        emit currentTemplateChanged();
    }
    
    // Reload templates
    loadTemplates();
    
    emit operationCompleted(true, "Template renamed from '" + oldName + "' to '" + newName + "'");
    qDebug() << "Renamed template from" << oldName << "to" << newName;
    return true;
}

// ===================== Template Item CRUD Operations =====================

bool TemplateModel::addTemplateItem(const QString &templateName, const QString &itemText, int categoryId)
{
    if (!m_dbManager || !m_dbManager->database().isOpen()) {
        emit operationCompleted(false, "Database not connected");
        return false;
    }

    if (templateName.isEmpty() || itemText.isEmpty()) {
        emit operationCompleted(false, "Template name and item text cannot be empty");
        return false;
    }

    QSqlQuery query(m_dbManager->database());
    query.prepare(QString("INSERT INTO \"%1\" (items, category_id) VALUES (?, ?)").arg(templateName));
    query.addBindValue(itemText);
    query.addBindValue(categoryId);
    
    if (!query.exec()) {
        QString error = "Failed to add item: " + query.lastError().text();
        qWarning() << error;
        emit operationCompleted(false, error);
        return false;
    }

    emit templateDataChanged();
    emit operationCompleted(true, "Item added successfully");
    qDebug() << "Added item to" << templateName << ":" << itemText;
    return true;
}

bool TemplateModel::updateTemplateItem(const QString &templateName, int itemId, const QString &newItemText, int newCategoryId)
{
    if (!m_dbManager || !m_dbManager->database().isOpen()) {
        emit operationCompleted(false, "Database not connected");
        return false;
    }

    if (templateName.isEmpty() || newItemText.isEmpty()) {
        emit operationCompleted(false, "Template name and item text cannot be empty");
        return false;
    }

    QSqlQuery query(m_dbManager->database());
    query.prepare(QString("UPDATE \"%1\" SET items = ?, category_id = ? WHERE id = ?").arg(templateName));
    query.addBindValue(newItemText);
    query.addBindValue(newCategoryId);
    query.addBindValue(itemId);
    
    if (!query.exec()) {
        QString error = "Failed to update item: " + query.lastError().text();
        qWarning() << error;
        emit operationCompleted(false, error);
        return false;
    }

    emit templateDataChanged();
    emit operationCompleted(true, "Item updated successfully");
    qDebug() << "Updated item" << itemId << "in" << templateName;
    return true;
}

bool TemplateModel::deleteTemplateItem(const QString &templateName, int itemId)
{
    if (!m_dbManager || !m_dbManager->database().isOpen()) {
        emit operationCompleted(false, "Database not connected");
        return false;
    }

    if (templateName.isEmpty()) {
        emit operationCompleted(false, "Template name cannot be empty");
        return false;
    }

    QSqlQuery query(m_dbManager->database());
    query.prepare(QString("DELETE FROM \"%1\" WHERE id = ?").arg(templateName));
    query.addBindValue(itemId);
    
    if (!query.exec()) {
        QString error = "Failed to delete item: " + query.lastError().text();
        qWarning() << error;
        emit operationCompleted(false, error);
        return false;
    }

    emit templateDataChanged();
    emit operationCompleted(true, "Item deleted successfully");
    qDebug() << "Deleted item" << itemId << "from" << templateName;
    return true;
}

#ifndef TEMPLATEMANAGER_H
#define TEMPLATEMANAGER_H

#include <QObject>
#include <QAbstractListModel>
#include <QVariantMap>
#include "database.h"

/**
 * @brief Model for managing templates from common_config.db
 */
class TemplateModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString currentTemplate READ currentTemplate WRITE setCurrentTemplate NOTIFY currentTemplateChanged)
    Q_PROPERTY(QStringList templateNames READ getTemplateNames NOTIFY templatesChanged)

public:
    enum TemplateRoles {
        IdRole = Qt::UserRole + 1,
        ItemsRole,
        CategoryIdRole
    };

    explicit TemplateModel(DbmPtr dbManager, QObject *parent = nullptr);

    // QAbstractListModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // Property getters/setters
    QString currentTemplate() const { return m_currentTemplate; }
    void setCurrentTemplate(const QString &templateName);

public slots:
    /**
     * @brief Load templates from the Template table
     * @return true if successful
     */
    Q_INVOKABLE bool loadTemplates();

    /**
     * @brief Load data from a specific template table (e.g., EPA)
     * @param templateName Name of the table to query
     * @return List of items from the template table
     */
    Q_INVOKABLE QVariantList loadTemplateData(const QString &templateName);

    /**
     * @brief Get all template names
     * @return List of template names
     */
    Q_INVOKABLE QStringList getTemplateNames() const;

    // ===================== Template CRUD Operations =====================
    
    /**
     * @brief Create a new template (adds entry to Template table and creates new table)
     * @param templateName Name of the new template
     * @param categoryId Category ID for the template
     * @return true if successful
     */
    Q_INVOKABLE bool createTemplate(const QString &templateName, int categoryId = 0);
    
    /**
     * @brief Delete a template (removes from Template table and drops the table)
     * @param templateName Name of the template to delete
     * @return true if successful
     */
    Q_INVOKABLE bool deleteTemplate(const QString &templateName);
    
    /**
     * @brief Rename a template
     * @param oldName Current name of the template
     * @param newName New name for the template
     * @return true if successful
     */
    Q_INVOKABLE bool renameTemplate(const QString &oldName, const QString &newName);

    // ===================== Template Item CRUD Operations =====================
    
    /**
     * @brief Add a new item to a template table
     * @param templateName Name of the template table
     * @param itemText Text for the new item
     * @param categoryId Category ID for the item
     * @return true if successful
     */
    Q_INVOKABLE bool addTemplateItem(const QString &templateName, const QString &itemText, int categoryId = 0);
    
    /**
     * @brief Update an existing item in a template table
     * @param templateName Name of the template table
     * @param itemId ID of the item to update
     * @param newItemText New text for the item
     * @param newCategoryId New category ID for the item
     * @return true if successful
     */
    Q_INVOKABLE bool updateTemplateItem(const QString &templateName, int itemId, const QString &newItemText, int newCategoryId);
    
    /**
     * @brief Delete an item from a template table
     * @param templateName Name of the template table
     * @param itemId ID of the item to delete
     * @return true if successful
     */
    Q_INVOKABLE bool deleteTemplateItem(const QString &templateName, int itemId);

signals:
    void currentTemplateChanged();
    void templateDataChanged();
    void templatesChanged();
    void operationCompleted(bool success, const QString &message);

private:
    struct TemplateItem {
        int id;
        QString items;
        int categoryId;
    };

    QList<TemplateItem> m_templates;
    QString m_currentTemplate;
    DbmPtr m_dbManager;
    
    bool tableExists(const QString &tableName);
};

#endif // TEMPLATEMANAGER_H

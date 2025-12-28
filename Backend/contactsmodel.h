#ifndef CONTACTSMODEL_H
#define CONTACTSMODEL_H

#include <QAbstractTableModel>
#include <QVariantMap>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include "database.h"

namespace project {

class ContactsModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum ContactRoles {
        NameRole = Qt::UserRole + 1,
        AffiliationRole,
        WebsiteRole,
        PhoneRole,
        EmailRole,
        ZoomRole,
        PhotoRole
    };

    explicit ContactsModel(DbmPtr dbm, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QHash<int, QByteArray> roleNames() const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    Q_INVOKABLE bool load_database(const QString &workspaceName = QString(), const QString &workspaceDir = QString());
    Q_INVOKABLE bool addContact(const QString &name);
    Q_INVOKABLE bool addOrUpdateContact(const QVariantMap &contact);
    Q_INVOKABLE bool delete_item(const QString &name);
    Q_INVOKABLE bool deleteContact(const QString &name) { return delete_item(name); }
    Q_INVOKABLE QStringList getAllItems() const;
    Q_INVOKABLE QVariantMap getContact(int row) const;

signals:
    void errorOccurred(const QString &message);

private:
    struct Contact {
        QString name;
        QString affiliation;
        QString website;
        QString phone;
        QString email;
        QString zoom;
        QString photo;
    };

    DbmPtr db_;
    QList<Contact> contacts_;

    bool ensureTableExists();
    int findIndexByName(const QString &name) const;
    bool saveContact(const Contact &contact);
    Contact mapFromVariant(const QVariantMap &contact) const;
};

} // namespace project

#endif // CONTACTSMODEL_H

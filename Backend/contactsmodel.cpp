#include "contactsmodel.h"

using namespace project;

namespace {
static const char *kCreateTableSql = R"(
    CREATE TABLE IF NOT EXISTS "Contacts" (
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name" TEXT NOT NULL,
        "affiliation" TEXT,
        "website" TEXT,
        "phone" TEXT,
        "email" TEXT NOT NULL,
        "zoom" TEXT,
        "photo" TEXT,
        UNIQUE("name")
    )
)";
}

ContactsModel::ContactsModel(DbmPtr dbm, QObject *parent)
    : QAbstractTableModel(parent)
    , db_(std::move(dbm))
{
    ensureTableExists();
}

int ContactsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return contacts_.size();
}

int ContactsModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 7; // name, affiliation, website, phone, email, zoom, photo
}

QVariant ContactsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= contacts_.size())
        return QVariant();

    const Contact &c = contacts_.at(index.row());
    const int col = index.column();
    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        switch (col) {
        case 0: return c.name;
        case 1: return c.affiliation;
        case 2: return c.website;
        case 3: return c.phone;
        case 4: return c.email;
        case 5: return c.zoom;
        case 6: return c.photo;
        default: return QVariant();
        }
    case NameRole: return c.name;
    case AffiliationRole: return c.affiliation;
    case WebsiteRole: return c.website;
    case PhoneRole: return c.phone;
    case EmailRole: return c.email;
    case ZoomRole: return c.zoom;
    case PhotoRole: return c.photo;
    default: return QVariant();
    }
}

Qt::ItemFlags ContactsModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

bool ContactsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.row() < 0 || index.row() >= contacts_.size())
        return false;

    Contact c = contacts_.at(index.row());

    auto applyColumn = [&](int col, const QVariant &val) {
        switch (col) {
        case 0: c.name = val.toString().trimmed(); break;
        case 1: c.affiliation = val.toString().trimmed(); break;
        case 2: c.website = val.toString().trimmed(); break;
        case 3: c.phone = val.toString().trimmed(); break;
        case 4: c.email = val.toString().trimmed(); break;
        case 5: c.zoom = val.toString().trimmed(); break;
        case 6: c.photo = val.toString().trimmed(); break;
        default: break;
        }
    };

    if (role == Qt::EditRole || role == Qt::DisplayRole) {
        applyColumn(index.column(), value);
    } else {
        switch (role) {
        case NameRole: c.name = value.toString().trimmed(); break;
        case AffiliationRole: c.affiliation = value.toString().trimmed(); break;
        case WebsiteRole: c.website = value.toString().trimmed(); break;
        case PhoneRole: c.phone = value.toString().trimmed(); break;
        case EmailRole: c.email = value.toString().trimmed(); break;
        case ZoomRole: c.zoom = value.toString().trimmed(); break;
        case PhotoRole: c.photo = value.toString().trimmed(); break;
        default: return false;
        }
    }

    if (!saveContact(c))
        return false;

    contacts_[index.row()] = c;
    emit dataChanged(index, index, {role, Qt::DisplayRole, Qt::EditRole});
    return true;
}

QHash<int, QByteArray> ContactsModel::roleNames() const
{
    return {
        {Qt::DisplayRole, "display"},
        {Qt::EditRole, "edit"},
        {NameRole, "name"},
        {AffiliationRole, "affiliation"},
        {WebsiteRole, "website"},
        {PhoneRole, "phone"},
        {EmailRole, "email"},
        {ZoomRole, "zoom"},
        {PhotoRole, "photo"}
    };
}

QVariant ContactsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        static const QStringList headers = {
            QStringLiteral("Name"),
            QStringLiteral("Affiliation"),
            QStringLiteral("Website"),
            QStringLiteral("Phone"),
            QStringLiteral("Email"),
            QStringLiteral("Video"),
            QStringLiteral("Photo")
        };
        if (section >= 0 && section < headers.size())
            return headers.at(section);
    } else {
        return section + 1; // 1-based row numbers for vertical header
    }
    return QVariant();
}

bool ContactsModel::ensureTableExists()
{
    if (!db_) {
        emit errorOccurred("Config database is not available");
        return false;
    }

    QSqlQuery query(db_->database());
    if (!query.exec(QString::fromUtf8(kCreateTableSql))) {
        emit errorOccurred("Failed to create Contacts table: " + query.lastError().text());
        return false;
    }
    return true;
}

bool ContactsModel::load_database(const QString &workspaceName, const QString &workspaceDir)
{
    Q_UNUSED(workspaceName)
    Q_UNUSED(workspaceDir)

    if (!ensureTableExists())
        return false;

    QSqlQuery query(db_->database());
    if (!query.exec("SELECT name, affiliation, website, phone, email, zoom, photo FROM Contacts ORDER BY name")) {
        emit errorOccurred("Failed to read contacts: " + query.lastError().text());
        return false;
    }

    QList<Contact> loaded;
    while (query.next()) {
        Contact c;
        c.name = query.value(0).toString();
        c.affiliation = query.value(1).toString();
        c.website = query.value(2).toString();
        c.phone = query.value(3).toString();
        c.email = query.value(4).toString();
        c.zoom = query.value(5).toString();
        c.photo = query.value(6).toString();
        loaded.append(c);
    }

    beginResetModel();
    contacts_ = loaded;
    endResetModel();
    return true;
}

ContactsModel::Contact ContactsModel::mapFromVariant(const QVariantMap &contact) const
{
    Contact c;
    c.name = contact.value("name").toString().trimmed();
    c.affiliation = contact.value("affiliation").toString().trimmed();
    c.website = contact.value("website").toString().trimmed();
    c.phone = contact.value("phone").toString().trimmed();
    c.email = contact.value("email").toString().trimmed();
    c.zoom = contact.value("zoom").toString().trimmed();
    c.photo = contact.value("photo").toString().trimmed();
    return c;
}

bool ContactsModel::saveContact(const Contact &contact)
{
    if (!ensureTableExists())
        return false;

    if (contact.name.isEmpty()) {
        emit errorOccurred("Name is required");
        return false;
    }

    QSqlQuery query(db_->database());
    query.prepare("INSERT INTO Contacts (name, affiliation, website, phone, email, zoom, photo) "
                  "VALUES (:name, :affiliation, :website, :phone, :email, :zoom, :photo) "
                  "ON CONFLICT(name) DO UPDATE SET "
                  "affiliation=excluded.affiliation, "
                  "website=excluded.website, "
                  "phone=excluded.phone, "
                  "email=excluded.email, "
                  "zoom=excluded.zoom, "
                  "photo=excluded.photo");

    query.bindValue(":name", contact.name);
    query.bindValue(":affiliation", contact.affiliation);
    query.bindValue(":website", contact.website);
    query.bindValue(":phone", contact.phone);
    query.bindValue(":email", contact.email);
    query.bindValue(":zoom", contact.zoom);
    query.bindValue(":photo", contact.photo);

    if (!query.exec()) {
        qWarning() << "[ContactsModel] Save failed for:" << contact.name;
        qWarning() << "[ContactsModel] SQL Error:" << query.lastError().text();
        emit errorOccurred("Failed to save contact: " + query.lastError().text());
        return false;
    }
    qDebug() << "[ContactsModel] Contact saved successfully:" << contact.name;
    emit layoutChanged();
    return true;
}

int ContactsModel::findIndexByName(const QString &name) const
{
    for (int i = 0; i < contacts_.size(); ++i) {
        if (contacts_.at(i).name.compare(name, Qt::CaseInsensitive) == 0)
            return i;
    }
    return -1;
}

bool ContactsModel::addContact(const QString &name)
{
    QVariantMap map;
    map.insert("name", name.trimmed());
    map.insert("email", QString());
    return addOrUpdateContact(map);
}

bool ContactsModel::addOrUpdateContact(const QVariantMap &contact)
{
    Contact c = mapFromVariant(contact);
    if (!saveContact(c))
        return false;

    int idx = findIndexByName(c.name);
    if (idx >= 0) {
        // Update existing contact
        contacts_[idx] = c;
        const QModelIndex first = index(idx, 0);
        const QModelIndex last  = index(idx, columnCount() - 1);
        emit dataChanged(first, last, {Qt::DisplayRole, Qt::EditRole, NameRole, AffiliationRole, WebsiteRole, PhoneRole, EmailRole, ZoomRole, PhotoRole});
    } else {
        // Add new contact
        beginInsertRows(QModelIndex(), contacts_.size(), contacts_.size());
        contacts_.append(c);
        endInsertRows();
    }
    return true;
}

bool ContactsModel::delete_item(const QString &name)
{
    if (name.isEmpty())
        return false;

    if (!ensureTableExists())
        return false;

    int idx = findIndexByName(name);
    QSqlQuery query(db_->database());
    query.prepare("DELETE FROM Contacts WHERE name = :name");
    query.bindValue(":name", name);
    if (!query.exec()) {
        emit errorOccurred("Failed to delete contact: " + query.lastError().text());
        return false;
    }

    if (idx >= 0) {
        beginRemoveRows(QModelIndex(), idx, idx);
        contacts_.removeAt(idx);
        endRemoveRows();
    }
    return true;
}

QStringList ContactsModel::getAllItems() const
{
    QStringList names;
    for (const auto &c : contacts_) {
        names << c.name;
    }
    return names;
}

QVariantMap ContactsModel::getContact(int row) const
{
    QVariantMap map;
    if (row < 0 || row >= contacts_.size())
        return map;
    const Contact &c = contacts_.at(row);
    map.insert("name", c.name);
    map.insert("affiliation", c.affiliation);
    map.insert("website", c.website);
    map.insert("phone", c.phone);
    map.insert("email", c.email);
    map.insert("zoom", c.zoom);
    map.insert("photo", c.photo);
    return map;
}

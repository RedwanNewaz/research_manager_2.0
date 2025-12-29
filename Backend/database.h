#ifndef DATABASE_H
#define DATABASE_H
#include <memory>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlRecord>
#include <QSharedDataPointer>
#include <QFileInfo>
#include <QDir>

class DatabaseManager;

typedef std::shared_ptr<DatabaseManager> DbmPtr;
class DatabaseManager: public std::enable_shared_from_this<DatabaseManager>{
public:
    DatabaseManager(const QString& connection_name)
    {
       db_ = QSqlDatabase::addDatabase("QSQLITE", connection_name);

    }

    explicit DatabaseManager(const QString& connection_name, const QString& db_path)
    {
        db_ = QSqlDatabase::addDatabase("QSQLITE", connection_name);
        connect(db_path);
    }

    DbmPtr getSharedPtr()
    {
        return shared_from_this();
    }

    QStringList getHeaderList(const QString& tableName) const
    {
        QSqlQuery query(db_);
        QString selectSql =  "SELECT * FROM " + tableName;
        QStringList result;

        if (!query.exec(selectSql)) {
            qWarning() << "Error: Failed to execute query:" << query.lastError().text();
            return result;
        }

        QSqlRecord record = query.record();


        for (int i = 0; i < record.count(); ++i) {
            QString columnName = record.fieldName(i);
            // qDebug() << "Index" << i << "is named:" << columnName;
            result << columnName;
        }
        return result;
    }

    QSqlQuery getBinder(const QString& cmd)
    {
        QSqlQuery query(db_);
        query.prepare(cmd);
        return query;
    }



    bool connect(const QString& db_path)
    {
        // Ensure the directory exists for the database file
        QFileInfo fileInfo(db_path);
        QDir dir = fileInfo.dir();
        if (!dir.exists()) {
            if (!dir.mkpath(".")) {
                qWarning() << "Error: Failed to create directory for database:" << dir.path();
                return false;
            }
            qDebug() << "Created directory for database:" << dir.path();
        }

        db_.setDatabaseName(db_path);

        if (!db_.open()) {
            qWarning() << "Error: Failed to connect to database:" << db_.lastError().text();
            return false;
        }
        return true;
    }

    QStringList queryRow(const QString& selectSql)
    {
        QStringList result;
        QSqlQuery query(db_);

        if (!query.exec(selectSql)) {
            qWarning() << "Error: Failed to execute query:" << query.lastError().text();
            return result;
        }


        // --- Fix applied here ---
        // Use QSqlRecord::count() to get the number of columns
        int columnCount = query.record().count();
        // ------------------------

        // Note: This function name 'queryRow' suggests it should only handle one row.
        // However, the 'while (query.next())' loop will read ALL returned rows.
        // If you only want the FIRST row, replace 'while' with 'if'.

        while (query.next()) {
            for(int i = 0; i < columnCount; ++i) { // Use the correct column count
                result << query.value(i).toString();
            }
            // If you only intend to read the first row, uncomment the line below:
            // break;
        }


        return result;
    }


    const QSqlDatabase& database() const { return db_; }
    QSqlDatabase& database() { return db_; }
    QString connectionName() const { return db_.connectionName(); }





    bool createTable(const QString& sqlCmd)
    {
        QSqlQuery query(db_);

        if (!query.exec(sqlCmd)) {
            qWarning() << "Failed to create project table:" << query.lastError().text();
            return false;
        }
        return true;
    }

    bool deleteItem(const QString& sqlCmd)
    {
        QSqlQuery query(db_);

        if (!query.exec(sqlCmd)) {
            qWarning() << "Failed to update project description:" << query.lastError().text();
            return false;
        }
        return true;
    }



    int queryCount(const QString& topic, const QString& table, const QString& constraint)
    {
        QString selectSql = QString("SELECT COUNT(%1) FROM %2 WHERE %3").arg(topic, table, constraint);
        QSqlQuery query(db_);

        if (!query.exec(selectSql)) {
            qWarning() << "Error: Failed to execute query:" << query.lastError().text();
            return -1;
        }

        while (query.next()) {
            return query.value(0).toInt();
        }

        return -1;

    }


    inline bool initializeDatabase() {

        QSqlQuery query(db_);

        // Create projects table first (referenced by other tables)
        if (!query.exec(R"(
            CREATE TABLE IF NOT EXISTS "projects" (
                "id" INTEGER NOT NULL,
                "name" VARCHAR NOT NULL,
                "description" VARCHAR,
                "category_id" INTEGER,
                PRIMARY KEY("id")
            )
        )")) {
            qDebug() << "Error creating projects table:" << query.lastError().text();
            return false;
        }

        // Create categories table
        if (!query.exec(R"(
            CREATE TABLE IF NOT EXISTS "categories" (
                "id" INTEGER,
                "name" TEXT,
                "year_id" INTEGER
            )
        )")) {
            qDebug() << "Error creating categories table:" << query.lastError().text();
            return false;
        }

        // Create calendars table
        if (!query.exec(R"(
            CREATE TABLE IF NOT EXISTS "calendars" (
                "id" INTEGER NOT NULL,
                "timestamp" DATETIME NOT NULL,
                "event" VARCHAR(200) NOT NULL,
                "description" TEXT,
                "project_id" INTEGER NOT NULL,
                PRIMARY KEY("id"),
                FOREIGN KEY("project_id") REFERENCES "projects"("id")
            )
        )")) {
            qDebug() << "Error creating calendars table:" << query.lastError().text();
            return false;
        }

        // Create contacts table
        if (!query.exec(R"(
            CREATE TABLE IF NOT EXISTS "contacts" (
                "id" INTEGER NOT NULL,
                "name" VARCHAR(200) NOT NULL,
                "affiliation" VARCHAR(200),
                "website" VARCHAR(500),
                "phone" VARCHAR(50),
                "email" VARCHAR(200),
                "zoom" VARCHAR(200),
                "photo" VARCHAR(500),
                "project_id" INTEGER NOT NULL,
                PRIMARY KEY("id"),
                FOREIGN KEY("project_id") REFERENCES "projects"("id")
            )
        )")) {
            qDebug() << "Error creating contacts table:" << query.lastError().text();
            return false;
        }

        // Create links table
        if (!query.exec(R"(
            CREATE TABLE IF NOT EXISTS "links" (
                "id" INTEGER NOT NULL,
                "url" VARCHAR(500) NOT NULL,
                "website" VARCHAR(200),
                "description" TEXT,
                "project_id" INTEGER NOT NULL,
                PRIMARY KEY("id"),
                FOREIGN KEY("project_id") REFERENCES "projects"("id")
            )
        )")) {
            qDebug() << "Error creating links table:" << query.lastError().text();
            return false;
        }

        // Create references table
        if (!query.exec(R"(
            CREATE TABLE IF NOT EXISTS "references" (
                "id" INTEGER NOT NULL,
                "title" VARCHAR(300) NOT NULL,
                "bibtex" TEXT,
                "project_id" INTEGER NOT NULL,
                PRIMARY KEY("id"),
                FOREIGN KEY("project_id") REFERENCES "projects"("id")
            )
        )")) {
            qDebug() << "Error creating references table:" << query.lastError().text();
            return false;
        }

        // Create tasks table
        if (!query.exec(R"(
            CREATE TABLE IF NOT EXISTS "tasks" (
                "id" INTEGER NOT NULL,
                "title" VARCHAR(200) NOT NULL,
                "description" TEXT,
                "timestamp" DATETIME,
                "pending" BOOLEAN,
                "project_id" INTEGER NOT NULL,
                PRIMARY KEY("id"),
                FOREIGN KEY("project_id") REFERENCES "projects"("id")
            )
        )")) {
            qDebug() << "Error creating tasks table:" << query.lastError().text();
            return false;
        }

        qDebug() << "Database initialized successfully!";
        return true;
    }


private:
    QSqlDatabase db_;
};


inline bool initializeDatabase(const QString &dbPath) {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "newResearch");
    db.setDatabaseName(dbPath);

    if (!db.open()) {
        qDebug() << "Error: Failed to open database:" << db.lastError().text();
        return false;
    }

    QSqlQuery query(db);

    // Create projects table first (referenced by other tables)
    if (!query.exec(R"(
            CREATE TABLE IF NOT EXISTS "projects" (
                "id" INTEGER NOT NULL,
                "name" VARCHAR NOT NULL,
                "description" VARCHAR,
                "category_id" INTEGER,
                PRIMARY KEY("id")
            )
        )")) {
        qDebug() << "Error creating projects table:" << query.lastError().text();
        return false;
    }

    // Create categories table
    if (!query.exec(R"(
            CREATE TABLE IF NOT EXISTS "categories" (
                "id" INTEGER,
                "name" TEXT,
                "year_id" INTEGER
            )
        )")) {
        qDebug() << "Error creating categories table:" << query.lastError().text();
        return false;
    }

    // Create calendars table
    if (!query.exec(R"(
            CREATE TABLE IF NOT EXISTS "calendars" (
                "id" INTEGER NOT NULL,
                "timestamp" DATETIME NOT NULL,
                "event" VARCHAR(200) NOT NULL,
                "description" TEXT,
                "project_id" INTEGER NOT NULL,
                PRIMARY KEY("id"),
                FOREIGN KEY("project_id") REFERENCES "projects"("id")
            )
        )")) {
        qDebug() << "Error creating calendars table:" << query.lastError().text();
        return false;
    }

    // Create contacts table
    if (!query.exec(R"(
            CREATE TABLE IF NOT EXISTS "contacts" (
                "id" INTEGER NOT NULL,
                "name" VARCHAR(200) NOT NULL,
                "affiliation" VARCHAR(200),
                "website" VARCHAR(500),
                "phone" VARCHAR(50),
                "email" VARCHAR(200),
                "zoom" VARCHAR(200),
                "photo" VARCHAR(500),
                "project_id" INTEGER NOT NULL,
                PRIMARY KEY("id"),
                FOREIGN KEY("project_id") REFERENCES "projects"("id")
            )
        )")) {
        qDebug() << "Error creating contacts table:" << query.lastError().text();
        return false;
    }

    // Create links table
    if (!query.exec(R"(
            CREATE TABLE IF NOT EXISTS "links" (
                "id" INTEGER NOT NULL,
                "url" VARCHAR(500) NOT NULL,
                "website" VARCHAR(200),
                "description" TEXT,
                "project_id" INTEGER NOT NULL,
                PRIMARY KEY("id"),
                FOREIGN KEY("project_id") REFERENCES "projects"("id")
            )
        )")) {
        qDebug() << "Error creating links table:" << query.lastError().text();
        return false;
    }

    // Create references table
    if (!query.exec(R"(
            CREATE TABLE IF NOT EXISTS "references" (
                "id" INTEGER NOT NULL,
                "title" VARCHAR(300) NOT NULL,
                "bibtex" TEXT,
                "project_id" INTEGER NOT NULL,
                PRIMARY KEY("id"),
                FOREIGN KEY("project_id") REFERENCES "projects"("id")
            )
        )")) {
        qDebug() << "Error creating references table:" << query.lastError().text();
        return false;
    }

    // Create tasks table
    if (!query.exec(R"(
            CREATE TABLE IF NOT EXISTS "tasks" (
                "id" INTEGER NOT NULL,
                "title" VARCHAR(200) NOT NULL,
                "description" TEXT,
                "timestamp" DATETIME,
                "pending" BOOLEAN,
                "project_id" INTEGER NOT NULL,
                PRIMARY KEY("id"),
                FOREIGN KEY("project_id") REFERENCES "projects"("id")
            )
        )")) {
        qDebug() << "Error creating tasks table:" << query.lastError().text();
        return false;
    }

    qDebug() << "Database initialized successfully!";
    return true;
}




#endif // DATABASE_H

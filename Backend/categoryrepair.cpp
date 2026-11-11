#include "categoryrepair.h"

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QSet>
#include <QSqlQuery>
#include <QSqlError>

namespace categoryrepair {
namespace {

/**
 * @brief Reduce a category name to a comparable key.
 *
 * Case, punctuation and the "&"/"and" spelling are all irrelevant when
 * deciding whether two databases mean the same category.
 */
QString normalize(const QString &name)
{
    QString key = name.trimmed().toLower();
    key.replace(QLatin1Char('&'), QLatin1String(" and "));
    QString compact;
    compact.reserve(key.size());
    for (const QChar &ch : key) {
        if (ch.isLetterOrNumber())
            compact.append(ch);
        else if (!compact.isEmpty() && compact.back() != QLatin1Char(' '))
            compact.append(QLatin1Char(' '));
    }
    return compact.trimmed();
}

/**
 * @brief Every spelling of a canonical category seen in the wild.
 *
 * The first entry of each list is the canonical name itself; the rest are the
 * names older builds of the application created.
 */
const QList<QStringList> &synonymGroups()
{
    static const QList<QStringList> groups = {
        { canonical::researchProjects(), QStringLiteral("Research"), QStringLiteral("Projects"),
          QStringLiteral("Research Project") },
        { canonical::publications(), QStringLiteral("papers"), QStringLiteral("paper"),
          QStringLiteral("publication"), QStringLiteral("manuscripts") },
        { canonical::presentations(), QStringLiteral("seminars"), QStringLiteral("seminar"),
          QStringLiteral("talks"), QStringLiteral("presentation") },
        { canonical::grants(), QStringLiteral("Grants and Funding"), QStringLiteral("proposals"),
          QStringLiteral("proposal"), QStringLiteral("grants"), QStringLiteral("grant"),
          QStringLiteral("funding") },
        { canonical::teaching(), QStringLiteral("teachings"), QStringLiteral("courses"),
          QStringLiteral("coursework"), QStringLiteral("course") },
        { canonical::service(), QStringLiteral("services") }
    };
    return groups;
}

/** @brief The canonical name @p name belongs to, or @p name itself. */
QString canonicalNameOf(const QString &name)
{
    const QString key = normalize(name);
    if (key.isEmpty())
        return QString();

    for (const QStringList &group : synonymGroups()) {
        for (const QString &variant : group) {
            if (normalize(variant) == key)
                return group.first();
        }
    }
    return name.trimmed();
}

/** @brief Names that mean the same category as @p canonicalName. */
QStringList synonymsOf(const QString &canonicalName)
{
    const QString key = normalize(canonicalName);
    for (const QStringList &group : synonymGroups()) {
        for (const QString &variant : group) {
            if (normalize(variant) == key)
                return group;
        }
    }
    return { canonicalName };
}

/** @brief All categories of a research database, as id -> name. */
QHash<int, QString> loadCategories(DatabaseManager &db)
{
    QHash<int, QString> result;
    QSqlQuery query(db.database());
    if (!query.exec(QStringLiteral("SELECT id, name FROM categories"))) {
        qWarning() << "[categoryrepair] Could not read categories:" << query.lastError().text();
        return result;
    }
    while (query.next())
        result.insert(query.value(0).toInt(), query.value(1).toString());
    return result;
}

/** @brief Immediate subdirectory names of @p path, empty when it does not exist. */
QSet<QString> subfolderNames(const QString &path)
{
    QSet<QString> names;
    QDir dir(path);
    if (!dir.exists())
        return names;

    const QFileInfoList entries = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QFileInfo &entry : entries)
        names.insert(entry.fileName().toLower());
    return names;
}

/** @brief One template: the folders it creates and the category it files under. */
struct TemplateInfo {
    QString name;
    QSet<QString> items;     ///< lower-cased folder names
    QString categoryName;    ///< canonical category name
};

/** @brief Read every template, its folders and its category, from the config database. */
QList<TemplateInfo> loadTemplates(DatabaseManager &configDb)
{
    QList<TemplateInfo> templates;

    QSqlQuery registry(configDb.database());
    if (!registry.exec(QStringLiteral("SELECT items, category_id FROM Template"))) {
        qWarning() << "[categoryrepair] Could not read templates:" << registry.lastError().text();
        return templates;
    }

    struct Row { QString name; int categoryId; };
    QList<Row> rows;
    while (registry.next())
        rows.append({ registry.value(0).toString(), registry.value(1).toInt() });

    for (const Row &row : rows) {
        if (row.name.trimmed().isEmpty())
            continue;

        TemplateInfo info;
        info.name = row.name;
        info.categoryName = categoryNameForId(configDb, row.categoryId);

        // Template names may contain spaces ("Invited Talk"), so the table name
        // has to be quoted.
        QSqlQuery items(configDb.database());
        items.prepare(QStringLiteral("SELECT items FROM \"%1\"").arg(row.name));
        if (!items.exec()) {
            qWarning() << "[categoryrepair] Could not read template" << row.name
                       << ":" << items.lastError().text();
            continue;
        }
        while (items.next())
            info.items.insert(items.value(0).toString().trimmed().toLower());

        if (!info.items.isEmpty())
            templates.append(info);
    }

    return templates;
}

/** @brief Copy @p path next to itself with a timestamp suffix. */
bool backupDatabaseFile(const QString &path)
{
    if (path.isEmpty() || !QFile::exists(path))
        return false;

    const QString stamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss"));
    const QString backup = QStringLiteral("%1.bak-%2").arg(path, stamp);
    if (QFile::exists(backup))
        return true;

    if (!QFile::copy(path, backup)) {
        qWarning() << "[categoryrepair] Could not back up" << path << "to" << backup;
        return false;
    }
    qInfo() << "[categoryrepair] Backed up" << path << "to" << backup;
    return true;
}

/** @brief Has this database already been repaired? */
bool alreadyRepaired(DatabaseManager &db)
{
    QSqlQuery query(db.database());
    if (!query.exec(QStringLiteral("CREATE TABLE IF NOT EXISTS app_meta ("
                                   "key TEXT PRIMARY KEY, value TEXT)"))) {
        qWarning() << "[categoryrepair] Could not create app_meta:" << query.lastError().text();
        return false;
    }

    query.prepare(QStringLiteral("SELECT value FROM app_meta WHERE key = :key"));
    query.bindValue(QStringLiteral(":key"), QStringLiteral("category_repair"));
    if (query.exec() && query.next())
        return !query.value(0).toString().isEmpty();
    return false;
}

/** @brief Record that this database has been repaired. */
void markRepaired(DatabaseManager &db, int changed)
{
    QSqlQuery query(db.database());
    query.prepare(QStringLiteral("INSERT OR REPLACE INTO app_meta (key, value) VALUES (:key, :value)"));
    query.bindValue(QStringLiteral(":key"), QStringLiteral("category_repair"));
    query.bindValue(QStringLiteral(":value"),
                    QStringLiteral("1 @ %1 (%2 changed)")
                        .arg(QDateTime::currentDateTime().toString(Qt::ISODate))
                        .arg(changed));
    if (!query.exec())
        qWarning() << "[categoryrepair] Could not record repair:" << query.lastError().text();
}

} // namespace

bool ensureConfigCategories(DatabaseManager &configDb)
{
    QSqlQuery query(configDb.database());
    if (!query.exec(QStringLiteral("CREATE TABLE IF NOT EXISTS Categories ("
                                   "id INTEGER PRIMARY KEY, name TEXT NOT NULL)"))) {
        qWarning() << "[categoryrepair] Could not create Categories:" << query.lastError().text();
        return false;
    }

    // These ids are the ones the existing templates were written with, so the
    // names below are what those templates have always meant. Ids 1 and 6 both
    // stood for general research work in different builds.
    static const QList<QPair<int, QString>> seed = {
        { 1, canonical::researchProjects() },
        { 2, canonical::grants() },
        { 3, canonical::publications() },
        { 4, canonical::presentations() },
        { 5, canonical::teaching() },
        { 6, canonical::researchProjects() },
        { 7, canonical::service() },
        { 8, canonical::presentations() }  // used by the "Invited Talk" template
    };

    for (const auto &entry : seed) {
        QSqlQuery insert(configDb.database());
        insert.prepare(QStringLiteral("INSERT OR IGNORE INTO Categories (id, name) VALUES (:id, :name)"));
        insert.bindValue(QStringLiteral(":id"), entry.first);
        insert.bindValue(QStringLiteral(":name"), entry.second);
        if (!insert.exec())
            qWarning() << "[categoryrepair] Could not seed category" << entry.second
                       << ":" << insert.lastError().text();
    }

    // A template saved with no category (0, or NULL from an older build) has
    // nothing to say about where its projects belong, so give it the neutral
    // one rather than leaving projects unlabelled.
    QSqlQuery fix(configDb.database());
    fix.prepare(QStringLiteral("UPDATE Template SET category_id = :good "
                               "WHERE category_id IS NULL "
                               "   OR category_id NOT IN (SELECT id FROM Categories)"));
    fix.bindValue(QStringLiteral(":good"), 1); // Research Projects
    if (!fix.exec())
        qWarning() << "[categoryrepair] Could not re-point stray templates:" << fix.lastError().text();
    else if (fix.numRowsAffected() > 0)
        qInfo() << "[categoryrepair] Re-pointed" << fix.numRowsAffected()
                << "template(s) with an unknown category";

    return true;
}

QString categoryNameForId(DatabaseManager &configDb, int categoryId)
{
    if (categoryId > 0) {
        QSqlQuery query(configDb.database());
        query.prepare(QStringLiteral("SELECT name FROM Categories WHERE id = :id"));
        query.bindValue(QStringLiteral(":id"), categoryId);
        if (query.exec() && query.next()) {
            const QString name = query.value(0).toString().trimmed();
            if (!name.isEmpty())
                return canonicalNameOf(name);
        }
    }
    return canonical::researchProjects();
}

QString categoryForTemplate(DatabaseManager &configDb, const QString &templateName)
{
    if (templateName.trimmed().isEmpty())
        return canonical::researchProjects();

    QSqlQuery query(configDb.database());
    query.prepare(QStringLiteral("SELECT category_id FROM Template WHERE items = :name"));
    query.bindValue(QStringLiteral(":name"), templateName);
    if (query.exec() && query.next())
        return categoryNameForId(configDb, query.value(0).toInt());

    qWarning() << "[categoryrepair] Unknown template" << templateName
               << "- filing under" << canonical::researchProjects();
    return canonical::researchProjects();
}

int resolveCategoryId(DatabaseManager &researchDb, const QString &categoryName)
{
    const QString wanted = canonicalNameOf(categoryName).isEmpty()
                               ? canonical::researchProjects()
                               : canonicalNameOf(categoryName);

    const QHash<int, QString> existing = loadCategories(researchDb);

    // Prefer an exact canonical match, then any accepted spelling.
    QSet<QString> accepted;
    for (const QString &variant : synonymsOf(wanted))
        accepted.insert(normalize(variant));

    int fallback = -1;
    for (auto it = existing.cbegin(); it != existing.cend(); ++it) {
        const QString key = normalize(it.value());
        if (key == normalize(wanted))
            return it.key();
        if (accepted.contains(key))
            fallback = it.key();
    }
    if (fallback > 0)
        return fallback;

    // Not in this workspace yet - add it, keeping the year_id the other rows use.
    int nextId = 1;
    int yearId = 0;
    QSqlQuery probe(researchDb.database());
    if (probe.exec(QStringLiteral("SELECT COALESCE(MAX(id), 0) + 1, "
                                  "COALESCE((SELECT year_id FROM categories LIMIT 1), 0) "
                                  "FROM categories"))
        && probe.next()) {
        nextId = probe.value(0).toInt();
        yearId = probe.value(1).toInt();
    }

    QSqlQuery insert(researchDb.database());
    insert.prepare(QStringLiteral("INSERT INTO categories (id, name, year_id) "
                                  "VALUES (:id, :name, :year)"));
    insert.bindValue(QStringLiteral(":id"), nextId);
    insert.bindValue(QStringLiteral(":name"), wanted);
    insert.bindValue(QStringLiteral(":year"), yearId);
    if (!insert.exec()) {
        qWarning() << "[categoryrepair] Could not add category" << wanted
                   << ":" << insert.lastError().text();
        return -1;
    }

    qInfo() << "[categoryrepair] Added category" << wanted << "with id" << nextId;
    return nextId;
}

int repairProjectCategories(DatabaseManager &researchDb,
                            DatabaseManager &configDb,
                            const QString &workspaceDir,
                            bool force)
{
    if (!researchDb.database().isOpen()) {
        qWarning() << "[categoryrepair] Research database is not open";
        return 0;
    }

    if (!force && alreadyRepaired(researchDb)) {
        qInfo() << "[categoryrepair] Already repaired"
                << researchDb.database().databaseName() << "- skipping";
        return 0;
    }

    const QList<TemplateInfo> templates = loadTemplates(configDb);
    const QHash<int, QString> categories = loadCategories(researchDb);

    struct ProjectRow { int id; QString name; int categoryId; };
    QList<ProjectRow> projects;
    {
        QSqlQuery query(researchDb.database());
        if (!query.exec(QStringLiteral("SELECT id, name, category_id FROM projects"))) {
            qWarning() << "[categoryrepair] Could not read projects:" << query.lastError().text();
            return 0;
        }
        while (query.next())
            projects.append({ query.value(0).toInt(),
                              query.value(1).toString(),
                              query.value(2).isNull() ? -1 : query.value(2).toInt() });
    }

    // Decide every change before touching the database, so the backup is only
    // made when there is something to change.
    struct Change { int projectId; QString projectName; QString from; QString to; };
    QList<Change> changes;

    for (const ProjectRow &project : projects) {
        const QString currentName = categories.value(project.categoryId);
        const bool currentIsValid = !currentName.trimmed().isEmpty();

        // What do the project's folders say it was created from?
        QString inferred;
        if (!workspaceDir.isEmpty() && !templates.isEmpty()) {
            const QSet<QString> folders = subfolderNames(QDir(workspaceDir).filePath(project.name));
            if (!folders.isEmpty()) {
                double bestScore = 0.0;
                int bestMatches = 0;
                for (const TemplateInfo &tpl : templates) {
                    const int matches = (tpl.items & folders).size();
                    const double score = double(matches) / double(tpl.items.size());
                    if (score > bestScore || (qFuzzyCompare(score, bestScore) && matches > bestMatches)) {
                        bestScore = score;
                        bestMatches = matches;
                        inferred = tpl.categoryName;
                    }
                }
                // Enough of the template has to be present to be convincing.
                if (bestScore < 0.6 || bestMatches < 3)
                    inferred.clear();
            }
        }

        QString target;
        if (!inferred.isEmpty())
            target = inferred;
        else if (!currentIsValid)
            target = canonical::researchProjects(); // nothing to go on, but it needs a label
        else
            continue;                               // leave a valid category alone

        if (currentIsValid && canonicalNameOf(currentName) == canonicalNameOf(target))
            continue;

        changes.append({ project.id, project.name,
                         currentIsValid ? currentName : QStringLiteral("(none)"),
                         target });
    }

    if (changes.isEmpty()) {
        qInfo() << "[categoryrepair] Nothing to repair in"
                << researchDb.database().databaseName();
        markRepaired(researchDb, 0);
        return 0;
    }

    backupDatabaseFile(researchDb.database().databaseName());

    int applied = 0;
    for (const Change &change : changes) {
        const int categoryId = resolveCategoryId(researchDb, change.to);
        if (categoryId <= 0)
            continue;

        QSqlQuery update(researchDb.database());
        update.prepare(QStringLiteral("UPDATE projects SET category_id = :cat WHERE id = :id"));
        update.bindValue(QStringLiteral(":cat"), categoryId);
        update.bindValue(QStringLiteral(":id"), change.projectId);
        if (!update.exec()) {
            qWarning() << "[categoryrepair] Could not update" << change.projectName
                       << ":" << update.lastError().text();
            continue;
        }

        ++applied;
        qInfo().noquote() << QStringLiteral("[categoryrepair] %1: %2 -> %3")
                                 .arg(change.projectName, change.from, change.to);
    }

    markRepaired(researchDb, applied);
    qInfo() << "[categoryrepair] Repaired" << applied << "project(s) in"
            << researchDb.database().databaseName();
    return applied;
}

} // namespace categoryrepair

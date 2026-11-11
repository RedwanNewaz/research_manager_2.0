#ifndef CATEGORYREPAIR_H
#define CATEGORYREPAIR_H

#include <QString>
#include <QStringList>
#include "database.h"

/**
 * @namespace categoryrepair
 * @brief Keeps project categories consistent across workspaces.
 *
 * Every workspace has its own research database, and those databases were
 * created by different builds of the application:
 *
 *   - older ones hold categories named "proposals", "papers", "seminars",
 *     "teachings", "Research" with ids 2..6;
 *   - newer ones are seeded with "Research Projects", "Publications",
 *     "Presentations", "Grants & Funding", "Teaching", "Service" with ids 1..6.
 *
 * The templates, however, live in the shared configuration database and only
 * store a numeric @c category_id. The same number therefore means different
 * things in different workspaces, which is how grant projects ended up filed
 * under "Publications" and how projects created from a template with no usable
 * category ended up with no label at all.
 *
 * The fix is to treat the category *name* as the identity:
 *
 *   1. The configuration database gets a @c Categories table that gives every
 *      template category id a canonical name (::ensureConfigCategories).
 *   2. New projects are filed by name, reusing whatever the workspace already
 *      calls that category (::resolveCategoryId).
 *   3. Existing databases are repaired once, inferring each project's real
 *      category from the template folders it actually contains
 *      (::repairProjectCategories).
 */
namespace categoryrepair {

/** @brief Canonical category names used when filing a new project. */
namespace canonical {
inline QString researchProjects() { return QStringLiteral("Research Projects"); }
inline QString publications()     { return QStringLiteral("Publications"); }
inline QString presentations()    { return QStringLiteral("Presentations"); }
inline QString grants()           { return QStringLiteral("Grants & Funding"); }
inline QString teaching()         { return QStringLiteral("Teaching"); }
inline QString service()          { return QStringLiteral("Service"); }
} // namespace canonical

/**
 * @brief Create and seed the @c Categories table in the configuration database.
 *
 * The table maps the @c category_id stored against each template to a
 * canonical name. The seeded ids keep the meaning the existing templates were
 * written with, so no template needs to be re-pointed.
 *
 * @param configDb the shared configuration database.
 * @return true when the table exists and is populated.
 */
bool ensureConfigCategories(DatabaseManager &configDb);

/**
 * @brief Canonical name for a template's @c category_id.
 * @param configDb the shared configuration database.
 * @param categoryId value stored in @c Template.category_id.
 * @return the canonical name, or "Research Projects" when @p categoryId is
 *         unknown or invalid.
 */
QString categoryNameForId(DatabaseManager &configDb, int categoryId);

/**
 * @brief Canonical name of the template @p templateName files projects under.
 * @param configDb the shared configuration database.
 * @param templateName a row of @c Template.items, e.g. "NSF".
 */
QString categoryForTemplate(DatabaseManager &configDb, const QString &templateName);

/**
 * @brief Find - or create - the category called @p categoryName in a workspace.
 *
 * Matching ignores case and accepts the legacy spellings, so "Grants &
 * Funding" resolves to an existing "proposals" row instead of adding a
 * near-duplicate category to an older workspace.
 *
 * @param researchDb the workspace's research database.
 * @param categoryName a canonical category name.
 * @return the category id, or -1 if it could neither be found nor created.
 */
int resolveCategoryId(DatabaseManager &researchDb, const QString &categoryName);

/**
 * @brief Repair project categories in one workspace, once.
 *
 * A project's folder contains the subfolders of the template it was created
 * from, which makes the template - and therefore the category - recoverable
 * from disk. Projects whose folder is missing, or whose folders match no
 * template well enough, keep their category unless it points at a category
 * that does not exist.
 *
 * The database file is copied to "<name>.bak-<timestamp>" before the first
 * change, and the repair is recorded in an @c app_meta table so it does not
 * run again and overwrite later manual edits.
 *
 * @param researchDb   the workspace's research database.
 * @param configDb     the shared configuration database (source of templates).
 * @param workspaceDir the workspace folder holding the project directories.
 * @param force        run again even if this database was repaired before.
 * @return the number of projects whose category was changed.
 */
int repairProjectCategories(DatabaseManager &researchDb,
                            DatabaseManager &configDb,
                            const QString &workspaceDir,
                            bool force = false);

} // namespace categoryrepair

#endif // CATEGORYREPAIR_H

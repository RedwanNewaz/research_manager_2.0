#ifndef APPPATHS_H
#define APPPATHS_H

#include <QString>
#include <QStringList>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDebug>

/**
 * @namespace apppaths
 * @brief Canonical, platform-correct locations for ResearchManager user data.
 *
 * The application stores its databases in a per-user, writable location so that
 * no manual configuration is required on a fresh install:
 *
 *   Windows : %APPDATA%\ResearchManager            (~\AppData\Roaming\ResearchManager)
 *   Linux   : $XDG_CONFIG_HOME/ResearchManager     (~/.config/ResearchManager)
 *   macOS   : ~/Library/Application Support/ResearchManager
 *
 * Users may still point the application at a different database from Settings;
 * that choice is stored in QSettings and always wins over these defaults.
 */
namespace apppaths {

inline QString appName()
{
    return QStringLiteral("ResearchManager");
}

/**
 * @brief Per-user configuration directory for the application.
 */
inline QString configDir()
{
    QString base;
#if defined(Q_OS_ANDROID)
    // Android sandboxes each app; AppDataLocation is already app-specific.
    base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (base.isEmpty())
        base = QDir::homePath() + QStringLiteral("/.config/") + appName();
    return QDir::cleanPath(base);
#elif defined(Q_OS_WIN)
    base = qEnvironmentVariable("APPDATA");
    if (base.isEmpty())
        base = QDir::homePath() + QStringLiteral("/AppData/Roaming");
#elif defined(Q_OS_MACOS)
    base = QDir::homePath() + QStringLiteral("/Library/Application Support");
#else
    base = qEnvironmentVariable("XDG_CONFIG_HOME");
    if (base.isEmpty())
        base = QDir::homePath() + QStringLiteral("/.config");
#endif
    return QDir::cleanPath(QDir(base).filePath(appName()));
}

/**
 * @brief Create the configuration directory if it does not exist yet.
 * @return true if the directory exists (or was created) and is usable.
 */
inline bool ensureConfigDir()
{
    QDir dir(configDir());
    if (dir.exists())
        return true;

    if (!dir.mkpath(QStringLiteral("."))) {
        qWarning() << "[apppaths] Failed to create configuration directory:" << dir.path();
        return false;
    }
    qInfo() << "[apppaths] Created configuration directory:" << dir.path();
    return true;
}

/** @brief Default path of the shared configuration database. */
inline QString configDatabasePath()
{
    return QDir(configDir()).filePath(QStringLiteral("common_config.db"));
}

/** @brief Default path of the default workspace's research database. */
inline QString researchDatabasePath()
{
    return QDir(configDir()).filePath(QStringLiteral("research.db"));
}

/** @brief Default folder for the "Default" workspace's files. */
inline QString defaultWorkspaceDir()
{
    return QDir::cleanPath(QDir::homePath() + QStringLiteral("/ResearchWorkspace"));
}

/**
 * @brief Historic locations where common_config.db may already live.
 *
 * Searched once, in order, when the canonical database does not exist yet, so
 * that users upgrading from an earlier build keep their data.
 */
inline QStringList legacyConfigDatabasePaths()
{
    const QString appDir = QCoreApplication::applicationDirPath();
    const QString cwd = QDir::currentPath();
    return QStringList{
        QDir::cleanPath(appDir + QStringLiteral("/common_config.db")),
        QDir::cleanPath(appDir + QStringLiteral("/../../Test/common_config.db")),
        QDir::cleanPath(cwd + QStringLiteral("/common_config.db")),
        QDir::cleanPath(cwd + QStringLiteral("/../../Test/common_config.db"))
    };
}

/** @brief Historic locations where research.db may already live. */
inline QStringList legacyResearchDatabasePaths()
{
    const QString appDir = QCoreApplication::applicationDirPath();
    const QString cwd = QDir::currentPath();
    return QStringList{
        QDir::cleanPath(appDir + QStringLiteral("/research.db")),
        QDir::cleanPath(cwd + QStringLiteral("/research.db"))
    };
}

/**
 * @brief Copy the first existing legacy database to @p target, once.
 *
 * Does nothing if @p target already exists. The copy is made writable, because
 * the source may have been shipped inside a read-only installation directory.
 *
 * @return true if a legacy database was migrated.
 */
inline bool migrateLegacyDatabase(const QString &target, const QStringList &candidates)
{
    if (target.isEmpty() || QFile::exists(target))
        return false;

    const QString targetAbs = QFileInfo(target).absoluteFilePath();

    for (const QString &candidate : candidates) {
        if (candidate.isEmpty() || !QFile::exists(candidate))
            continue;
        if (QFileInfo(candidate).absoluteFilePath() == targetAbs)
            continue;

        if (!QFile::copy(candidate, target)) {
            qWarning() << "[apppaths] Failed to migrate database from" << candidate << "to" << target;
            continue;
        }

        QFile::setPermissions(target,
                              QFile::ReadOwner | QFile::WriteOwner |
                              QFile::ReadUser  | QFile::WriteUser);
        qInfo() << "[apppaths] Migrated existing database from" << candidate << "to" << target;
        return true;
    }
    return false;
}

} // namespace apppaths

#endif // APPPATHS_H

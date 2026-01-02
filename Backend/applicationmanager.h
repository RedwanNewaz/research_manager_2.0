#ifndef APPLICATIONMANAGER_H
#define APPLICATIONMANAGER_H

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QFileSystemModel>
#include <QSettings>
#include <memory>

// Forward declarations
class DatabaseManager;
class WorkspaceModel;
class TemplateModel;
class AiConfig;

namespace project {
    class ProjectPage;
    class TaskManger;
    class LinkViewer;
    class ContactsModel;
    class FileSystemModelWrapper;
    class FileListViewer;
    class CalendarView;
    class DeadlineModel;
    class FileIconProvider;
    class FileDownloader;

}

namespace collab {
    class CollaboratorModel;
    class MessageViewer;
}

namespace homepage {
    class ProjectView;
    class CreateProject;
}

/**
 * @brief Manages application settings including database paths
 */
class SettingsManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString configDatabasePath READ getConfigDatabasePath NOTIFY configDatabasePathChanged)

public:
    explicit SettingsManager(QObject *parent = nullptr);
    
    QString getConfigDatabasePath() const;
    
public slots:
    void setConfigDatabasePath(const QString &path);
    QString browseForDatabase(QObject *parentWindow = nullptr);

signals:
    void configDatabasePathChanged(const QString &path);

private:
    QSettings m_settings;
};

/**
 * @brief Manages the application lifecycle, database connections, and model initialization
 */
class ApplicationManager : public QObject
{
    Q_OBJECT

public:
    explicit ApplicationManager(QGuiApplication *app, QObject *parent = nullptr);
    ~ApplicationManager();

    /**
     * @brief Initialize the application and load QML
     * @return true if initialization successful, false otherwise
     */
    bool initialize();

    /**
     * @brief Start the application event loop
     * @return Application exit code
     */
    int run();

public slots:
    QString browseForConfigDatabase();

private:
    // Initialization methods
    void setupEngine();
    void setupDatabases();
    void initializeModels();
    void setupSignalConnections();
    void registerContextProperties();
    bool loadQmlApplication();
    void setupCleanupHandlers();

    // Helper methods
    bool createResearchDatabase(const QString &dbPath);
    bool createConfigDatabase(const QString &configPath, const QString &researchDbPath);
    bool ensureConfigDatabaseTables(const QString &configPath);
    QString getConfigDatabasePath();
    void showStartupConfigDialog();
    
    // Member variables
    QGuiApplication *m_app;
    QQmlApplicationEngine *m_engine;
    QString m_appDir;
    SettingsManager *m_settingsManager;

    // Database managers
    std::shared_ptr<DatabaseManager> m_researchDb;
    std::shared_ptr<DatabaseManager> m_configDb;

    // Models (stack allocated)
    homepage::ProjectView *m_homepage;
    homepage::CreateProject *m_templateProject;
    project::ProjectPage *m_project;
    WorkspaceModel *m_wsModel;
    TemplateModel *m_templateModel;
    project::TaskManger *m_task;
    project::LinkViewer *m_lnModel;

    // Models (heap allocated)
    QFileSystemModel *m_fsModel;
    project::FileSystemModelWrapper *m_fsWrapper;
    project::FileListViewer *m_flModel;
    project::CalendarView *m_calModel;
    project::DeadlineModel *m_dlModel;
    project::FileDownloader *m_fileDownloader;
    project::ContactsModel *m_contactsModel;
    collab::CollaboratorModel *m_colModel;
    collab::MessageViewer *m_msgModel;
    AiConfig *m_aiConfig;
};

#endif // APPLICATIONMANAGER_H

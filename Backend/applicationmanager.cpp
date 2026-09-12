#include "applicationmanager.h"
#include "backend.h"
#include "templatemanager.h"
#include "apppaths.h"
#include <QFile>
#include <QDir>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QSettings>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QTimer>
#include <QFileInfo>
#include <QIcon>
#include <QUrl>
#include <QDate>

using namespace project;

// SettingsManager Implementation
SettingsManager::SettingsManager(QObject *parent)
    : QObject(parent)
    , m_settings("ResearchManager", "ResearchManager")
{
}

QString SettingsManager::getConfigDatabasePath() const
{
    const QString stored = m_settings.value("configDatabasePath", "").toString();
    return stored.isEmpty() ? apppaths::configDatabasePath() : stored;
}

QString SettingsManager::defaultConfigDatabasePath() const
{
    return apppaths::configDatabasePath();
}

void SettingsManager::setConfigDatabasePath(const QString &raw_path)
{
    QString path = raw_path.trimmed();
    if (path.isEmpty())
        return;

    // Accept values coming from QML FileDialog ("file:///C:/..." or "file:///home/...")
    if (path.startsWith(QLatin1String("file:")))
        path = QUrl(path).toLocalFile();

    path = QDir::cleanPath(QFileInfo(path).absoluteFilePath());

    if (path != m_settings.value("configDatabasePath", "").toString()) {
        m_settings.setValue("configDatabasePath", path);
        m_settings.sync();
        emit configDatabasePathChanged(path);
    }
}

void SettingsManager::resetConfigDatabasePathToDefault()
{
    setConfigDatabasePath(apppaths::configDatabasePath());
}

QString SettingsManager::browseForDatabase(QObject *parentWindow)
{
    // Get the parent widget if available
    QWidget *parent = qobject_cast<QWidget*>(parentWindow);
    
    QString selectedPath = QFileDialog::getOpenFileName(
        parent,
        "Select Config Database",
        apppaths::configDir(),
        "Database Files (*.db);;All Files (*)"
    );
    
    if (!selectedPath.isEmpty()) {
        setConfigDatabasePath(selectedPath);
    }
    
    return selectedPath;
}

ApplicationManager::ApplicationManager(QGuiApplication *app, QObject *parent)
    : QObject(parent)
    , m_app(app)
    , m_engine(nullptr)
    , m_homepage(nullptr)
    , m_templateProject(nullptr)
    , m_project(nullptr)
    , m_wsModel(nullptr)
    , m_templateModel(nullptr)
    , m_task(nullptr)
    , m_lnModel(nullptr)
    , m_fsModel(nullptr)
    , m_fsWrapper(nullptr)
    , m_flModel(nullptr)
    , m_calModel(nullptr)
    , m_dlModel(nullptr)
    , m_fileDownloader(nullptr)
    , m_contactsModel(nullptr)
    , m_aiConfig(nullptr)
    , m_settingsManager(nullptr)
{
    m_appDir = QCoreApplication::applicationDirPath();
    m_settingsManager = new SettingsManager(this);
}

ApplicationManager::~ApplicationManager()
{
    // Clean up heap-allocated models
    delete m_aiConfig;
    delete m_fileDownloader;
    delete m_contactsModel;
    delete m_dlModel;
    delete m_calModel;
    delete m_flModel;
    delete m_fsWrapper;
    delete m_fsModel;
    
    // Clean up stack-allocated models
    delete m_lnModel;
    delete m_task;
    delete m_templateModel;
    delete m_wsModel;
    delete m_project;
    delete m_templateProject;
    delete m_homepage;
    delete m_colModel;
    delete m_msgModel;
    
    delete m_engine;
}

bool ApplicationManager::initialize()
{
    // Resolve default (writable, per-user) database locations before anything
    // touches the databases. No manual configuration required.
    ensureDefaultConfiguration();
    
    setupEngine();
    setupDatabases();
    initializeModels();
    setupSignalConnections();
    registerContextProperties();
    
    if (!loadQmlApplication()) {
        return false;
    }
    
    setupCleanupHandlers();
    return true;
}

int ApplicationManager::run()
{
    return m_app->exec();
}

void ApplicationManager::setupEngine()
{
    m_engine = new QQmlApplicationEngine(this);

    // Register custom image provider for file icons
    m_engine->addImageProvider(QLatin1String("fileicon"), new project::FileIconProvider);

    // Setup import paths
    m_engine->addImportPath(m_appDir);
    m_engine->addImportPath(m_appDir + "/qml");
    m_engine->addImportPath(m_appDir + "/ResearchManager");

    // Set application icon for Linux and macOS builds
    #if !defined(Q_OS_WIN)
    QIcon appIcon(":/ResearchManager/ResearchManager/images/ResearchManager.png");
    if (!appIcon.isNull()) {
        m_app->setWindowIcon(appIcon);
        qInfo() << "[ApplicationManager] Set application icon from resources";
    } else {
        qWarning() << "[ApplicationManager] Failed to load application icon from resources";
    }
    #endif

    // Handle object creation failures
    QObject::connect(
        m_engine,
        &QQmlApplicationEngine::objectCreationFailed,
        m_app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection
    );
}

void ApplicationManager::setupDatabases()
{
    // --- Research database -------------------------------------------------
    // Lives in the per-user configuration directory so it is always writable,
    // even when the application itself is installed read-only.
    const QString dbPath = getResearchDatabasePath();

    if (!QFile::exists(dbPath)) {
        createResearchDatabase(dbPath);
    }

    m_researchDb = std::make_shared<DatabaseManager>("research");
    m_researchDb->connect(dbPath);

    // --- Config database ---------------------------------------------------
    const QString configPath = getConfigDatabasePath();

    // Ensure the containing directory exists
    QDir dir = QFileInfo(configPath).dir();
    if (!dir.exists() && !dir.mkpath(".")) {
        qWarning() << "[ApplicationManager] Could not create directory for config database:" << dir.path();
    }

    if (!QFile::exists(configPath)) {
        createConfigDatabase(configPath, dbPath);
    } else {
        // Ensure existing database has all required tables
        ensureConfigDatabaseTables(configPath);
    }

    m_configDb = std::make_shared<DatabaseManager>("config", configPath);
}

QString ApplicationManager::getResearchDatabasePath()
{
    QSettings settings("ResearchManager", "ResearchManager");
    const QString stored = settings.value("researchDatabasePath", "").toString();

    if (!stored.isEmpty() && QFileInfo(stored).isAbsolute() && QFile::exists(stored)) {
        return stored;
    }

    apppaths::ensureConfigDir();
    const QString defaultPath = apppaths::researchDatabasePath();

    // First run after an upgrade: bring along a database created by an older build.
    apppaths::migrateLegacyDatabase(defaultPath, apppaths::legacyResearchDatabasePaths());

    settings.setValue("researchDatabasePath", defaultPath);
    settings.sync();
    return defaultPath;
}

bool ApplicationManager::createResearchDatabase(const QString &dbPath)
{
    if (!initializeDatabase(dbPath)) {
        qWarning() << "Failed to initialize research database";
        return false;
    }
    return true;
}

bool ApplicationManager::createConfigDatabase(const QString &configPath, const QString &researchDbPath)
{
    QSqlDatabase configDb = QSqlDatabase::addDatabase("QSQLITE", "tempConfig");
    configDb.setDatabaseName(configPath);
    
    if (!configDb.open()) {
        qWarning() << "Failed to create config database:" << configDb.lastError().text();
        configDb = QSqlDatabase();
        QSqlDatabase::removeDatabase("tempConfig");
        return false;
    }

    // Scope all queries so they are destroyed before the connection is removed.
    {
    QSqlQuery query(configDb);
    query.exec("CREATE TABLE IF NOT EXISTS Workspace (name TEXT, database TEXT, year INTEGER, workspace TEXT, icon TEXT)");

    // Seed the Default workspace, and make sure its folder actually exists.
    const QString defaultWorkspaceDir = apppaths::defaultWorkspaceDir();
    QDir().mkpath(defaultWorkspaceDir);

    query.prepare("INSERT INTO Workspace (name, database, year, workspace, icon) "
                  "VALUES (:name, :database, :year, :workspace, :icon)");
    query.bindValue(":name", "Default");
    query.bindValue(":database", researchDbPath);
    query.bindValue(":year", QDate::currentDate().year());
    query.bindValue(":workspace", defaultWorkspaceDir);
    query.bindValue(":icon", "local-folder.svg");
    if (!query.exec()) {
        qWarning() << "[ApplicationManager] Failed to seed Default workspace:" << query.lastError().text();
    }
    
    // Create Contacts table
    query.exec("CREATE TABLE IF NOT EXISTS Contacts ("
               "name TEXT NOT NULL,"
               "affiliation TEXT,"
               "website TEXT,"
               "phone TEXT,"
               "email TEXT NOT NULL,"
               "zoom TEXT,"
               "photo TEXT,"
               "UNIQUE(name),"
               "PRIMARY KEY(name)"
               ")");
    
    // Create Template table
    query.exec("CREATE TABLE IF NOT EXISTS Template ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "items TEXT NOT NULL,"
               "category_id INTEGER"
               ")");
    } // end query scope

    configDb.close();
    configDb = QSqlDatabase();
    QSqlDatabase::removeDatabase("tempConfig");
    
    return true;
}

bool ApplicationManager::ensureConfigDatabaseTables(const QString &configPath)
{
    QSqlDatabase configDb = QSqlDatabase::addDatabase("QSQLITE", "tempMigration");
    configDb.setDatabaseName(configPath);
    
    if (!configDb.open()) {
        qWarning() << "Failed to open config database for migration:" << configDb.lastError().text();
        configDb = QSqlDatabase();
        QSqlDatabase::removeDatabase("tempMigration");
        return false;
    }

    // Scope all queries so they are destroyed before the connection is removed.
    {
    QSqlQuery query(configDb);
    
    // Create Template table if it doesn't exist
    if (!query.exec("CREATE TABLE IF NOT EXISTS Template ("
                   "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                   "items TEXT NOT NULL,"
                   "category_id INTEGER"
                   ")")) {
        qWarning() << "Failed to ensure Template table exists:" << query.lastError().text();
    }

    // Ensure other tables exist. Note the Workspace schema must match the one
    // used by createConfigDatabase(), including the "year" column.
    if (!query.exec("CREATE TABLE IF NOT EXISTS Workspace ("
                    "name TEXT, database TEXT, year INTEGER, workspace TEXT, icon TEXT)")) {
        qWarning() << "Failed to ensure Workspace table exists:" << query.lastError().text();
    }

    // Older builds created Workspace without the "year" column - add it in place.
    {
        QStringList workspaceColumns;
        QSqlQuery pragma(configDb);
        if (pragma.exec("PRAGMA table_info(Workspace)")) {
            while (pragma.next())
                workspaceColumns << pragma.value(1).toString();
        }
        if (!workspaceColumns.isEmpty() && !workspaceColumns.contains("year", Qt::CaseInsensitive)) {
            if (!query.exec("ALTER TABLE Workspace ADD COLUMN year INTEGER")) {
                qWarning() << "Failed to add 'year' column to Workspace:" << query.lastError().text();
            } else {
                qInfo() << "[ApplicationManager] Added missing 'year' column to Workspace table";
            }
        }
    }

    if (!query.exec("CREATE TABLE IF NOT EXISTS Contacts ("
                    "name TEXT NOT NULL,"
                    "affiliation TEXT,"
                    "website TEXT,"
                    "phone TEXT,"
                    "email TEXT NOT NULL,"
                    "zoom TEXT,"
                    "photo TEXT,"
                    "PRIMARY KEY(name)"
                    ")")) {
        qWarning() << "Failed to ensure Contacts table exists:" << query.lastError().text();
    }

    // A config database with no workspace at all leaves the UI empty; seed one.
    QSqlQuery countQuery(configDb);
    if (countQuery.exec("SELECT COUNT(*) FROM Workspace") && countQuery.next()
        && countQuery.value(0).toInt() == 0) {
        const QString defaultWorkspaceDir = apppaths::defaultWorkspaceDir();
        QDir().mkpath(defaultWorkspaceDir);

        QSqlQuery seed(configDb);
        seed.prepare("INSERT INTO Workspace (name, database, year, workspace, icon) "
                     "VALUES (:name, :database, :year, :workspace, :icon)");
        seed.bindValue(":name", "Default");
        seed.bindValue(":database", apppaths::researchDatabasePath());
        seed.bindValue(":year", QDate::currentDate().year());
        seed.bindValue(":workspace", defaultWorkspaceDir);
        seed.bindValue(":icon", "local-folder.svg");
        if (!seed.exec())
            qWarning() << "Failed to seed Default workspace:" << seed.lastError().text();
        else
            qInfo() << "[ApplicationManager] Seeded Default workspace in existing config database";
    }
    } // end query scope

    configDb.close();
    configDb = QSqlDatabase();
    QSqlDatabase::removeDatabase("tempMigration");
    
    qDebug() << "Config database migration completed";
    return true;
}

void ApplicationManager::initializeModels()
{
    // Homepage models
    m_homepage = new homepage::ProjectView(m_researchDb->getSharedPtr(), this);
    m_templateProject = new homepage::CreateProject(m_configDb->getSharedPtr(), this);
    
    // Project and workspace models
    m_project = new project::ProjectPage(m_researchDb->getSharedPtr(), this);
    m_wsModel = new WorkspaceModel(m_configDb->getSharedPtr(), m_engine);
    
    // Template model
    m_templateModel = new TemplateModel(m_configDb->getSharedPtr(), this);
    
    // Task and link models
    m_task = new project::TaskManger(m_researchDb->getSharedPtr(), this);
    m_lnModel = new project::LinkViewer(m_researchDb->getSharedPtr(), this);
    
    // File system models
    m_fsModel = new QFileSystemModel(m_engine);
    m_fsModel->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);
    m_fsWrapper = new project::FileSystemModelWrapper(m_fsModel, m_engine);
    m_flModel = new project::FileListViewer(m_engine);
    
    // Calendar and deadline models
    m_calModel = new project::CalendarView(m_researchDb->getSharedPtr(), m_engine);
    m_dlModel = new project::DeadlineModel(m_researchDb->getSharedPtr(), m_engine);
    
    // Contacts model uses config database
    m_contactsModel = new project::ContactsModel(m_configDb->getSharedPtr(), m_engine);
    m_contactsModel->load_database();

    // collaborator model
    m_colModel = new collab::CollaboratorModel(m_researchDb->getSharedPtr(), m_engine);
    m_msgModel = new collab::MessageViewer(m_researchDb->getSharedPtr(), m_engine);
    // AI Config model uses config database
    m_aiConfig = new AiConfig(m_configDb->getSharedPtr(), m_engine);

    // File downloader
    m_fileDownloader = new project::FileDownloader(m_engine);
}

void ApplicationManager::setupSignalConnections()
{
    // Template project connections
    QObject::connect(m_templateProject, SIGNAL(setProjectInfo(QStringList)),
                     m_project, SLOT(setProjectInfo(QStringList)));
    QObject::connect(m_templateProject, SIGNAL(setRootDir(QString)),
                     m_project, SLOT(setRootDir(QString)));
    QObject::connect(m_templateProject, SIGNAL(setReserachDB(QString)),
                     m_homepage, SLOT(setReserachDB(QString)));
    QObject::connect(m_templateProject, SIGNAL(setReserachDB(QString)),
                     m_calModel, SLOT(updateCalendarDB(QString)));
    
    // Project connections
    QObject::connect(m_project, SIGNAL(projectIdChanged(int)),
                     m_task, SLOT(projectIdChanged(int)));
    QObject::connect(m_project, SIGNAL(projectIdChanged(int)),
                     m_lnModel, SLOT(projectIdChanged(int)));
    QObject::connect(m_project, SIGNAL(projectIdChanged(int)),
                     m_dlModel, SLOT(projectIdChanged(int)));

    QObject::connect(m_project, SIGNAL(projectIdChanged(int)),
                     m_colModel, SLOT(projectIdChanged(int)));
    QObject::connect(m_project, SIGNAL(projectIdChanged(int)),
                     m_msgModel, SLOT(projectIdChanged(int)));



    QObject::connect(m_project, SIGNAL(projectRootDir(QString)),
                     m_fsWrapper, SLOT(setRootDir(QString)));
    QObject::connect(m_project, SIGNAL(projectRootDir(QString)),
                     m_flModel, SLOT(setRootDir(QString)));
    QObject::connect(m_flModel, SIGNAL(folderPathSelected(QString)),
                     m_project, SLOT(setLinkText(QString)));

    QObject::connect(m_templateProject, SIGNAL(setWsPathRoot(QString)),
                     m_project, SLOT(setWsPathRoot(QString)));


    
    // File downloader connections
    QObject::connect(m_project, SIGNAL(setDownloadLink(QString)),
                     m_fileDownloader, SLOT(setDownloadLink(QString)));
    QObject::connect(m_project, SIGNAL(setDownloadDirectory(QString)),
                     m_fileDownloader, SLOT(setDownloadDirectory(QString)));
}

void ApplicationManager::registerContextProperties()
{
    QQmlContext *context = m_engine->rootContext();
    
    // Register application manager and settings manager for QML access
    context->setContextProperty("appManager", this);
    context->setContextProperty("settingsManager", m_settingsManager);
    
    // Register all models as context properties
    // Note: reinterpret_cast is safe here since all models inherit from QObject
    context->setContextProperty("homepageModel", reinterpret_cast<QObject*>(m_homepage));
    context->setContextProperty("templateModel", reinterpret_cast<QObject*>(m_templateModel));
    context->setContextProperty("tpModel", reinterpret_cast<QObject*>(m_templateProject));
    context->setContextProperty("project", reinterpret_cast<QObject*>(m_project));
    context->setContextProperty("wsModel", reinterpret_cast<QObject*>(m_wsModel));
    context->setContextProperty("task", reinterpret_cast<QObject*>(m_task));
    context->setContextProperty("lnModel", reinterpret_cast<QObject*>(m_lnModel));
    context->setContextProperty("fsModel", reinterpret_cast<QObject*>(m_fsModel));
    context->setContextProperty("fsWrapper", reinterpret_cast<QObject*>(m_fsWrapper));
    context->setContextProperty("flModel", reinterpret_cast<QObject*>(m_flModel));
    context->setContextProperty("calModel", reinterpret_cast<QObject*>(m_calModel));
    context->setContextProperty("dlModel", reinterpret_cast<QObject*>(m_dlModel));
    context->setContextProperty("colModel", reinterpret_cast<QObject*>(m_colModel));
    context->setContextProperty("msgModel", reinterpret_cast<QObject*>(m_msgModel));
    context->setContextProperty("fileDownloader", reinterpret_cast<QObject*>(m_fileDownloader));
    context->setContextProperty("pcModel", reinterpret_cast<QObject*>(m_contactsModel));
    context->setContextProperty("aiConfig", reinterpret_cast<QObject*>(m_aiConfig));
}

bool ApplicationManager::loadQmlApplication()
{
    QUrl qmlFile;
    bool loaded = false;
    
    // Method 1: Try qrc resource first (preferred)
    qmlFile = QUrl("qrc:/ResearchManager/App.qml");
    m_engine->load(qmlFile);
    
    if (!m_engine->rootObjects().isEmpty()) {
        loaded = true;
        qDebug() << "Loaded QML from qrc:/ResearchManager/App.qml";
    } else {
        m_engine->clearComponentCache();
        
        // Method 2: Try direct file path
        qmlFile = QUrl::fromLocalFile(m_appDir + "/ResearchManager/App.qml");
        m_engine->load(qmlFile);
        
        if (!m_engine->rootObjects().isEmpty()) {
            loaded = true;
            qDebug() << "Loaded QML from file:" << qmlFile.toString();
        } else {
            m_engine->clearComponentCache();
            
            // Method 3: Try module loading as fallback
            try {
                m_engine->loadFromModule("ResearchManager", "App");
                if (!m_engine->rootObjects().isEmpty()) {
                    loaded = true;
                    qDebug() << "Loaded QML from module";
                }
            } catch (...) {
                qDebug() << "Module loading failed";
            }
        }
    }
    
    if (!loaded) {
        qWarning() << "Failed to load QML from all methods. Import paths:";
        for (const QString& path : m_engine->importPathList()) {
            qWarning() << "  " << path;
        }
    }
    
    return loaded;
}

void ApplicationManager::setupCleanupHandlers()
{
    QObject::connect(m_app, &QCoreApplication::aboutToQuit, [this]() {
        // Clear QML engine and all its components first to avoid crashes during cleanup
        m_engine->clearComponentCache();
        
        // Delete all root objects explicitly before closing database connections
        auto rootObjects = m_engine->rootObjects();
        for (QObject* obj : rootObjects) {
            delete obj;
        }
        
        // Now safely close database connections
        if (QSqlDatabase::contains("research")) {
            QSqlDatabase::database("research").close();
            QSqlDatabase::removeDatabase("research");
        }
        if (QSqlDatabase::contains("config")) {
            QSqlDatabase::database("config").close();
            QSqlDatabase::removeDatabase("config");
        }
    });
}

QString ApplicationManager::getConfigDatabasePath()
{
    // Load settings from QSettings
    QSettings settings("ResearchManager", "ResearchManager");
    const QString stored = settings.value("configDatabasePath", "").toString();

    // Honour an explicit, absolute choice made by the user, if it still exists.
    // Relative values come from older builds (e.g. "../../Test/common_config.db")
    // and are deliberately discarded: they depend on the working directory.
    if (!stored.isEmpty() && QFileInfo(stored).isAbsolute() && QFile::exists(stored)) {
        qDebug() << "[ApplicationManager] Using config database from settings:" << stored;
        return stored;
    }

    if (!stored.isEmpty() && !QFileInfo(stored).isAbsolute()) {
        qInfo() << "[ApplicationManager] Discarding relative config database path from settings:" << stored;
    }

    // Fall back to the per-user default location, creating it on demand.
    apppaths::ensureConfigDir();
    const QString defaultPath = apppaths::configDatabasePath();

    // First run after an upgrade: bring along a database created by an older build.
    apppaths::migrateLegacyDatabase(defaultPath, apppaths::legacyConfigDatabasePaths());

    settings.setValue("configDatabasePath", defaultPath);
    settings.sync();

    qInfo() << "[ApplicationManager] Using default config database:" << defaultPath;
    return defaultPath;
}

void ApplicationManager::ensureDefaultConfiguration()
{
    // Resolves (and, on first run, creates/migrates) the default database
    // locations so the application is usable without any manual setup.
    // The user can still override the config database from Settings.
    apppaths::ensureConfigDir();

    const QString configPath = getConfigDatabasePath();
    const QString researchPath = getResearchDatabasePath();

    qInfo() << "[ApplicationManager] Config database :" << configPath;
    qInfo() << "[ApplicationManager] Research database:" << researchPath;
}

QString ApplicationManager::browseForConfigDatabase()
{
    // Get the root object (main window) from the QML engine
    QObject *rootObject = nullptr;
    auto rootObjects = m_engine->rootObjects();
    if (!rootObjects.isEmpty()) {
        rootObject = rootObjects.first();
    }
    
    // Call the settings manager's browse function with the proper parent window
    return m_settingsManager->browseForDatabase(rootObject);
}

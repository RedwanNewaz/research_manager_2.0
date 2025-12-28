#include "fileexplorer.h"
#include <QDebug>
#include <QDir>
#include <QTimer>
#include <QProcess>
#include <QFileInfo>
#include <QUrl>
#include <QDesktopServices>
#include <QGuiApplication>
#include <QClipboard>

namespace project {

FileSystemModelWrapper::FileSystemModelWrapper(QFileSystemModel *model, QObject *parent)
    : QObject(parent), m_model(model), m_rootIndex(QModelIndex()), m_currentRootPath("")
{
    // Connect to directoryLoaded signal - fires when model finishes async loading
    connect(m_model, &QFileSystemModel::directoryLoaded, this, 
            &FileSystemModelWrapper::onDirectoryLoaded, Qt::UniqueConnection);
    
    // Connect to rowsInserted - fires IMMEDIATELY when rows are added (more reliable)
    connect(m_model, &QFileSystemModel::rowsInserted, this,
            &FileSystemModelWrapper::onRowsInserted, Qt::UniqueConnection);
}

bool FileSystemModelWrapper::isDir(const QModelIndex &index) const {
    if (!index.isValid()) return false;
    return m_model->isDir(index);
}

bool FileSystemModelWrapper::hasChildren(const QModelIndex &index) const {
    if (!index.isValid()) return false;
    return m_model->hasChildren(index);
}

QString FileSystemModelWrapper::filePath(const QModelIndex &index) const {
    if (!index.isValid()) return QString();
    return m_model->filePath(index);
}

QString FileSystemModelWrapper::fileName(const QModelIndex &index) const {
    if (!index.isValid()) return QString();
    return m_model->fileName(index);
}

void FileSystemModelWrapper::showInFileManager(const QString &filePath) const
{
    if (filePath.isEmpty()) {
        qWarning() << "Empty file path provided to showInFileManager";
        return;
    }
    
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        qWarning() << "File does not exist:" << filePath;
        return;
    }
    
    qInfo() << "showInFileManager called with:" << filePath;
    qInfo() << "Is file:" << fileInfo.isFile() << "Is dir:" << fileInfo.isDir();
    
    QString targetPath = fileInfo.absoluteFilePath();
    if (fileInfo.isFile()) {
        targetPath = fileInfo.absoluteDir().absolutePath();
    }
    
    qInfo() << "Target path to open:" << targetPath;
    
#ifdef Q_OS_WIN
    QProcess::startDetached("explorer", QStringList() << "/select," << QDir::toNativeSeparators(filePath));
#elif defined(Q_OS_MAC)
    // macOS: Use AppleScript for more reliable file manager opening
    QString script = QString("tell application \"Finder\" to reveal POSIX file \"%1\"").arg(fileInfo.absoluteFilePath());
    bool success = QProcess::startDetached("osascript", QStringList() << "-e" << script);
    
    if (!success) {
        qWarning() << "AppleScript method failed, trying 'open -R'";
        success = QProcess::startDetached("open", QStringList() << "-R" << fileInfo.absoluteFilePath());
    }
    
    if (success) {
        qInfo() << "Successfully opened" << targetPath << "in Finder";
    } else {
        qWarning() << "Failed to open in Finder:" << targetPath;
    }
#else
    // Linux - try different file managers in order of preference
    QStringList fileManagers = {"nemo", "nautilus", "dolphin", "thunar", "pcmanfm", "caja"};
    
    bool success = false;
    for (const QString &fileManager : fileManagers) {
        QProcess process;
        process.start("which", QStringList() << fileManager);
        process.waitForFinished(1000);
        
        if (process.exitCode() == 0) {
            qInfo() << "Found file manager:" << fileManager;
            // File manager is available
            QStringList args;
            
            if (fileManager == "nemo" || fileManager == "nautilus" || fileManager == "caja") {
                args << targetPath;
            } else if (fileManager == "dolphin") {
                if (fileInfo.isFile()) {
                    args << "--select" << fileInfo.absoluteFilePath();
                } else {
                    args << targetPath;
                }
            } else {
                args << targetPath;
            }
            
            qInfo() << "Attempting to start" << fileManager << "with args:" << args;
            success = QProcess::startDetached(fileManager, args);
            
            if (success) {
                qInfo() << "Successfully opened" << targetPath << "with" << fileManager;
                break;
            } else {
                qWarning() << "Failed to start" << fileManager;
            }
        }
    }
    
    if (!success) {
        qInfo() << "Fallback: Using QDesktopServices to open:" << targetPath;
        // Fallback: just open the directory with default application
        QUrl dirUrl = QUrl::fromLocalFile(targetPath);
        success = QDesktopServices::openUrl(dirUrl);
        
        if (success) {
            qInfo() << "Opened directory with default application:" << targetPath;
        } else {
            qWarning() << "Failed to open directory:" << targetPath;
        }
    }
#endif
}

void FileSystemModelWrapper::copyToClipboard(const QString &text) const
{
    if (text.isEmpty()) {
        qWarning() << "Empty text provided to copyToClipboard";
        return;
    }
    
    QClipboard *clipboard = QGuiApplication::clipboard();
    if (clipboard) {
        clipboard->setText(text, QClipboard::Clipboard);
        
        // Also set to selection clipboard on Linux for middle-click paste
#ifdef Q_OS_LINUX
        clipboard->setText(text, QClipboard::Selection);
#endif
        qInfo() << "Copied to clipboard:" << text;
    } else {
        qWarning() << "Failed to access clipboard";
    }
}

void FileSystemModelWrapper::openFile(const QString &filePath) const
{
    if (filePath.isEmpty()) {
        qWarning() << "Empty file path provided to openFile";
        return;
    }
    
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        qWarning() << "File does not exist:" << filePath;
        return;
    }
    
    if (fileInfo.isDir()) {
        qWarning() << "openFile called on a directory, use showInFileManager instead";
        return;
    }
    
    qInfo() << "Opening file:" << filePath;
    
#ifdef Q_OS_WIN
    // Windows: Use ShellExecute via QDesktopServices
    QUrl fileUrl = QUrl::fromLocalFile(filePath);
    bool success = QDesktopServices::openUrl(fileUrl);
    if (success) {
        qInfo() << "Successfully opened file:" << filePath;
    } else {
        qWarning() << "Failed to open file:" << filePath;
    }
#elif defined(Q_OS_MAC)
    // macOS: Use 'open' command which properly handles default applications
    bool success = QProcess::startDetached("open", QStringList() << fileInfo.absoluteFilePath());
    if (success) {
        qInfo() << "Successfully opened file with default application:" << filePath;
    } else {
        qWarning() << "QProcess failed, trying QDesktopServices fallback";
        QUrl fileUrl = QUrl::fromLocalFile(filePath);
        success = QDesktopServices::openUrl(fileUrl);
        if (success) {
            qInfo() << "Successfully opened file with QDesktopServices:" << filePath;
        } else {
            qWarning() << "Failed to open file:" << filePath;
        }
    }
#else
    // Linux: Use xdg-open which respects default applications
    QUrl fileUrl = QUrl::fromLocalFile(filePath);
    bool success = QDesktopServices::openUrl(fileUrl);
    
    if (!success) {
        qWarning() << "Failed to open file using xdg-open:" << filePath;
    } else {
        qInfo() << "Successfully opened file:" << filePath;
    }
#endif
}

void FileSystemModelWrapper::setRootDir(const QString &rootDir)
{
    m_loadStartTime = std::chrono::high_resolution_clock::now();
    qInfo() << "=== Setting root directory:" << rootDir;

    // Normalize the path
    QString normalizedPath = QDir::toNativeSeparators(QDir::cleanPath(rootDir));

    // Verify the directory exists
    QDir dir(normalizedPath);
    if (!dir.exists()) {
        qWarning() << "Directory does not exist:" << normalizedPath;
        return;
    }

    // Store the current root path
    m_currentRootPath = normalizedPath;
    emit currentRootPathChanged();

    // Set the root path - initiates async loading
    qInfo() << "Setting model root path to:" << normalizedPath;
    m_model->setRootPath(normalizedPath);

    // Get the index for this path
    QModelIndex newIndex = m_model->index(normalizedPath);
    
    if (newIndex.isValid()) {
        // Index is valid - set immediately and force aggressive fetch
        m_rootIndex = newIndex;
        emit rootIndexChanged();
        
        // Use aggressive fetching strategy
        forceFetchAll(newIndex);
        
        qInfo() << "Root index set immediately, row count:" << m_model->rowCount(m_rootIndex);
    } else {
        // Index not ready - emit invalid state and use timer fallback
        m_rootIndex = QModelIndex();
        emit rootIndexChanged();
        qInfo() << "Root index invalid - fetching with timeout fallback";
        
        // Use a single timeout to force update if loading takes too long (2 seconds max)
        QTimer::singleShot(2000, this, &FileSystemModelWrapper::ensureDataLoaded);
    }
}

void FileSystemModelWrapper::forceFetchAll(const QModelIndex &index)
{
    // Aggressively fetch all data
    if (!m_model->canFetchMore(index)) {
        return;
    }
    
    // Keep fetching until no more data
    int previousCount = 0;
    int fetchAttempts = 0;
    const int maxAttempts = 50;  // Prevent infinite loops
    
    while (m_model->canFetchMore(index) && fetchAttempts < maxAttempts) {
        int currentCount = m_model->rowCount(index);
        
        // If we're not making progress, break
        if (currentCount == previousCount && fetchAttempts > 0) {
            qInfo() << "Fetch complete, no new rows added";
            break;
        }
        
        previousCount = currentCount;
        m_model->fetchMore(index);
        fetchAttempts++;
    }
    
    if (fetchAttempts > 0) {
        qInfo() << "Force fetched" << fetchAttempts << "times, final row count:" << m_model->rowCount(index);
    }
}

void FileSystemModelWrapper::onRowsInserted(const QModelIndex &parent, int start, int end)
{
    // This signal fires immediately when rows are added
    // Check if this is for our current directory
    QString parentPath = m_model->filePath(parent);
    
    if (parentPath == m_currentRootPath) {
        qInfo() << "Rows inserted:" << start << "-" << end << "for" << parentPath;
        qInfo() << "Updated row count:" << m_model->rowCount(parent);
        
        // Update root index if not already set
        if (!m_rootIndex.isValid()) {
            m_rootIndex = parent;
            emit rootIndexChanged();
        }
        
        // Continue fetching if more data is available
        if (m_model->canFetchMore(parent)) {
            m_model->fetchMore(parent);
        }
    }
}

void FileSystemModelWrapper::onDirectoryLoaded(const QString &path)
{
    // Normalize paths for comparison
    QString normalizedPath = QDir::toNativeSeparators(QDir::cleanPath(path));
    
    if (normalizedPath == m_currentRootPath) {
        auto elapsed = std::chrono::high_resolution_clock::now() - m_loadStartTime;
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
        qInfo() << "[FileSystemModelWrapper] Directory fully loaded:" << normalizedPath << "- Load time:" << ms << "ms";
        
        QModelIndex newIndex = m_model->index(m_currentRootPath);
        if (newIndex.isValid()) {
            m_rootIndex = newIndex;
            emit rootIndexChanged();
            qInfo() << "[FileSystemModelWrapper]: Directory loaded, final row count:" << m_model->rowCount(m_rootIndex);
        }
    }
}

void FileSystemModelWrapper::ensureDataLoaded()
{
    // Fallback function called after timeout if data still hasn't loaded
    if (m_rootIndex.isValid()) {
        auto elapsed = std::chrono::high_resolution_clock::now() - m_loadStartTime;
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
        qInfo() << "Data already loaded (via timeout check) - Load time:" << ms << "ms";
        return;  // Data already loaded
    }
    
    // Try to get the index again
    QModelIndex newIndex = m_model->index(m_currentRootPath);
    if (newIndex.isValid()) {
        m_rootIndex = newIndex;
        emit rootIndexChanged();
        auto elapsed = std::chrono::high_resolution_clock::now() - m_loadStartTime;
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
        qInfo() << "Data loaded by timeout fallback - Load time:" << ms << "ms, row count:" << m_model->rowCount(m_rootIndex);
        
        // Try aggressive fetch
        forceFetchAll(newIndex);
    } else {
        qWarning() << "Timeout: Could not load directory" << m_currentRootPath;
    }
}

FileIconProvider::FileIconProvider()
    : QQuickImageProvider(QQuickImageProvider::Pixmap)
{
}

QPixmap FileIconProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    QFileInfo fileInfo(id);
    QFileIconProvider iconProvider;
    QIcon icon = iconProvider.icon(fileInfo);

    int width = requestedSize.width() > 0 ? requestedSize.width() : 32;
    int height = requestedSize.height() > 0 ? requestedSize.height() : 32;

    QPixmap pixmap = icon.pixmap(width, height);

    if (size)
        *size = pixmap.size();

    return pixmap;
}

} // namespace project

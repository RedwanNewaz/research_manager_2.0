#include "filedownloader.h"
#include <QNetworkRequest>
#include <QUrl>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>

using namespace project;

FileDownloader::FileDownloader(QObject *parent)
    : QObject{parent}
    , m_networkManager(new QNetworkAccessManager(this))
    , m_currentReply(nullptr)
    , m_downloadFile(nullptr)
    , m_isDownloading(false)
{
}

FileDownloader::~FileDownloader()
{
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
    }
    if (m_downloadFile) {
        m_downloadFile->close();
        delete m_downloadFile;
    }
}

bool FileDownloader::isDownloading() const
{
    return m_isDownloading;
}

QString FileDownloader::downloadStatus() const
{
    return m_downloadStatus;
}

QString FileDownloader::downloadDirectory() const
{
    return m_downloadDirectory;
}

void FileDownloader::setDownloadDirectory(const QString& directory)
{
    if (m_downloadDirectory == directory)
        return;
    m_downloadDirectory = directory;
    emit downloadDirectoryChanged();
    qInfo() << "Download directory set to:" << m_downloadDirectory;
}

void FileDownloader::setIsDownloading(bool downloading)
{
    if (m_isDownloading == downloading)
        return;
    m_isDownloading = downloading;
    emit isDownloadingChanged();
}

void FileDownloader::setDownloadStatus(const QString& status)
{
    if (m_downloadStatus == status)
        return;
    m_downloadStatus = status;
    emit downloadStatusChanged();
}

QString FileDownloader::extractFileName(const QString& url)
{
    QUrl qurl(url);
    QString path = qurl.path();
    QString fileName = QFileInfo(path).fileName();
    
    if (fileName.isEmpty()) {
        fileName = "downloaded_file";
    }
    
    return fileName;
}

void FileDownloader::setDownloadLink(const QString& link)
{
    if (link.isEmpty()) {
        qWarning() << "Empty download link provided";
        return;
    }

    // Cancel any existing download
    if (m_isDownloading) {
        cancelDownload();
    }

    qInfo() << "Starting download from:" << link;
    
    // Prepare the download directory (use custom directory if set, otherwise Downloads folder)
    QString downloadDir;
    if (!m_downloadDirectory.isEmpty() && QDir(m_downloadDirectory).exists()) {
        downloadDir = m_downloadDirectory;
    } else {
        downloadDir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    }
    
    QDir dir;
    if (!dir.exists(downloadDir)) {
        dir.mkpath(downloadDir);
    }

    // Extract filename from URL
    QString fileName = extractFileName(link);
    m_currentDownloadPath = downloadDir + "/" + fileName;

    // Handle file name conflicts
    int counter = 1;
    while (QFile::exists(m_currentDownloadPath)) {
        QFileInfo fileInfo(fileName);
        QString baseName = fileInfo.completeBaseName();
        QString extension = fileInfo.suffix();
        if (!extension.isEmpty()) {
            m_currentDownloadPath = downloadDir + "/" + baseName + QString("_%1.").arg(counter) + extension;
        } else {
            m_currentDownloadPath = downloadDir + "/" + fileName + QString("_%1").arg(counter);
        }
        counter++;
    }

    qInfo() << "Download will be saved to:" << m_currentDownloadPath;

    // Create the file
    m_downloadFile = new QFile(m_currentDownloadPath);
    if (!m_downloadFile->open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open file for writing:" << m_currentDownloadPath;
        setDownloadStatus("Error: Cannot create file");
        emit downloadError("Cannot create file: " + m_currentDownloadPath);
        delete m_downloadFile;
        m_downloadFile = nullptr;
        return;
    }

    // Start the download
    QNetworkRequest request(link);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    
    m_currentReply = m_networkManager->get(request);
    
    // Connect signals
    connect(m_currentReply, &QNetworkReply::finished, this, &FileDownloader::onDownloadFinished);
    connect(m_currentReply, &QNetworkReply::downloadProgress, this, &FileDownloader::onDownloadProgress);
    connect(m_currentReply, &QNetworkReply::errorOccurred, this, &FileDownloader::onDownloadError);
    connect(m_currentReply, &QNetworkReply::readyRead, this, [this]() {
        if (m_downloadFile && m_currentReply) {
            m_downloadFile->write(m_currentReply->readAll());
        }
    });

    setIsDownloading(true);
    setDownloadStatus("Downloading...");
}

void FileDownloader::cancelDownload()
{
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }
    
    if (m_downloadFile) {
        m_downloadFile->close();
        m_downloadFile->remove();
        delete m_downloadFile;
        m_downloadFile = nullptr;
    }
    
    setIsDownloading(false);
    setDownloadStatus("Download cancelled");
    qInfo() << "Download cancelled";
}

void FileDownloader::onDownloadFinished()
{
    if (!m_currentReply || !m_downloadFile) {
        return;
    }

    // Write any remaining data
    if (m_currentReply->bytesAvailable() > 0) {
        m_downloadFile->write(m_currentReply->readAll());
    }

    m_downloadFile->close();
    delete m_downloadFile;
    m_downloadFile = nullptr;

    if (m_currentReply->error() == QNetworkReply::NoError) {
        qInfo() << "Download completed successfully:" << m_currentDownloadPath;
        setDownloadStatus("Download complete");
        emit downloadComplete(m_currentDownloadPath);
    } else {
        qWarning() << "Download failed with error:" << m_currentReply->errorString();
        setDownloadStatus("Download failed");
        emit downloadError(m_currentReply->errorString());
        
        // Remove incomplete file
        QFile::remove(m_currentDownloadPath);
    }

    m_currentReply->deleteLater();
    m_currentReply = nullptr;
    
    setIsDownloading(false);
}

void FileDownloader::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if (bytesTotal > 0) {
        double progress = (double)bytesReceived / bytesTotal * 100.0;
        QString status = QString("Downloading... %1%").arg(QString::number(progress, 'f', 1));
        setDownloadStatus(status);
        qInfo() << "Download progress:" << progress << "%";
    }
}

void FileDownloader::onDownloadError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error);
    qWarning() << "Download error occurred:" << m_currentReply->errorString();
}

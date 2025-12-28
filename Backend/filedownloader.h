#ifndef FILEDOWNLOADER_H
#define FILEDOWNLOADER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>
#include <QFile>

namespace project {
/**
 * @class FileDownloader
 * @brief Manages network file transfers and synchronizes UI download states.
 *
 * This class orchestrates file downloads using the Qt Network module. It responds
 * to signals from the ProjectPage to initiate transfers and update directory paths.
 * Additionally, it manages the visibility of a busy indicator in the FolderViewer
 * search row to provide user feedback during active downloads.
 */
class FileDownloader : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isDownloading READ isDownloading NOTIFY isDownloadingChanged FINAL)
    Q_PROPERTY(QString downloadStatus READ downloadStatus NOTIFY downloadStatusChanged FINAL)
    Q_PROPERTY(QString downloadDirectory READ downloadDirectory WRITE setDownloadDirectory NOTIFY downloadDirectoryChanged FINAL)

public:
    explicit FileDownloader(QObject *parent = nullptr);
    ~FileDownloader();

    bool isDownloading() const;
    QString downloadStatus() const;
    QString downloadDirectory() const;

public slots:
    void setDownloadLink(const QString& link);
    void setDownloadDirectory(const QString& directory);
    void cancelDownload();

signals:
    void isDownloadingChanged();
    void downloadStatusChanged();
    void downloadDirectoryChanged();
    void downloadComplete(const QString& filePath);
    void downloadError(const QString& error);

private slots:
    void onDownloadFinished();
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onDownloadError(QNetworkReply::NetworkError error);

private:
    QNetworkAccessManager* m_networkManager;
    QNetworkReply* m_currentReply;
    QFile* m_downloadFile;
    bool m_isDownloading;
    QString m_downloadStatus;
    QString m_currentDownloadPath;
    QString m_downloadDirectory;

    void setIsDownloading(bool downloading);
    void setDownloadStatus(const QString& status);
    QString extractFileName(const QString& url);
};

} // namespace project

#endif // FILEDOWNLOADER_H

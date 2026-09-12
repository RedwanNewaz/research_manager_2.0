#include "filedownloader.h"
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QRegularExpression>
#include <QXmlStreamReader>
#include <QDebug>

using namespace project;

FileDownloader::FileDownloader(QObject *parent)
    : QObject{parent}
    , m_networkManager(new QNetworkAccessManager(this))
    , m_currentReply(nullptr)
    , m_metadataReply(nullptr)
    , m_downloadFile(nullptr)
    , m_isDownloading(false)
{
}

FileDownloader::~FileDownloader()
{
    if (m_metadataReply) {
        m_metadataReply->abort();
        m_metadataReply->deleteLater();
        m_metadataReply = nullptr;
    }
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

    // arXiv links carry no useful file name ("2309.10311" and no extension),
    // so look the paper up first and name the file after its title.
    const QString arxivId = arxivIdFromUrl(link);
    if (!arxivId.isEmpty()) {
        fetchArxivMetadata(link, arxivId);
        return;
    }

    startDownload(link, extractFileName(link));
}

QString FileDownloader::resolveDownloadDirectory()
{
    // Use the custom directory if set, otherwise the system Downloads folder
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
    return downloadDir;
}

void FileDownloader::startDownload(const QString& link, const QString& requestedName)
{
    qInfo() << "Starting download from:" << link;

    const QString downloadDir = resolveDownloadDirectory();

    QString fileName = requestedName.isEmpty() ? extractFileName(link) : requestedName;
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
        setIsDownloading(false);
        return;
    }

    // Start the download
    QNetworkRequest request((QUrl(link)));
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setHeader(QNetworkRequest::UserAgentHeader, "ResearchManager/1.0 (+https://airlab.cs.uno.edu)");
    
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
    if (m_metadataReply) {
        m_metadataReply->abort();
        m_metadataReply->deleteLater();
        m_metadataReply = nullptr;
    }

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
        // Some links (arXiv among them) end without a file extension. If the
        // server told us it sent a PDF, give the saved file the right suffix.
        if (QFileInfo(m_currentDownloadPath).suffix().isEmpty()) {
            const QString contentType =
                m_currentReply->header(QNetworkRequest::ContentTypeHeader).toString();
            if (contentType.contains(QLatin1String("pdf"), Qt::CaseInsensitive)) {
                const QString withSuffix = m_currentDownloadPath + QStringLiteral(".pdf");
                if (!QFile::exists(withSuffix) && QFile::rename(m_currentDownloadPath, withSuffix)) {
                    m_currentDownloadPath = withSuffix;
                    qInfo() << "[FileDownloader] Added .pdf extension:" << m_currentDownloadPath;
                }
            }
        }

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

// ============================================================================
// arXiv support
// ============================================================================

QString FileDownloader::arxivIdFromUrl(const QString& url)
{
    QString normalized = url.trimmed();

    // Users often paste "arxiv.org/pdf/2309.10311" without a scheme, which QUrl
    // would otherwise parse as a relative path with no host.
    if (!normalized.contains(QLatin1String("://")))
        normalized.prepend(QLatin1String("https://"));

    const QUrl qurl(normalized);
    const QString host = qurl.host().toLower();

    if (!host.endsWith(QLatin1String("arxiv.org")))
        return QString();

    // Accepted shapes:
    //   /pdf/2309.10311        /pdf/2309.10311v2       /pdf/2309.10311.pdf
    //   /abs/2309.10311        /abs/math/0309136       /pdf/math/0309136v1
    static const QRegularExpression re(
        R"(^/(?:pdf|abs|format)/((?:[a-z\-]+(?:\.[A-Z]{2})?/)?\d{4,7}\.?\d{0,5}(?:v\d+)?))",
        QRegularExpression::CaseInsensitiveOption);

    const QRegularExpressionMatch match = re.match(qurl.path());
    if (!match.hasMatch())
        return QString();

    QString id = match.captured(1);
    if (id.endsWith(QLatin1String(".pdf"), Qt::CaseInsensitive))
        id.chop(4);

    return id;
}

QString FileDownloader::arxivPdfUrl(const QString& url, const QString& arxivId)
{
    QString normalized = url.trimmed();
    if (!normalized.contains(QLatin1String("://")))
        normalized.prepend(QLatin1String("https://"));

    QUrl qurl(normalized);

    // /abs/ and /format/ are landing pages - rewrite them to the PDF itself.
    if (!qurl.path().startsWith(QLatin1String("/pdf/"), Qt::CaseInsensitive)) {
        qurl.setPath(QStringLiteral("/pdf/") + arxivId);
        qurl.setQuery(QString());
    }
    if (qurl.scheme().isEmpty())
        qurl.setScheme(QStringLiteral("https"));

    return qurl.toString();
}

QString FileDownloader::sanitizeFileName(const QString& name)
{
    QString clean = name.simplified();

    // Strip characters that are illegal (Windows) or awkward (all platforms)
    static const QRegularExpression illegal(R"([\\/:*?"<>|\x00-\x1F])");
    clean.replace(illegal, QStringLiteral(" "));

    // LaTeX leftovers that frequently appear in arXiv titles
    clean.remove(QLatin1Char('$'));
    clean.replace(QLatin1Char('{'), QLatin1Char(' '));
    clean.replace(QLatin1Char('}'), QLatin1Char(' '));

    // Words joined by underscores read better than spaces in a file name
    static const QRegularExpression spaces(R"(\s+)");
    clean.replace(spaces, QStringLiteral("_"));

    static const QRegularExpression repeats(R"(_{2,})");
    clean.replace(repeats, QStringLiteral("_"));

    // Trailing dots/underscores/spaces are invalid or invisible on Windows
    static const QRegularExpression edges(R"(^[_.\s]+|[_.\s]+$)");
    clean.remove(edges);

    // Keep the full path comfortably under filesystem limits
    const int kMaxLength = 120;
    if (clean.length() > kMaxLength) {
        clean.truncate(kMaxLength);
        const int lastSep = clean.lastIndexOf(QLatin1Char('_'));
        if (lastSep > kMaxLength / 2)
            clean.truncate(lastSep);
    }

    return clean;
}

QString FileDownloader::parseArxivTitle(const QByteArray& atomXml)
{
    QXmlStreamReader xml(atomXml);
    bool insideEntry = false;

    while (!xml.atEnd() && !xml.hasError()) {
        const QXmlStreamReader::TokenType token = xml.readNext();

        if (token == QXmlStreamReader::StartElement) {
            if (xml.name() == QLatin1String("entry")) {
                insideEntry = true;
            } else if (insideEntry && xml.name() == QLatin1String("title")) {
                // The feed itself also has a <title>; only the one inside
                // <entry> is the paper's title.
                const QString title = xml.readElementText().simplified();

                // arXiv answers an unknown id, or a rate-limited request, with a
                // well-formed feed whose single entry is titled "Error".
                if (title.compare(QLatin1String("Error"), Qt::CaseInsensitive) == 0) {
                    qWarning() << "[FileDownloader] arXiv returned an error entry";
                    return QString();
                }
                return title;
            }
        } else if (token == QXmlStreamReader::EndElement
                   && xml.name() == QLatin1String("entry")) {
            insideEntry = false;
        }
    }

    if (xml.hasError())
        qWarning() << "[FileDownloader] Failed to parse arXiv metadata:" << xml.errorString();

    return QString();
}

void FileDownloader::fetchArxivMetadata(const QString& link, const QString& arxivId)
{
    m_pendingLink = arxivPdfUrl(link, arxivId);
    m_pendingArxivId = arxivId;

    QUrl apiUrl(QStringLiteral("https://export.arxiv.org/api/query"));
    QUrlQuery apiQuery;
    apiQuery.addQueryItem(QStringLiteral("id_list"), arxivId);
    apiQuery.addQueryItem(QStringLiteral("max_results"), QStringLiteral("1"));
    apiUrl.setQuery(apiQuery);

    QNetworkRequest request(apiUrl);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setHeader(QNetworkRequest::UserAgentHeader, "ResearchManager/1.0 (+https://airlab.cs.uno.edu)");

    qInfo() << "[FileDownloader] Looking up arXiv metadata for" << arxivId;
    setIsDownloading(true);
    setDownloadStatus("Fetching paper details...");

    m_metadataReply = m_networkManager->get(request);
    connect(m_metadataReply, &QNetworkReply::finished, this, &FileDownloader::onMetadataFinished);
}

void FileDownloader::onMetadataFinished()
{
    if (!m_metadataReply)
        return;

    QNetworkReply* reply = m_metadataReply;
    m_metadataReply = nullptr;
    reply->deleteLater();

    const QString link = m_pendingLink;
    const QString arxivId = m_pendingArxivId;
    m_pendingLink.clear();
    m_pendingArxivId.clear();

    if (link.isEmpty()) // download was cancelled while the lookup was in flight
        return;

    // Fall back to the arXiv id if the lookup fails - never block the download.
    QString fileName = sanitizeFileName(arxivId) + QStringLiteral(".pdf");

    if (reply->error() == QNetworkReply::NoError) {
        const QString title = parseArxivTitle(reply->readAll());
        const QString sanitized = sanitizeFileName(title);
        if (!sanitized.isEmpty()) {
            fileName = sanitized + QStringLiteral(".pdf");
            qInfo() << "[FileDownloader] arXiv" << arxivId << "->" << fileName;
        } else {
            qWarning() << "[FileDownloader] No usable title in arXiv response for" << arxivId;
        }
    } else {
        qWarning() << "[FileDownloader] arXiv metadata lookup failed:" << reply->errorString();
    }

    startDownload(link, fileName);
}

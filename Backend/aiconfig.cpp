#include "aiconfig.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QRegularExpression>

#ifdef HAS_QT_PDF
#include <QPdfDocument>
#include <QPdfSelection>
#endif

AiConfig::AiConfig(DbmPtr configDb, QObject *parent)
    : QObject(parent)
    , m_configDb(configDb)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_isLoading(false)
    , m_currentRenameReply(nullptr)
{
    connect(m_networkManager, &QNetworkAccessManager::finished, 
            this, &AiConfig::onNetworkReply);
    
    // Ensure table exists
    createTableIfNotExists();
    
    // Load configuration
    loadConfig();
}

AiConfig::~AiConfig()
{
}

void AiConfig::createTableIfNotExists()
{
    if (!m_configDb) {
        qWarning() << "[AiConfig] Database pointer is null";
        return;
    }

    QString createTableSql = R"(
        CREATE TABLE IF NOT EXISTS AiConfig (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            key TEXT NOT NULL UNIQUE,
            value TEXT
        )
    )";

    if (!m_configDb->createTable(createTableSql)) {
        qWarning() << "[AiConfig] Failed to create AiConfig table";
    } else {
        qInfo() << "[AiConfig] AiConfig table ready";
    }
}

void AiConfig::loadConfig()
{
    m_geminiApiKey = getConfigValue("gemini_api_key", "");
    m_pdfRenamePrompt = getConfigValue("pdf_rename_prompt", 
        "Analyze this PDF document and suggest a descriptive filename based on its content.");
    m_calendarPrompt = getConfigValue("calendar_prompt", 
        "Parse the following text and extract calendar events with dates, times, and descriptions.");
    m_taskPrompt = getConfigValue("task_prompt", 
        "Parse the following text and extract task items with priorities and deadlines.");
    
    emit geminiApiKeyChanged();
    emit pdfRenamePromptChanged();
    emit calendarPromptChanged();
    emit taskPromptChanged();
    
    qInfo() << "[AiConfig] Configuration loaded";
}

bool AiConfig::saveConfig()
{
    bool success = true;
    
    success &= updateConfigValue("gemini_api_key", m_geminiApiKey);
    success &= updateConfigValue("pdf_rename_prompt", m_pdfRenamePrompt);
    success &= updateConfigValue("calendar_prompt", m_calendarPrompt);
    success &= updateConfigValue("task_prompt", m_taskPrompt);
    
    if (success) {
        emit configSaved();
        qInfo() << "[AiConfig] Configuration saved successfully";
    } else {
        emit configLoadError("Failed to save some configuration values");
        qWarning() << "[AiConfig] Failed to save some configuration values";
    }
    
    return success;
}

QString AiConfig::getConfigValue(const QString &key, const QString &defaultValue)
{
    if (!m_configDb) {
        return defaultValue;
    }

    QString sql = QString("SELECT value FROM AiConfig WHERE key = '%1'").arg(key);
    QStringList result = m_configDb->queryRow(sql);
    
    if (result.isEmpty()) {
        // Insert default value
        updateConfigValue(key, defaultValue);
        return defaultValue;
    }
    
    return result.first();
}

bool AiConfig::updateConfigValue(const QString &key, const QString &value)
{
    if (!m_configDb) {
        return false;
    }

    // // Use INSERT OR REPLACE to handle both insert and update
    // QString sql = QString(
    //     "INSERT OR REPLACE INTO AiConfig (key, value) VALUES ('%1', '%2')"
    // ).arg(key).arg(value.toHtmlEscaped().replace("'", "''"));
    
    // return m_configDb->updateDB(sql);

    auto query = m_configDb->getBinder(
        "INSERT OR REPLACE INTO AiConfig (key, value) "
        "VALUES (:key, :value)"
        );

    query.bindValue(":key", key);
    query.bindValue(":value", value.toHtmlEscaped().replace("'", "''"));
    return query.exec();
}

void AiConfig::setGeminiApiKey(const QString &key)
{
    if (m_geminiApiKey != key) {
        m_geminiApiKey = key;
        updateConfigValue("gemini_api_key", key);
        emit geminiApiKeyChanged();
    }
}

void AiConfig::setPdfRenamePrompt(const QString &prompt)
{
    if (m_pdfRenamePrompt != prompt) {
        m_pdfRenamePrompt = prompt;
        updateConfigValue("pdf_rename_prompt", prompt);
        emit pdfRenamePromptChanged();
    }
}

void AiConfig::setCalendarPrompt(const QString &prompt)
{
    if (m_calendarPrompt != prompt) {
        m_calendarPrompt = prompt;
        updateConfigValue("calendar_prompt", prompt);
        emit calendarPromptChanged();
    }
}

void AiConfig::setTaskPrompt(const QString &prompt)
{
    if (m_taskPrompt != prompt) {
        m_taskPrompt = prompt;
        updateConfigValue("task_prompt", prompt);
        emit taskPromptChanged();
    }
}

void AiConfig::testGemini(const QString &prompt)
{
    if (m_geminiApiKey.isEmpty()) {
        m_testResponse = "Error: API key is not set";
        emit testResponseChanged();
        emit testCompleted(false, m_testResponse);
        return;
    }

    if (prompt.isEmpty()) {
        m_testResponse = "Error: Prompt is empty";
        emit testResponseChanged();
        emit testCompleted(false, m_testResponse);
        return;
    }

    m_isLoading = true;
    emit isLoadingChanged();

    // Construct Gemini API URL (using gemini-2.5-flash model)
    QUrl url("https://generativelanguage.googleapis.com/v1beta/models/gemini-2.5-flash:generateContent");

    // Create network request with API key in header
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("x-goog-api-key", m_geminiApiKey.toUtf8());

    // Build JSON request body
    QJsonObject textPart;
    textPart["text"] = prompt;

    QJsonArray parts;
    parts.append(textPart);

    QJsonObject content;
    content["parts"] = parts;

    QJsonArray contents;
    contents.append(content);

    QJsonObject body;
    body["contents"] = contents;

    QJsonDocument doc(body);
    QByteArray data = doc.toJson();

    qInfo() << "[AiConfig] Sending test request to Gemini API";
    m_networkManager->post(request, data);
}

void AiConfig::onNetworkReply(QNetworkReply *reply)
{
    // Check if this is a rename reply
    if (reply == m_currentRenameReply) {
        onRenameReply(reply);
        return;
    }
    
    m_isLoading = false;
    emit isLoadingChanged();

    if (reply->error() != QNetworkReply::NoError) {
        m_testResponse = QString("Network Error: %1").arg(reply->errorString());
        qWarning() << "[AiConfig] Network error:" << reply->errorString();
        emit testResponseChanged();
        emit testCompleted(false, m_testResponse);
        reply->deleteLater();
        return;
    }

    QByteArray responseData = reply->readAll();
    QJsonDocument responseDoc = QJsonDocument::fromJson(responseData);

    if (responseDoc.isNull()) {
        m_testResponse = "Error: Failed to parse response";
        qWarning() << "[AiConfig] Failed to parse JSON response";
        emit testResponseChanged();
        emit testCompleted(false, m_testResponse);
        reply->deleteLater();
        return;
    }

    QJsonObject responseObj = responseDoc.object();

    // Check for API error
    if (responseObj.contains("error")) {
        QJsonObject errorObj = responseObj["error"].toObject();
        m_testResponse = QString("API Error: %1").arg(errorObj["message"].toString());
        qWarning() << "[AiConfig] API error:" << m_testResponse;
        emit testResponseChanged();
        emit testCompleted(false, m_testResponse);
        reply->deleteLater();
        return;
    }

    // Extract response text from Gemini response
    QJsonArray candidates = responseObj["candidates"].toArray();
    if (!candidates.isEmpty()) {
        QJsonObject firstCandidate = candidates[0].toObject();
        QJsonObject content = firstCandidate["content"].toObject();
        QJsonArray parts = content["parts"].toArray();
        
        if (!parts.isEmpty()) {
            m_testResponse = parts[0].toObject()["text"].toString();
            qInfo() << "[AiConfig] Received response from Gemini";
            emit testResponseChanged();
            emit testCompleted(true, m_testResponse);
        } else {
            m_testResponse = "Error: Empty response from API";
            emit testResponseChanged();
            emit testCompleted(false, m_testResponse);
        }
    } else {
        m_testResponse = "Error: No candidates in response";
        emit testResponseChanged();
        emit testCompleted(false, m_testResponse);
    }

    reply->deleteLater();
}

void AiConfig::renamePdfWithAi(const QString &filePath)
{
    qInfo() << "[AiConfig] Starting AI rename for:" << filePath;
    
    // Validate file
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        emit pdfRenameError("File does not exist: " + filePath);
        return;
    }
    
    if (fileInfo.suffix().toLower() != "pdf") {
        emit pdfRenameError("File is not a PDF: " + filePath);
        return;
    }
    
    if (m_geminiApiKey.isEmpty()) {
        emit pdfRenameError("API key is not set. Please configure your Gemini API key.");
        return;
    }
    
    if (m_pdfRenamePrompt.isEmpty()) {
        emit pdfRenameError("PDF rename prompt is not set. Please configure the prompt in AI Config.");
        return;
    }
    
    m_isLoading = true;
    emit isLoadingChanged();
    
    // Store the file path for later use
    m_pendingRenamePath = filePath;
    
    // Extract text from the first page of the PDF using Qt PDF
    QString firstPageText;
#ifdef HAS_QT_PDF
    QPdfDocument pdfDoc;
    QPdfDocument::Error loadError = pdfDoc.load(filePath);
    
    if (loadError == QPdfDocument::Error::None && pdfDoc.pageCount() > 0) {
        // Get selection for the entire first page
        QSizeF pageSize = pdfDoc.pagePointSize(0);
        QPdfSelection selection = pdfDoc.getAllText(0);
        firstPageText = selection.text();
        
        qInfo() << "[AiConfig] Extracted" << firstPageText.length() << "characters from first page";
        
        // Truncate if too long (to avoid API limits)
        if (firstPageText.length() > 4000) {
            firstPageText = firstPageText.left(4000) + "...";
        }
    } else {
        qWarning() << "[AiConfig] Failed to load PDF or no pages found. Error:" << static_cast<int>(loadError);
        firstPageText = "(Could not extract text from PDF)";
    }
#else
    qWarning() << "[AiConfig] Qt PDF module not available - using filename only";
    firstPageText = "(PDF text extraction not available - Qt6::Pdf module not installed)";
#endif
    
    // Build the prompt with the extracted text
    QString currentFileName = fileInfo.fileName();
    QString fullPrompt = m_pdfRenamePrompt + 
                         "\n\n--- PDF FIRST PAGE CONTENT ---\n" + 
                         firstPageText +
                         "\n--- END OF CONTENT ---\n\n"
                         "Current filename: " + currentFileName + 
                         "\n\nBased on the content above, please respond with ONLY the new filename "
                         "(without path, with .pdf extension). "
                         "Do not include any explanation or additional text.";
    
    // Construct Gemini API URL
    QUrl url("https://generativelanguage.googleapis.com/v1beta/models/gemini-2.5-flash:generateContent");

    // Create network request with API key in header
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("x-goog-api-key", m_geminiApiKey.toUtf8());

    // Build JSON request body
    QJsonObject textPart;
    textPart["text"] = fullPrompt;

    QJsonArray parts;
    parts.append(textPart);

    QJsonObject content;
    content["parts"] = parts;

    QJsonArray contents;
    contents.append(content);

    QJsonObject body;
    body["contents"] = contents;

    QJsonDocument doc(body);
    QByteArray data = doc.toJson();

    qInfo() << "[AiConfig] Sending PDF rename request to Gemini API with first page content";
    m_currentRenameReply = m_networkManager->post(request, data);
}

void AiConfig::onRenameReply(QNetworkReply *reply)
{
    m_currentRenameReply = nullptr;
    m_isLoading = false;
    emit isLoadingChanged();
    
    QString oldPath = m_pendingRenamePath;
    m_pendingRenamePath.clear();
    
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "[AiConfig] Network error during rename:" << reply->errorString();
        emit pdfRenameError("Network Error: " + reply->errorString());
        reply->deleteLater();
        return;
    }

    QByteArray responseData = reply->readAll();
    QJsonDocument responseDoc = QJsonDocument::fromJson(responseData);

    if (responseDoc.isNull()) {
        qWarning() << "[AiConfig] Failed to parse rename response";
        emit pdfRenameError("Failed to parse AI response");
        reply->deleteLater();
        return;
    }

    QJsonObject responseObj = responseDoc.object();

    // Check for API error
    if (responseObj.contains("error")) {
        QJsonObject errorObj = responseObj["error"].toObject();
        QString errorMsg = errorObj["message"].toString();
        qWarning() << "[AiConfig] API error during rename:" << errorMsg;
        emit pdfRenameError("API Error: " + errorMsg);
        reply->deleteLater();
        return;
    }

    // Extract response text
    QString suggestedName;
    QJsonArray candidates = responseObj["candidates"].toArray();
    if (!candidates.isEmpty()) {
        QJsonObject firstCandidate = candidates[0].toObject();
        QJsonObject content = firstCandidate["content"].toObject();
        QJsonArray parts = content["parts"].toArray();
        
        if (!parts.isEmpty()) {
            suggestedName = parts[0].toObject()["text"].toString().trimmed();
        }
    }
    
    if (suggestedName.isEmpty()) {
        emit pdfRenameError("AI returned empty filename suggestion");
        reply->deleteLater();
        return;
    }
    
    // Clean up the suggested name - remove any quotes, newlines, extra text
    suggestedName = suggestedName.split('\n').first().trimmed();
    suggestedName.remove(QRegularExpression("^[\"']|[\"']$"));
    
    // Ensure it ends with .pdf
    if (!suggestedName.toLower().endsWith(".pdf")) {
        suggestedName += ".pdf";
    }
    
    // Sanitize filename - remove invalid characters
    suggestedName.remove(QRegularExpression("[<>:\"/\\\\|?*]"));
    
    // Construct new path
    QFileInfo oldFileInfo(oldPath);
    QString newPath = oldFileInfo.absolutePath() + "/" + suggestedName;
    
    // Check if new file already exists
    if (QFile::exists(newPath) && newPath != oldPath) {
        // Add a number suffix to make it unique
        QString baseName = suggestedName.left(suggestedName.length() - 4); // Remove .pdf
        int counter = 1;
        while (QFile::exists(newPath)) {
            newPath = oldFileInfo.absolutePath() + "/" + baseName + "_" + QString::number(counter) + ".pdf";
            counter++;
        }
    }
    
    // Perform the rename
    QFile file(oldPath);
    if (file.rename(newPath)) {
        qInfo() << "[AiConfig] Successfully renamed PDF:" << oldPath << "to" << newPath;
        emit pdfRenameCompleted(true, oldPath, newPath);
    } else {
        qWarning() << "[AiConfig] Failed to rename file:" << file.errorString();
        emit pdfRenameError("Failed to rename file: " + file.errorString());
    }
    
    reply->deleteLater();
}

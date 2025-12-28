#ifndef AICONFIG_H
#define AICONFIG_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "database.h"

/**
 * @brief Backend class for managing AI configuration settings
 * 
 * Handles storing/retrieving AI prompts and API keys from the config database,
 * and provides functionality to test the Gemini API connection.
 */
class AiConfig : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(QString geminiApiKey READ geminiApiKey WRITE setGeminiApiKey NOTIFY geminiApiKeyChanged)
    Q_PROPERTY(QString pdfRenamePrompt READ pdfRenamePrompt WRITE setPdfRenamePrompt NOTIFY pdfRenamePromptChanged)
    Q_PROPERTY(QString calendarPrompt READ calendarPrompt WRITE setCalendarPrompt NOTIFY calendarPromptChanged)
    Q_PROPERTY(QString taskPrompt READ taskPrompt WRITE setTaskPrompt NOTIFY taskPromptChanged)
    Q_PROPERTY(QString testResponse READ testResponse NOTIFY testResponseChanged)
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)

public:
    explicit AiConfig(DbmPtr configDb, QObject *parent = nullptr);
    ~AiConfig();

    // Property getters
    QString geminiApiKey() const { return m_geminiApiKey; }
    QString pdfRenamePrompt() const { return m_pdfRenamePrompt; }
    QString calendarPrompt() const { return m_calendarPrompt; }
    QString taskPrompt() const { return m_taskPrompt; }
    QString testResponse() const { return m_testResponse; }
    bool isLoading() const { return m_isLoading; }

    // Property setters
    void setGeminiApiKey(const QString &key);
    void setPdfRenamePrompt(const QString &prompt);
    void setCalendarPrompt(const QString &prompt);
    void setTaskPrompt(const QString &prompt);

public slots:
    /**
     * @brief Load configuration from the database
     */
    void loadConfig();
    
    /**
     * @brief Save all configuration to the database
     * @return true if save successful
     */
    bool saveConfig();
    
    /**
     * @brief Test the Gemini API with a custom prompt
     * @param prompt The prompt to send to Gemini
     */
    void testGemini(const QString &prompt);
    
    /**
     * @brief Rename a PDF file using AI based on its content
     * @param filePath The full path to the PDF file
     */
    Q_INVOKABLE void renamePdfWithAi(const QString &filePath);

signals:
    void geminiApiKeyChanged();
    void pdfRenamePromptChanged();
    void calendarPromptChanged();
    void taskPromptChanged();
    void testResponseChanged();
    void isLoadingChanged();
    void configSaved();
    void configLoadError(const QString &error);
    void testCompleted(bool success, const QString &response);
    void pdfRenameCompleted(bool success, const QString &oldPath, const QString &newPath);
    void pdfRenameError(const QString &error);

private slots:
    void onNetworkReply(QNetworkReply *reply);
    void onRenameReply(QNetworkReply *reply);

private:
    void createTableIfNotExists();
    bool updateConfigValue(const QString &key, const QString &value);
    QString getConfigValue(const QString &key, const QString &defaultValue = QString());
    void sendGeminiRequest(const QString &prompt, std::function<void(QNetworkReply*)> callback);

    DbmPtr m_configDb;
    QNetworkAccessManager *m_networkManager;
    
    QString m_geminiApiKey;
    QString m_pdfRenamePrompt;
    QString m_calendarPrompt;
    QString m_taskPrompt;
    QString m_testResponse;
    bool m_isLoading;
    
    // PDF rename tracking
    QString m_pendingRenamePath;
    QNetworkReply *m_currentRenameReply;
};

#endif // AICONFIG_H

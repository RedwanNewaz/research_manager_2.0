#ifndef FILEEXPLORER_H
#define FILEEXPLORER_H

#include <QObject>
#include <QFileSystemModel>
#include <QFileIconProvider>
#include <QQuickImageProvider>
#include <QModelIndex>
#include <chrono>

namespace project {

class FileSystemModelWrapper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QModelIndex rootIndex READ rootIndex NOTIFY rootIndexChanged)
    Q_PROPERTY(QString currentRootPath READ currentRootPath NOTIFY currentRootPathChanged)

public:
    explicit FileSystemModelWrapper(QFileSystemModel *model, QObject *parent = nullptr);

    Q_INVOKABLE bool isDir(const QModelIndex &index) const;
    Q_INVOKABLE bool hasChildren(const QModelIndex &index) const;
    Q_INVOKABLE QString filePath(const QModelIndex &index) const;
    Q_INVOKABLE QString fileName(const QModelIndex &index) const;
    Q_INVOKABLE void showInFileManager(const QString &filePath) const;
    Q_INVOKABLE void copyToClipboard(const QString &text) const;
    Q_INVOKABLE void openFile(const QString &filePath) const;

    QModelIndex rootIndex() const { return m_rootIndex; }
    QString currentRootPath() const { return m_currentRootPath; }

public slots:
    void setRootDir(const QString &rootDir);

signals:
    void rootIndexChanged();
    void currentRootPathChanged();

private:
    QFileSystemModel *m_model;
    QModelIndex m_rootIndex;
    QString m_currentRootPath;
    std::chrono::high_resolution_clock::time_point m_loadStartTime;
    
    // Helper methods
    void onRowsInserted(const QModelIndex &parent, int start, int end);
    void onDirectoryLoaded(const QString &path);
    void forceFetchAll(const QModelIndex &index);
    void ensureDataLoaded();
};

class FileIconProvider : public QQuickImageProvider
{
public:
    FileIconProvider();
    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override;

private:
    QFileIconProvider m_iconProvider;
};

} // namespace project

#endif // FILEEXPLORER_H

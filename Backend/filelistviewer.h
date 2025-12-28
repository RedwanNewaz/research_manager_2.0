#ifndef FILELISTVIEWER_H
#define FILELISTVIEWER_H

#include <QObject>
#include <QAbstractListModel>
#include <QFileSystemModel>

namespace project{
class FileListViewer : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString currentFolderPath READ currentFolderPath WRITE setCurrentFolderPath NOTIFY currentFolderPathChanged FINAL)

public:
    explicit FileListViewer(QObject *parent = nullptr);
    Q_INVOKABLE void setRootDir(const QString& rootDir);

signals:
    void currentFolderPathChanged();
    void currentFolderPathUpdated(const QString &newPath);
    void folderPathSelected(const QString &path);

public:
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void onDirectoryLoaded(const QString &path);
    Q_INVOKABLE void refresh();

    QString currentFolderPath() const;
    void setCurrentFolderPath(const QString &newCurrentFolderPath);

private:
    QFileSystemModel *m_fileModel;
    QModelIndex m_rootIndex;
    enum Roles {
        filePath = Qt::UserRole + 1,
        fileName,
        fileType
    };

    QString m_currentFolderPath;
};
}

#endif // FILELISTVIEWER_H

#include "filelistviewer.h"

namespace project {
FileListViewer::FileListViewer(QObject *parent)
    : QAbstractListModel{parent}
{

    m_fileModel = new QFileSystemModel(this);
    m_fileModel->setFilter(QDir::Files | QDir::NoDotAndDotDot);
    // Handle async loading
    connect(m_fileModel, &QFileSystemModel::directoryLoaded,
            this, &FileListViewer::onDirectoryLoaded);

}

void FileListViewer::setRootDir(const QString &rootDir)
{
    beginResetModel();
     m_rootIndex = m_fileModel->setRootPath(rootDir);
    endResetModel();
    emit currentFolderPathChanged();
}

int FileListViewer::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0; // flat list

    return m_fileModel->rowCount(m_rootIndex);
}

QVariant FileListViewer::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    QModelIndex fsIndex =
        m_fileModel->index(index.row(), 0, m_rootIndex);

    if (!fsIndex.isValid())
        return {};

    switch (role) {
    case filePath:
        return m_fileModel->filePath(fsIndex);

    case fileName:
        return m_fileModel->fileName(fsIndex);

    case fileType:
        return "file";
    }

    return {};
}

QHash<int, QByteArray> FileListViewer::roleNames() const
{
    return {
        { filePath, "filePath" },
        { fileName, "fileName" },
        { fileType, "fileType" }
    };
}

void FileListViewer::onDirectoryLoaded(const QString &path)
{
    if (path != m_fileModel->rootPath())
        return;

    qInfo() << "Directory loaded:" << path;
    qInfo() << "Model rowCount:"
            << m_fileModel->rowCount(m_rootIndex);

    // Notify QML/view that data is now available
    beginResetModel();
    endResetModel();
}

void FileListViewer::refresh()
{
    if (m_currentFolderPath.isEmpty())
        return;
    
    qInfo() << "[FileListViewer] Refreshing directory:" << m_currentFolderPath;
    
    beginResetModel();
    m_rootIndex = m_fileModel->setRootPath("");
    m_rootIndex = m_fileModel->setRootPath(m_currentFolderPath);
    endResetModel();
}

QString FileListViewer::currentFolderPath() const
{
    return m_currentFolderPath;
}

void FileListViewer::setCurrentFolderPath(const QString &newCurrentFolderPath)
{
    if (m_currentFolderPath == newCurrentFolderPath)
        return;
    m_currentFolderPath = newCurrentFolderPath;

    beginResetModel();
    m_rootIndex = m_fileModel->setRootPath(newCurrentFolderPath);
    endResetModel();

    qInfo() << "new current path = " << newCurrentFolderPath;

    emit currentFolderPathChanged();
    emit currentFolderPathUpdated(newCurrentFolderPath);
    emit folderPathSelected(newCurrentFolderPath);
}


}

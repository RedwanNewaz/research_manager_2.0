#ifndef PROJECTPAGE_H
#define PROJECTPAGE_H

#include <QObject>
#include <QDebug>
#include <QDir>
#include <QAbstractListModel>
#include "database.h"


namespace project{
    class ProjectPage : public QObject
    {
        Q_OBJECT
        Q_PROPERTY(bool searchButton READ searchButton WRITE setSearchButton NOTIFY searchButtonChanged FINAL)
        Q_PROPERTY(QString projectRoot READ projectRoot WRITE setProjectRoot NOTIFY projectRootChanged FINAL)
        Q_PROPERTY(QString projectName READ projectName WRITE setProjectName NOTIFY projectNameChanged FINAL)
        Q_PROPERTY(QString projectDescription READ projectDescription WRITE setProjectDescription NOTIFY projectDescriptionChanged FINAL)
        Q_PROPERTY(QString linkText READ linkText WRITE setLinkText NOTIFY linkTextChanged FINAL)
    public:
        explicit ProjectPage(DbmPtr db, QObject *parent = nullptr);

        QString projectName() const;
        void setProjectName(const QString &newProjectName);

        QString projectDescription() const;
        void setProjectDescription(const QString &newProjectDescription);

        QString linkText() const;
        

        QString projectRoot() const;
        void setProjectRoot(const QString &newProjectRoot);

        bool searchButton() const;
        void setSearchButton(bool newSearchButton);
        Q_INVOKABLE void downloadFile(const QString& url);

    public slots:
        void setRootDir(const QString& root);
        void setProjectInfo(const QStringList& pInfo);
        void setLinkText(const QString &newLinkText);
        void setWsPathRoot(const QString& wsPath);

    signals:
        void projectNameChanged();
        void projectIdChanged(int);
        void projectDescriptionChanged();

        void linkTextChanged();
        void projectRootDir(QString);

        void projectRootChanged();

        void searchButtonChanged();

        void setDownloadLink(const QString& link);
        void setDownloadDirectory(const QString& directory);

    private:
        QString m_projectName;
        DbmPtr db_;
        QString m_projectDescription;
        QString m_linkText;
        QString m_projectRoot;
        bool m_searchButton;
        QString m_wsPath;
    };


}

#endif // PROJECTPAGE_H

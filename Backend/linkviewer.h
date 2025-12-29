#ifndef LINKVIEWER_H
#define LINKVIEWER_H

#include <QObject>
#include <QAbstractListModel>
#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QEventLoop>
#include "database.h"
namespace project{

class LinkViewer : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit LinkViewer(DbmPtr dbm, QObject *parent = nullptr);
    // QAbstractItemModel interface
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void checkData(int index, bool value);
    Q_INVOKABLE void addLink(const QString& link);
    Q_INVOKABLE void deleteLinks();
    Q_INVOKABLE bool anyCheck();
    Q_INVOKABLE void updateWebsiteName(int index, const QString& webName);


signals:

public slots:
    void projectIdChanged(int);

protected:
    QString getWebsiteName(const QString &urlString);

private:
    struct WebData{
        int index;
        int id;
        bool checked;
        QString website;
        QString url;
    };

    DbmPtr db_;
    mutable QMap<int, WebData> web_map_;
    int m_projectId;

    QNetworkAccessManager m_manager;

    enum LinkRoles {
        UrlRole = Qt::UserRole + 1,
        WebsiteRole,
        CheckBoxRole
    };
};
}

#endif // LINKVIEWER_H

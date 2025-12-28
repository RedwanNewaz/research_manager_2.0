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

    Q_INVOKABLE void setData(const QModelIndex &index, bool value, int role);
    Q_INVOKABLE void addLink(const QString& link);
    Q_INVOKABLE void deleteLink(int index);


signals:

public slots:
    void projectIdChanged(int);

protected:
    QString getWebsiteName(const QString &urlString);

private:
    DbmPtr db_;
    mutable QStringList data_, site_names_;
    int m_projectId, m_checkBoxIndex;
    mutable QHash<QString, int> id_map_;

    QNetworkAccessManager m_manager;

    enum LinkRoles {
        UrlRole = Qt::UserRole + 1,
        WebsiteRole,
        CheckBoxRole
    };
};
}

#endif // LINKVIEWER_H

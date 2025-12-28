#ifndef CREATEPROJECT_H
#define CREATEPROJECT_H

#include <QObject>
#include <QDir>
#include <QAbstractListModel>
#include <QTimer>
#include <QMap>
#include "database.h"

namespace homepage{


class CreateProject : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int WsIndex READ WsIndex WRITE setWsIndex NOTIFY WsIndexChanged FINAL)
    Q_PROPERTY(QString WsName READ WsName WRITE setWsName NOTIFY WsNameChanged FINAL)
    Q_PROPERTY(QString WsIcon READ WsIcon WRITE setWsIcon NOTIFY WsIconChanged FINAL)
    Q_PROPERTY(QString WsPath READ WsPath WRITE setWsPath NOTIFY WsPathChanged FINAL)
public:
    explicit CreateProject(DbmPtr db, QObject *parent = nullptr);
    Q_INVOKABLE int selectTemplate(const QString& name);
    Q_INVOKABLE void createProject(const QString& project_name, const QString& template_name);
    Q_INVOKABLE bool deleteProject(const QString& project_name);

signals:
    void setProjectInfo(QStringList);
    void setRootDir(QString);
    void setReserachDB(QString);
    void WsIndexChanged();
    void setWsPathRoot(QString);

    void WsNameChanged();

    void WsIconChanged();

    void WsPathChanged();

public slots:
    void setRoot();
private:
    DbmPtr db_;
    QString m_root_dir;
    QTimer * m_timer;
    // int m_ws_index = 1;
    QMap<int, QStringList> workspace_map_;
    mutable QStringList data_, template_;
    enum TemplateRoles {
        NameRole = Qt::UserRole + 1,
        IdRole
    };

    // QAbstractItemModel interface
    int m_WsIndex = -1;

    QString m_WsName;

    QString m_WsIcon;

    QString m_WsPath;

public:
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    int WsIndex() const;
    void setWsIndex(int newWsIndex);
    QString WsName() const;
    void setWsName(const QString &newWsName);
    QString WsIcon() const;
    void setWsIcon(const QString &newWsIcon);
    QString WsPath() const;
    void setWsPath(const QString &newWsPath);
};
}

#endif // CREATEPROJECT_H

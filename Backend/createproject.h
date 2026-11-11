#ifndef CREATEPROJECT_H
#define CREATEPROJECT_H

#include <QObject>
#include <QDir>
#include <QAbstractListModel>
#include <QTimer>
#include <QMap>
#include "database.h"
#include "categoryrepair.h"

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

    /**
     * @brief Load @p name's folder list and report the category it files under.
     * @param name a template name, e.g. "NSF" or "Invited Talk".
     * @return the canonical category name for @p name.
     */
    Q_INVOKABLE QString selectTemplate(const QString& name);

    /**
     * @brief Create @p project_name from @p template_name.
     *
     * Adds the project to the workspace's research database and creates the
     * template's folders on disk. Reports the outcome through
     * projectCreated() or projectCreationFailed() so the page can react
     * instead of silently doing nothing.
     *
     * @return true when the project was created.
     */
    Q_INVOKABLE bool createProject(const QString& project_name, const QString& template_name);
    Q_INVOKABLE bool deleteProject(const QString& project_name);

signals:
    /** @brief (project name, category name) for the project being created. */
    void setProjectInfo(QStringList);

    /** @brief A project was created; the home page should reload. */
    void projectCreated(QString name);

    /** @brief The project could not be created, with a message for the user. */
    void projectCreationFailed(QString reason);
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

#include "linkviewer.h"
using namespace project;

LinkViewer::LinkViewer(DbmPtr dbm, QObject *parent)
    : QAbstractListModel{parent}, db_(dbm), m_projectId(-1)
{
     m_checkBoxIndex = -1;
}

int LinkViewer::rowCount(const QModelIndex &parent) const
{
    if(m_projectId < 0) return 0;

    QString sqlCmd= QString("SELECT url FROM links WHERE project_id = %1").arg(m_projectId);
    data_ = db_->queryRow(sqlCmd);

    sqlCmd= QString("SELECT website FROM links WHERE project_id = %1").arg(m_projectId);

    site_names_ = db_->queryRow(sqlCmd);
    // qInfo() << "[LinkViewer] website names size = " << site_names_.size();

    sqlCmd= QString("SELECT id, url FROM links WHERE project_id = %1").arg(m_projectId);
    auto id_data = db_->queryRow(sqlCmd);
    for(int i = 0; i < id_data.size(); i+=2)
    {
        int j = i + 1;
        int indx = id_data.at(i).toInt();
        QString data = id_data.at(j);
        id_map_[data] = indx;
    }

    return data_.size();
}

QVariant LinkViewer::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    // qInfo() << " data row " << row;
    if (!index.isValid() || m_projectId < 0)
        return QVariant();
    switch (role) {
        case UrlRole: return data_[row];
        case WebsiteRole:   return site_names_[row];
        case  CheckBoxRole:   return m_checkBoxIndex == row;
        default: return QVariant();
    }
}

QHash<int, QByteArray> LinkViewer::roleNames() const
{
    return {
        { UrlRole, "url" },
        { WebsiteRole,   "website" },
        {CheckBoxRole, "checked"}
    };
}

void LinkViewer::projectIdChanged(int id)
{
    m_projectId = id;
    qInfo() << "[LinkViewer] project id updated = " << id;
    emit layoutChanged();
}

QString LinkViewer::getWebsiteName(const QString &urlString)
{
    QUrl url(urlString);
    QNetworkRequest request(url);

    // Sync wait (similar to Python's requests.get)
    QNetworkReply *reply = m_manager.get(request);
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Error fetching URL:" << reply->errorString();
        reply->deleteLater();
        return url.host();
    }

    QString html = QString::fromUtf8(reply->readAll());
    reply->deleteLater();

    // 1. Try to find <title> tag
    QRegularExpression titleRegex("<title>(.*?)</title>", QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch titleMatch = titleRegex.match(html);
    if (titleMatch.hasMatch()) {
        return titleMatch.captured(1).trimmed();
    }

    // 2. Try to find meta og:site_name
    // Matches: <meta property="og:site_name" content="TheName">
    QRegularExpression metaRegex("<meta[^>]*property=[\"']og:site_name[\"'][^>]*content=[\"'](.*?)[\"']",
                                 QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch metaMatch = metaRegex.match(html);
    if (metaMatch.hasMatch()) {
        return metaMatch.captured(1).trimmed();
    }

    return url.host();
}


void LinkViewer::setData(const QModelIndex &index, bool value, int role)
{
    m_checkBoxIndex = index.row();
}

void LinkViewer::addLink(const QString &rlink)
{
    QString link = rlink.trimmed();
    QString website = getWebsiteName(link);
    QString safeDescription = ""; // Use actual empty string, QSqlQuery handles nulls/empties

    qInfo() << "[LinkViewer]: website =" << website;

    // 1. Define the SQL with placeholders (?)
    QString sqlCmd = QString("INSERT INTO links (url, website, description, project_id) VALUES ('%1', '%2', '%3', %4)")
                    .arg(link)
                    .arg(website)
                    .arg(safeDescription)
                    .arg(m_projectId);

    // 4. Execute (Assuming your db_ helper can accept a QSqlQuery or just use query.exec())
    db_->updateDB(sqlCmd);

    m_checkBoxIndex = -1;
    emit layoutChanged();
}

void LinkViewer::deleteLink(int index)
{
    qInfo() << "[LinkViewer]: deleteLink " << index;
    auto id = id_map_[data_[index]];
    QString sqlCmd = QString("DELETE FROM links WHERE id = %1").arg(id);
    if(db_->updateDB(sqlCmd))
    {
        qInfo() << "[TaskManger] success ";
    }
    else{
        qInfo() << sqlCmd;
    }
    m_checkBoxIndex = -1;
    emit layoutChanged();
}

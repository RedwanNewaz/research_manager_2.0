#include "linkviewer.h"
using namespace project;

LinkViewer::LinkViewer(DbmPtr dbm, QObject *parent)
    : QAbstractListModel{parent}, db_(dbm), m_projectId(-1)
{

}

int LinkViewer::rowCount(const QModelIndex &parent) const
{
    if(m_projectId < 0) return 0;

    QString sqlCmd= QString("SELECT id, website, url FROM links WHERE project_id = %1").arg(m_projectId);
    auto results = db_->queryRow(sqlCmd);
    web_map_.clear();

    int index = 0;
    for(int i = 0; i < results.size(); i+=3)
    {
        int j = i + 1;
        int k = i + 2;

        WebData web;
        web.checked = false;
        web.index = index;
        web.id = results[i].toInt();
        web.website = results[j];
        web.url = results[k];
        web_map_[index] = web;
        ++index;
    }

    return web_map_.size();
}

QVariant LinkViewer::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if (!index.isValid() || m_projectId < 0 || !web_map_.contains(row))
        return QVariant();

    auto web = web_map_[row];
    switch (role) {
        case UrlRole: return web.url;
        case WebsiteRole:   return web.website;
        case  CheckBoxRole:   return web.checked;
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


void LinkViewer::checkData(int index, bool value)
{
    if(!web_map_.contains(index))
        return;

    web_map_[index].checked = value;

    // Notify view that this item has changed
    QModelIndex modelIndex = createIndex(index, 0);
    emit dataChanged(modelIndex, modelIndex, {CheckBoxRole});

    // debug purposes only
    for(const auto& val:web_map_.values())
        if(val.checked)
            qInfo() << "[LinkViewer] checked = " << val.index;
}

void LinkViewer::addLink(const QString &rlink)
{
    QString link = rlink.trimmed();
    QString website = getWebsiteName(link);
    QString safeDescription = ""; // Use actual empty string, QSqlQuery handles nulls/empties

    qInfo() << "[LinkViewer]: website =" << website;

    // // 1. Define the SQL with placeholders (?)
    // QString sqlCmd = QString("INSERT INTO links (url, website, description, project_id) VALUES ('%1', '%2', '%3', %4)")
    //                 .arg(link)
    //                 .arg(website)
    //                 .arg(safeDescription)
    //                 .arg(m_projectId);

    // // 4. Execute (Assuming your db_ helper can accept a QSqlQuery or just use query.exec())
    // db_->updateDB(sqlCmd);

    auto query = db_->getBinder(
        "INSERT INTO links (url, website, description, project_id) "
        "VALUES (:url, :web, :desc, :pid)"
        );

    query.bindValue(":url", link);
    query.bindValue(":web", website);
    query.bindValue(":desc", safeDescription);
    query.bindValue(":pid", m_projectId);
    query.exec();

    emit layoutChanged();
}

void LinkViewer::deleteLinks()
{
    QList<int> indicesToRemove;
    QList<int> idsToDelete;

    // Collect indices and IDs to delete
    for(const auto& pair : web_map_.toStdMap())
    {
        if(pair.second.checked)
        {
            indicesToRemove.append(pair.first);
            idsToDelete.append(pair.second.id);
        }
    }

    if(idsToDelete.isEmpty())
        return;

    // Delete from database
    for(int id : idsToDelete)
    {
        QString sqlCmd = QString("DELETE FROM links WHERE id = %1").arg(id);
        if(db_->deleteItem(sqlCmd))
        {
            qInfo() << "[LinkViewer] deleted link id " << id;
        }
        else{
            qInfo() << "Failed to delete link: " << sqlCmd;
        }
    }

    // Remove from model in reverse order to maintain indices
    std::sort(indicesToRemove.begin(), indicesToRemove.end(), std::greater<int>());
    for(int index : indicesToRemove)
    {
        beginRemoveRows(QModelIndex(), index, index);
        web_map_.remove(index);
        endRemoveRows();
    }

    // Update remaining indices
    QMap<int, WebData> updatedMap;
    int newIndex = 0;
    for(const auto& web : web_map_)
    {
        WebData updatedWeb = web;
        updatedWeb.index = newIndex;
        updatedMap[newIndex] = updatedWeb;
        newIndex++;
    }
    web_map_ = updatedMap;
}

bool LinkViewer::anyCheck()
{
    for(const auto& web: web_map_.values())
        if(web.checked)
            return true;
    return false;
}

void LinkViewer::updateWebsiteName(int index, const QString &webName)
{
    if(!web_map_.contains(index))
        return;

    auto web = web_map_[index];
    auto query = db_->getBinder("UPDATE links SET website = :web WHERE id = :id");
    query.bindValue(":web", webName);
    query.bindValue(":id", web.id);
    query.exec();

    emit layoutChanged();
}

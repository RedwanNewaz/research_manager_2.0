#include <gtest/gtest.h>
#include <QUrl>
#include <QString>
#include <QDebug>
#include <QCoreApplication>

#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QEventLoop>


class WebsiteParser : public QObject {
public:
    QString getWebsiteName(const QString &urlString) {
        QNetworkAccessManager manager;
        QUrl url(urlString);
        QNetworkRequest request(url);

        // Sync wait (similar to Python's requests.get)
        QNetworkReply *reply = manager.get(request);
        QEventLoop loop;
        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "Error fetching URL:" << reply->errorString();
            reply->deleteLater();
            return QString();
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

        return QString();
    }
};

TEST(NetworkManager, UrlParsing) {
    QString link = "https://www.cognitoforms.com/UNOResearch1/BudgetGenerator#KdoaJrF4eTbmQBpu2qGR99B-gFl2n0-43SXplxMNOkE$*";
    QUrl url(link);

    WebsiteParser webParser;

    if (url.isValid()) {
        QString websiteName = url.host();
        websiteName = webParser.getWebsiteName(link);
        qDebug() << "Website Name:" << websiteName;
    }

    ASSERT_TRUE(url.isValid());
}


// Custom main that initializes Qt before running tests
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

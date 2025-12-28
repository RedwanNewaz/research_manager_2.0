#include <gtest/gtest.h>
#include <QCoreApplication>
#include "../Backend/database.h"
#include <QDebug>


TEST(DatabaseManager, ConnectionTest) {
    DatabaseManager db("research");
    QString db_path = "../../Test/research_manager.db";
    ASSERT_TRUE(db.connect(db_path));
}


TEST(DatabaseManager, QueryRowTest) {
    DatabaseManager db("research");
    QString db_path = "../../Test/research_manager.db";
    ASSERT_TRUE(db.connect(db_path));

    QString sql = "SELECT name FROM categories";
    auto response = db.queryRow(sql);
    qInfo() << response;

    ASSERT_EQ(response.size(), 5);
}

TEST(DatabaseManager, CategoryDataTest) {
    DatabaseManager db("research");
    QString db_path = "../../Test/research_manager.db";
    ASSERT_TRUE(db.connect(db_path));

    QString sql = "SELECT name FROM categories";
    auto response = db.queryRow(sql);
    qInfo() << response;
    ASSERT_EQ(response.size(), 5);

    for(const auto& cat: response)
    {
        sql = "SELECT title FROM " + cat;
        auto items = db.queryRow(sql);
        qInfo() << items;
    }
}

TEST(DatabaseManager, CountTaskTest) {
    DatabaseManager db("research");
    QString db_path = "../../Test/research_manager.db";
    ASSERT_TRUE(db.connect(db_path));
    /*
     * SELECT
    COUNT(title)
FROM
    tasks
WHERE
    project_id = 13;
     */
    QString topic = "title";
    QString table = "tasks";
    QString constraint = "project_id = 13";
    auto response = db.queryCount(topic, table, constraint);
    qInfo() << response;
    ASSERT_EQ(response, 17);
}

// Custom main that initializes Qt before running tests
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

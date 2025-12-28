#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QDebug>
#include "../Backend/deadlineparser.h"



TEST(DeadlineParser, InputParsing) {
    QString newDeadlineTxt = "01/30/2026 new submission deadline";
    auto result = DeadlineParser::parseDeadlines(newDeadlineTxt);
    qInfo() << result.first;
}


TEST(DeadlineParser, MultipleInputsParsing) {
    QString newDeadlineTxt = R"(Paper Submission Opens 15 July 2024
        Paper Submission Deadline 15 September 2024
        Workshop/Tutorial submission deadline 25 September 2024
        Notification of Acceptance of Workshop and Tutorial Proposals 21 December 2024
        Registration & Housing Opens 3 January 2025
        Notification of paper acceptance/rejection 31 January 2025
        Submission of final papers 6 March 2025)";

    auto result = DeadlineParser::parseDeadlines(newDeadlineTxt);
    qInfo() << result.first;
}



// Custom main that initializes Qt before running tests
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

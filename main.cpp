#include <QGuiApplication>
#include <QSqlDatabase>
#include "Backend/applicationmanager.h"

/**
 * @brief Main entry point for the Research Manager application
 * 
 * Initializes the application, sets up all models and connections,
 * and starts the event loop.
 * 
 * @param argc Argument count
 * @param argv Argument values
 * @return Application exit code
 */
int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QSqlDatabase::removeDatabase("QSQLMIMER");
    
    ApplicationManager appManager(&app);
    
    if (!appManager.initialize()) {
        qCritical() << "Failed to initialize application";
        return -1;
    }
    
    return appManager.run();
}

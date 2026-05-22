#include <QApplication>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QDir>
#include <QStringList>
#include <QMainWindow>

#include "core/User.h"
#include "security/DatabaseManager.h"
#include "security/AuthManager.h"
#include "security/Logger.h"
#include "gui/AdminDashboard.h"
#include "gui/CustomerDashboard.h"
#include "gui/LoginWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Set application properties
    app.setApplicationName("Banking Management System");
    app.setApplicationVersion("1.0.0");

    // Initialize logging
    Logger* logger = Logger::getInstance();
    logger->info(Logger::SYSTEM, "Banking Management System starting up...");

    // Initialize DatabaseManager
    DatabaseManager* dbManager = DatabaseManager::getInstance();
    
    // MySQL connection parameters. If QMYSQL is unavailable, DatabaseManager falls back to SQLite.
    QString dbHostname = "localhost";
    QString dbName = "banking_system";
    QString dbUsername = "root";
    QString dbPassword = "adrien@12"; // Used only when MySQL/QMYSQL is available
    
    bool connected = dbManager->connect(dbHostname, dbName, dbUsername, dbPassword);
    
    if (!connected) {
        QString driverList = QSqlDatabase::drivers().join(", ");
        QString databaseError = dbManager->getLastDatabaseError();
        if (databaseError.isEmpty()) {
            databaseError = "No database error details were reported.";
        }
        logger->error(Logger::SYSTEM, "Failed to connect to database: " + databaseError);

        QMessageBox::critical(nullptr, "Database Error", 
                             "Failed to connect to the database.\n\n"
                             "Qt error: " + databaseError + "\n"
                             "Available SQL drivers: " + driverList + "\n\n"
                             "Please ensure:\n"
                             "1. MySQL is running if using QMYSQL\n"
                             "2. Database 'banking_system' exists if using MySQL\n"
                             "3. Your MySQL credentials are correct if using MySQL\n"
                             "4. Qt has either the QMYSQL or QSQLITE driver available");
        logger->error(Logger::SYSTEM, "Failed to connect to database - exiting");
        return -1;
    }

    logger->info(Logger::SYSTEM, "Database connected successfully");

    // Initialize AuthManager
    AuthManager* authManager = AuthManager::getInstance();
    authManager->initialize();

    logger->info(Logger::SYSTEM, "AuthManager initialized successfully");

    // Create and show login window
    LoginWindow loginWindow;
    loginWindow.setWindowTitle("Banking Management System - Login");
    loginWindow.resize(800, 600);

    QObject::connect(&loginWindow, &LoginWindow::loginSuccessful,
                     [&loginWindow, logger](int userId, const QString& username, const QString& role) {
        QMainWindow* dashboard = nullptr;

        if (role.compare("ADMIN", Qt::CaseInsensitive) == 0) {
            dashboard = new AdminDashboard(userId, username);
        } else {
            dashboard = new CustomerDashboard(userId, username);
        }

        QObject::connect(dashboard, &QObject::destroyed, &loginWindow, &LoginWindow::show);
        dashboard->setAttribute(Qt::WA_DeleteOnClose);
        dashboard->show();
        loginWindow.hide();
        logger->info(Logger::SYSTEM, QString("Opened %1 dashboard for %2").arg(role, username));
    });

    loginWindow.show();

    logger->info(Logger::SYSTEM, "Login window displayed");

    int result = app.exec();

    // Cleanup
    logger->info(Logger::SYSTEM, "Application shutting down...");
    dbManager->disconnect();

    return result;
}

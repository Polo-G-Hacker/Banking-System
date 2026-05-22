#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QMutex>
#include <QStringList>
#include <QMap>
#include <memory>

class DatabaseManager;

class Logger {

public:

    enum LogLevel {
        LevelDebug = 0,
        LevelInfo = 1,
        LevelWarning = 2,
        LevelError = 3,
        LevelCritical = 4,
        DEBUG = LevelDebug,
        INFO = LevelInfo,
        WARNING = LevelWarning,
        ERROR = LevelError,
        CRITICAL = LevelCritical
    };

    enum LogCategory {
        AUTHENTICATION,
        TRANSACTION,
        ADMIN_ACTION,
        SECURITY,
        SYSTEM,
        USER_ACTION,
        DATABASE,
        NETWORK
    };

private:
    static Logger* instance;

    QFile* logFile;
    QTextStream* logStream;
    QMutex logMutex;

    QString logFilePath;

    DatabaseManager* databaseManager;

    bool fileLoggingEnabled;
    bool databaseLoggingEnabled;

    LogLevel currentLogLevel;

    Logger();

public:
    static Logger* getInstance();
    ~Logger();

    // Configuration
    void setLogFilePath(const QString& filePath);

    void setFileLoggingEnabled(bool enabled) {
        fileLoggingEnabled = enabled;
    }

    void setDatabaseLoggingEnabled(bool enabled) {
        databaseLoggingEnabled = enabled;
    }

    void setLogLevel(LogLevel level) {
        currentLogLevel = level;
    }

    LogLevel getLogLevel() const {
        return currentLogLevel;
    }

    // Logging methods
    void log(LogLevel level,
             LogCategory category,
             const QString& message,
             int userId = 0,
             const QString& ipAddress = "");

    void debug(LogCategory category,
               const QString& message,
               int userId = 0);

    void info(LogCategory category,
              const QString& message,
              int userId = 0);

    void warning(LogCategory category,
                 const QString& message,
                 int userId = 0);

    void error(LogCategory category,
               const QString& message,
               int userId = 0);

    void critical(LogCategory category,
                  const QString& message,
                  int userId = 0);

    // Auth and Event specific loggers
    void logLoginAttempt(const QString& username, bool success, const QString& ipAddress = "");
    void logLogout(int userId);
    void logTransaction(int userId, const QString& transactionType, double amount, 
                        const QString& fromAccount, const QString& toAccount = "");
    void logAdminAction(int adminId, const QString& action, int targetUserId = 0, 
                        const QString& details = "");
    void logSecurityEvent(int userId, const QString& eventType, const QString& details = "");
    void logFailedAttempt(int userId, const QString& action, const QString& reason);
    void logSystemEvent(const QString& message);
    void logDatabaseEvent(const QString& operation, bool success, const QString& details = "");

    // Log Query Methods
    QStringList getRecentLogs(int count = 100);
    QStringList getLogsByUser(int userId, int hours = 24);
    QStringList getLogsByCategory(LogCategory category, int hours = 24);
    QStringList getLogsByLevel(LogLevel level, int hours = 24);
    QStringList searchLogs(const QString& searchTerm, int hours = 24);
    int getLogCount(LogLevel level, int hours = 24);
    int getLogCount(LogCategory category, int hours = 24);
    QMap<LogLevel, int> getLogDistribution(int hours = 24);
    QMap<LogCategory, int> getCategoryDistribution(int hours = 24);
    
    // Management
    void rotateLogs();
    void cleanupOldLogs(int daysToKeep = 30);
    void clearAllLogs();
    QString exportLogs(const QString& filePath, int hours = 24);
    
    // Security Analysis
    bool detectAnomalousActivity(int userId, int hours = 24);
    QStringList getSuspiciousActivities(int hours = 24);
    void generateSecurityReport(const QString& filePath);

private:
    QString levelToString(LogLevel level);

    QString categoryToString(LogCategory category);

    QString formatLogMessage(LogLevel level,
                             LogCategory category,
                             const QString& message,
                             int userId,
                             const QString& ipAddress);

    void writeToFile(const QString& message);

    void writeToDatabase(LogLevel level,
                         LogCategory category,
                         const QString& message,
                         int userId,
                         const QString& ipAddress);

    QString getCurrentTimestamp();

    void ensureLogDirectoryExists();
};

#endif

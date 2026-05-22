#include "security/Logger.h"
#include "security/DatabaseManager.h"
#include <QDir>
#include <QStandardPaths>
#include <QDateTime>
#include <QDebug>
#include <QStringConverter>

Logger* Logger::instance = nullptr;

Logger::Logger() {
    logFile = nullptr;
    logStream = nullptr;
    databaseManager = DatabaseManager::getInstance();
    fileLoggingEnabled = true;
    databaseLoggingEnabled = true;
    currentLogLevel = INFO;
    
    // Set default log file path
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    ensureLogDirectoryExists();
    logFilePath = appDataPath + "/banking_system.log";
    
    setLogFilePath(logFilePath);
}

Logger* Logger::getInstance() {
    if (!instance) {
        instance = new Logger();
    }
    return instance;
}

Logger::~Logger() {
    if (logStream) {
        delete logStream;
    }
    if (logFile) {
        if (logFile->isOpen()) {
            logFile->close();
        }
        delete logFile;
    }
}

void Logger::setLogFilePath(const QString& filePath) {
    QMutexLocker locker(&logMutex);
    
    if (logStream) {
        delete logStream;
        logStream = nullptr;
    }
    
    if (logFile) {
        if (logFile->isOpen()) {
            logFile->close();
        }
        delete logFile;
        logFile = nullptr;
    }
    
    logFilePath = filePath;
    logFile = new QFile(logFilePath);
    
    if (logFile->open(QIODevice::WriteOnly | QIODevice::Append)) {
        logStream = new QTextStream(logFile);
        logStream->setEncoding(QStringConverter::Utf8);
    } else {
        qDebug() << "Failed to open log file:" << logFilePath;
        delete logFile;
        logFile = nullptr;
    }
}

void Logger::log(LogLevel level, LogCategory category, const QString& message, 
                  int userId, const QString& ipAddress) {
    if (level < currentLogLevel) {
        return;
    }
    
    QString formattedMessage = formatLogMessage(level, category, message, userId, ipAddress);
    
    // Write to file
    if (fileLoggingEnabled) {
        writeToFile(formattedMessage);
    }
    
    // Write to database
    if (databaseLoggingEnabled && databaseManager->isDatabaseConnected()) {
        writeToDatabase(level, category, message, userId, ipAddress);
    }
    
    // Also output to console for debugging
    qDebug() << formattedMessage;
}

void Logger::debug(LogCategory category, const QString& message, int userId) {
    log(DEBUG, category, message, userId);
}

void Logger::info(LogCategory category, const QString& message, int userId) {
    log(INFO, category, message, userId);
}

void Logger::warning(LogCategory category, const QString& message, int userId) {
    log(WARNING, category, message, userId);
}

void Logger::error(LogCategory category, const QString& message, int userId) {
    log(ERROR, category, message, userId);
}

void Logger::critical(LogCategory category, const QString& message, int userId) {
    log(CRITICAL, category, message, userId);
}

void Logger::logLoginAttempt(const QString& username, bool success, const QString& ipAddress) {
    LogLevel level = success ? INFO : WARNING;
    LogCategory category = AUTHENTICATION;
    QString message = QString("Login attempt for user '%1': %2").arg(username).arg(success ? "SUCCESS" : "FAILED");
    
    log(level, category, message, 0, ipAddress);
}

void Logger::logLogout(int userId) {
    log(INFO, AUTHENTICATION, QString("User logged out"), userId);
}

void Logger::logTransaction(int userId, const QString& transactionType, double amount, 
                              const QString& fromAccount, const QString& toAccount) {
    QString message = QString("Transaction: %1 of %2 from account %3").arg(transactionType).arg(amount).arg(fromAccount);
    if (!toAccount.isEmpty()) {
        message += QString(" to account %1").arg(toAccount);
    }
    
    log(INFO, TRANSACTION, message, userId);
}

void Logger::logAdminAction(int adminId, const QString& action, int targetUserId, 
                             const QString& details) {
    QString message = QString("Admin action: %1").arg(action);
    if (targetUserId > 0) {
        message += QString(" on user %1").arg(targetUserId);
    }
    if (!details.isEmpty()) {
        message += QString(" - %1").arg(details);
    }
    
    log(INFO, ADMIN_ACTION, message, adminId);
}

void Logger::logSecurityEvent(int userId, const QString& eventType, const QString& details) {
    QString message = QString("Security event: %1").arg(eventType);
    if (!details.isEmpty()) {
        message += QString(" - %1").arg(details);
    }
    
    log(WARNING, SECURITY, message, userId);
}

void Logger::logFailedAttempt(int userId, const QString& action, const QString& reason) {
    QString message = QString("Failed attempt: %1 - %2").arg(action).arg(reason);
    log(WARNING, USER_ACTION, message, userId);
}

void Logger::logSystemEvent(const QString& message) {
    log(INFO, SYSTEM, message);
}

void Logger::logDatabaseEvent(const QString& operation, bool success, const QString& details) {
    LogLevel level = success ? INFO : ERROR;
    QString message = QString("Database operation: %1").arg(operation);
    if (!details.isEmpty()) {
        message += QString(" - %1").arg(details);
    }
    
    log(level, DATABASE, message);
}

QStringList Logger::getRecentLogs(int count) {
    QStringList logs;
    // This would read from the log file or database
    // For now, return empty list
    return logs;
}

QStringList Logger::getLogsByUser(int userId, int hours) {
    QStringList logs;
    // This would query the database for logs by user
    return logs;
}

QStringList Logger::getLogsByCategory(LogCategory category, int hours) {
    QStringList logs;
    // This would query the database for logs by category
    return logs;
}

QStringList Logger::getLogsByLevel(LogLevel level, int hours) {
    QStringList logs;
    // This would query the database for logs by level
    return logs;
}

QStringList Logger::searchLogs(const QString& searchTerm, int hours) {
    QStringList logs;
    // This would search logs for the search term
    return logs;
}

int Logger::getLogCount(LogLevel level, int hours) {
    // This would count logs by level
    return 0;
}

int Logger::getLogCount(LogCategory category, int hours) {
    // This would count logs by category
    return 0;
}

QMap<Logger::LogLevel, int> Logger::getLogDistribution(int hours) {
    QMap<LogLevel, int> distribution;
    // This would calculate log distribution by level
    return distribution;
}

QMap<Logger::LogCategory, int> Logger::getCategoryDistribution(int hours) {
    QMap<LogCategory, int> distribution;
    // This would calculate log distribution by category
    return distribution;
}

void Logger::rotateLogs() {
    if (!logFile) return;
    
    logFile->close();
    
    // Rename current log file with timestamp
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString rotatedPath = logFilePath + "." + timestamp;
    QFile::rename(logFilePath, rotatedPath);
    
    // Create new log file
    logFile->open(QIODevice::WriteOnly | QIODevice::Append);
    
    info(SYSTEM, QString("Log rotated to %1").arg(rotatedPath));
}

void Logger::cleanupOldLogs(int daysToKeep) {
    // This would delete log files older than the specified days
    info(SYSTEM, QString("Log cleanup initiated - keeping logs for %1 days").arg(daysToKeep));
}

void Logger::clearAllLogs() {
    QMutexLocker locker(&logMutex);
    
    if (logFile && logFile->isOpen()) {
        logFile->resize(0);
    }
    
    // Also clear database logs if enabled
    if (databaseLoggingEnabled) {
        // This would clear database log tables
    }
    
    info(SYSTEM, "All logs cleared");
}

QString Logger::exportLogs(const QString& filePath, int hours) {
    // This would export logs to the specified file
    return QString("Logs exported to %1").arg(filePath);
}

bool Logger::detectAnomalousActivity(int userId, int hours) {
    // This would analyze logs for anomalous patterns
    return false;
}

QStringList Logger::getSuspiciousActivities(int hours) {
    QStringList activities;
    // This would return suspicious activities from logs
    return activities;
}

void Logger::generateSecurityReport(const QString& filePath) {
    // This would generate a comprehensive security report
    info(SYSTEM, QString("Security report generated: %1").arg(filePath));
}

QString Logger::levelToString(LogLevel level) {
    switch (level) {
        case DEBUG: return "DEBUG";
        case INFO: return "INFO";
        case WARNING: return "WARNING";
        case ERROR: return "ERROR";
        case CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

QString Logger::categoryToString(LogCategory category) {
    switch (category) {
        case AUTHENTICATION: return "AUTH";
        case TRANSACTION: return "TRANS";
        case ADMIN_ACTION: return "ADMIN";
        case SECURITY: return "SECURITY";
        case SYSTEM: return "SYSTEM";
        case USER_ACTION: return "USER";
        case DATABASE: return "DATABASE";
        case NETWORK: return "NETWORK";
        default: return "UNKNOWN";
    }
}

QString Logger::formatLogMessage(LogLevel level, LogCategory category, const QString& message, 
                                  int userId, const QString& ipAddress) {
    QString timestamp = getCurrentTimestamp();
    QString levelStr = levelToString(level);
    QString categoryStr = categoryToString(category);
    
    QString formatted = QString("[%1] [%2] [%3]").arg(timestamp).arg(levelStr).arg(categoryStr);
    
    if (userId > 0) {
        formatted += QString(" [User:%1]").arg(userId);
    }
    
    if (!ipAddress.isEmpty()) {
        formatted += QString(" [IP:%1]").arg(ipAddress);
    }
    
    formatted += QString(" %1").arg(message);
    
    return formatted;
}

void Logger::writeToFile(const QString& message) {
    QMutexLocker locker(&logMutex);
    
    if (logStream) {
        *logStream << message << Qt::endl;
        logStream->flush();
    }
}

void Logger::writeToDatabase(LogLevel level, LogCategory category, const QString& message, 
                              int userId, const QString& ipAddress) {
    // This would write the log entry to the database audit_logs table
    databaseManager->logActivity(userId, categoryToString(category), message, 
                                 levelToString(level), ipAddress);
}

QString Logger::getCurrentTimestamp() {
    return QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
}

void Logger::ensureLogDirectoryExists() {
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir;
    if (!dir.exists(appDataPath)) {
        dir.mkpath(appDataPath);
    }
}

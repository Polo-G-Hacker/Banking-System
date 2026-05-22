#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QString>
#include <QSqlDatabase>
#include <QstringList>
#include <Qmap>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDateTime>
#include <memory>

class User;
class Customer;
class Admin;
class Account;
class Transaction;

class DatabaseManager {
private:
    static DatabaseManager* instance;
    QSqlDatabase database;
    QString hostname;
    QString databaseName;
    QString username;
    QString password;
    int port;
    bool isConnected;
    
    DatabaseManager();

public:
    static DatabaseManager* getInstance();
    ~DatabaseManager();
    
    // Connection management
    bool connect(const QString& hostname, const QString& databaseName, 
                 const QString& username, const QString& password, int port = 3306);
    void disconnect();
    bool isDatabaseConnected() const { return isConnected; }
    QString getLastDatabaseError() const;
    
    // Transaction management
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();
    
    // User operations
    std::shared_ptr<User> authenticateUser(const QString& username, const QString& password);
    std::shared_ptr<User> getUserById(int userId);
    std::shared_ptr<User> getUserByUsername(const QString& username);
    bool createUser(const User& user);
    bool updateUser(const User& user);
    bool deleteUser(int userId);
    QList<std::shared_ptr<User>> getAllUsers();
    QList<std::shared_ptr<User>> getUsersByRole(const QString& role);
    
    // Customer operations
    std::shared_ptr<Customer> getCustomerById(int customerId);
    std::shared_ptr<Customer> getCustomerByUsername(const QString& username);
    QList<std::shared_ptr<Customer>> getAllCustomers();
    bool createCustomer(const Customer& customer);
    bool updateCustomer(const Customer& customer);
    bool suspendCustomer(int customerId, int minutes = 1440);
    bool activateCustomer(int customerId);
    
    // Admin operations
    std::shared_ptr<Admin> getAdminById(int adminId);
    QList<std::shared_ptr<Admin>> getAllAdmins();
    
    // Account operations
    std::shared_ptr<Account> getAccountById(int accountId);
    std::shared_ptr<Account> getAccountByNumber(const QString& accountNumber);
    QList<std::shared_ptr<Account>> getAccountsByUserId(int userId);
    QList<std::shared_ptr<Account>> getAllAccounts();
    bool createAccount(const Account& account);
    bool updateAccount(const Account& account);
    bool closeAccount(int accountId);
    double getTotalBalanceByUserId(int userId);
    
    // Transaction operations
    std::shared_ptr<Transaction> getTransactionById(int transactionId);
    QList<std::shared_ptr<Transaction>> getTransactionsByAccountId(int accountId);
    QList<std::shared_ptr<Transaction>> getTransactionsByUserId(int userId);
    QList<std::shared_ptr<Transaction>> getTransactionsByDateRange(const QDateTime& startDate, 
                                                                   const QDateTime& endDate);
    bool createTransaction(const Transaction& transaction);
    bool updateTransactionStatus(int transactionId, const QString& status);
    
    // Audit logging
    bool logActivity(int userId, const QString& action, const QString& description, 
                     const QString& status, const QString& ipAddress = "");
    bool logFailedLogin(const QString& username, const QString& ipAddress, const QString& reason);
    bool logAdminAction(int adminId, int targetUserId, const QString& actionType, 
                        const QString& description);
    
    // OTP operations
    bool generateOTP(int userId, const QString& purpose, QString& otpCode);
    bool validateOTP(int userId, const QString& otpCode, const QString& purpose);
    void cleanupExpiredOTPs();
    
    // Suspicious activity operations
    bool logSuspiciousActivity(int userId, const QString& activityType, 
                               const QString& description, const QString& severity);
    
    // Email notification operations
    bool createEmailNotification(int userId, const QString& emailType, 
                                  const QString& subject, const QString& body);
    
    // Database backup and restore
    bool createBackup(const QString& backupPath);
    bool restoreFromBackup(const QString& backupPath);
    QString generateBackupFileName();
    
    // Statistics and reporting
    int getTotalCustomers();
    int getTotalAccounts();
    int getTotalTransactions();
    double getTotalBankDeposits();
    double getTodayTransactionsTotal();
    int getFailedLoginAttempts(const QString& username, int hours = 24);
    
    // Database maintenance
    bool optimizeDatabase();
    bool cleanupOldData(int daysToKeep = 365);
    bool checkDatabaseIntegrity();

private:
    void initializeDatabase();
    QSqlQuery executeQuery(const QString& query);
    QSqlQuery executePreparedQuery(const QString& query, const QVariantList& parameters = QVariantList());
    std::shared_ptr<User> createUserFromQuery(QSqlQuery& query);
    std::shared_ptr<Account> createAccountFromQuery(QSqlQuery& query);
    std::shared_ptr<Transaction> createTransactionFromQuery(QSqlQuery& query);
    QString encryptField(const QString& data);
    QString decryptField(const QString& encryptedData);
};

#endif // DATABASEMANAGER_H

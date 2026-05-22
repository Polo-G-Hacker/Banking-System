#ifndef BANKSERVICE_H
#define BANKSERVICE_H

#include <QString>
#include <QDateTime>
#include <memory>
#include <QList>

class User;
class Customer;
class Admin;
class Account;
class Transaction;
class DatabaseManager;
class AuthManager;
class SecurityManager;
class Logger;

class BankService {
private:
    static BankService* instance;
    DatabaseManager* databaseManager;
    AuthManager* authManager;
    SecurityManager* securityManager;
    Logger* logger;
    
    BankService();

public:
    static BankService* getInstance();
    ~BankService();
    
    // Initialization
    bool initialize(const QString& dbHostname, const QString& dbName, 
                     const QString& dbUsername, const QString& dbPassword, int dbPort = 3306);
    bool isInitialized() const;
    void shutdown();
    
    // Authentication services
    struct AuthResult {
        bool success;
        QString message;
        bool requiresOTP;
        bool requiresCaptcha;
        QString captchaQuestion;
        int userId;
    };
    
    AuthResult authenticateUser(const QString& username, const QString& password, 
                                 const QString& captchaAnswer = "");
    bool verifyOTP(const QString& otpCode);
    bool logout();
    bool isLoggedIn();
    std::shared_ptr<User> getCurrentUser();
    
    // Customer services
    struct TransactionResult {
        bool success;
        QString message;
        QString transactionId;
        double newBalance;
    };
    
    TransactionResult deposit(int accountId, double amount, const QString& description = "");
    TransactionResult withdraw(int accountId, double amount, const QString& description = "");
    TransactionResult transfer(int fromAccountId, int toAccountId, double amount, 
                                const QString& description = "");
    QList<std::shared_ptr<Account>> getUserAccounts(int userId);
    QList<std::shared_ptr<Transaction>> getAccountTransactions(int accountId);
    double getAccountBalance(int accountId);
    
    // Admin services
    struct CustomerCreationResult {
        bool success;
        QString message;
        int customerId;
    };
    
    CustomerCreationResult createCustomer(const QString& username, const QString& email,
                                          const QString& password, const QString& fullName,
                                          const QString& phoneNumber);
    bool createAccount(int userId, const QString& accountType, double initialBalance = 0.0);
    bool suspendCustomer(int customerId, const QString& reason);
    bool activateCustomer(int customerId);
    QList<std::shared_ptr<Customer>> getAllCustomers();
    QList<std::shared_ptr<Account>> getAllAccounts();
    
    // User management
    bool changePassword(const QString& oldPassword, const QString& newPassword);
    bool changeTransactionPin(const QString& oldPin, const QString& newPin);
    bool resetPassword(const QString& username, const QString& email, const QString& newPassword);
    
    // Account management
    bool closeAccount(int accountId);
    std::shared_ptr<Account> getAccountById(int accountId);
    std::shared_ptr<Account> getAccountByNumber(const QString& accountNumber);
    
    // Transaction services
    std::shared_ptr<Transaction> getTransactionById(int transactionId);
    QList<std::shared_ptr<Transaction>> getTransactionsByDateRange(const QDateTime& startDate, 
                                                                     const QDateTime& endDate);
    QList<std::shared_ptr<Transaction>> getUserTransactions(int userId);
    
    // Reporting and statistics
    struct BankStatistics {
        int totalCustomers;
        int activeCustomers;
        int totalAccounts;
        int activeAccounts;
        double totalDeposits;
        int todayTransactions;
        double todayTransactionVolume;
        int failedLoginsToday;
    };
    
    BankStatistics getBankStatistics();
    QString generateCustomerReport(int customerId);
    QString generateTransactionReport(const QDateTime& startDate, const QDateTime& endDate);
    QString generateActivityReport(int hours = 24);
    
    // Security services
    bool isAccountLocked(int userId);
    void lockAccount(int userId, int minutes = 5);
    void unlockAccount(int userId);
    int getFailedLoginAttempts(int userId);
    
    // Backup and restore
    bool createBackup(const QString& backupPath);
    bool restoreBackup(const QString& backupPath);
    QString getBackupFileName();
    
    // Validation services
    bool validateEmail(const QString& email);
    bool validatePhoneNumber(const QString& phone);
    bool validatePasswordStrength(const QString& password);
    bool validateTransactionPin(const QString& pin);
    bool validateAmount(double amount);
    bool validateAccountNumber(const QString& accountNumber);
    
    // Notification services
    bool sendEmailNotification(int userId, const QString& subject, const QString& body);
    bool sendTransactionNotification(int userId, const QString& transactionType, 
                                     double amount, const QString& accountNumber);
    bool sendSecurityAlert(int userId, const QString& alertType, const QString& details);

private:
    bool initialized;
    void logTransaction(const TransactionResult& result, int userId, const QString& operation);
    void logAdminAction(const QString& action, const QString& details);
    bool validateTransactionPermission(int accountId);
    bool performAtomicTransaction(std::function<bool()> transaction);
    QString generateTransactionReference(const QString& type);
};

#endif // BANKSERVICE_H

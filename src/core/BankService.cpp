#include "core/BankService.h"
#include "security/DatabaseManager.h"
#include "security/AuthManager.h"
#include "security/SecurityManager.h"
#include "security/Logger.h"
#include "core/User.h"
#include "core/Customer.h"
#include "core/Admin.h"
#include "core/Account.h"
#include "core/Transaction.h"
#include <QDebug>
#include <QFile>

BankService* BankService::instance = nullptr;

BankService::BankService() {
    databaseManager = DatabaseManager::getInstance();
    authManager = AuthManager::getInstance();
    securityManager = SecurityManager::getInstance();
    logger = Logger::getInstance();
    initialized = false;
}

BankService* BankService::getInstance() {
    if (!instance) {
        instance = new BankService();
    }
    return instance;
}

BankService::~BankService() {
    shutdown();
}

bool BankService::initialize(const QString& dbHostname, const QString& dbName, 
                              const QString& dbUsername, const QString& dbPassword, int dbPort) {
    if (initialized) {
        return true;
    }
    
    bool dbConnected = databaseManager->connect(dbHostname, dbName, dbUsername, dbPassword, dbPort);
    
    if (dbConnected) {
        initialized = true;
        logger->info(Logger::SYSTEM, "BankService initialized successfully");
        return true;
    } else {
        logger->error(Logger::SYSTEM, "Failed to initialize BankService - Database connection failed");
        return false;
    }
}

bool BankService::isInitialized() const {
    return initialized;
}

void BankService::shutdown() {
    if (initialized) {
        authManager->logout();
        databaseManager->disconnect();
        initialized = false;
        logger->info(Logger::SYSTEM, "BankService shutdown completed");
    }
}

BankService::AuthResult BankService::authenticateUser(const QString& username, const QString& password, 
                                                        const QString& captchaAnswer) {
    AuthResult result;
    auto loginResult = authManager->login(username, password, captchaAnswer);
    
    result.success = loginResult.success;
    result.message = loginResult.message;
    result.requiresOTP = !loginResult.otpRequired.isEmpty();
    result.requiresCaptcha = !loginResult.captchaRequired.isEmpty();
    result.captchaQuestion = loginResult.captchaRequired;
    result.userId = loginResult.userId;
    
    if (result.success) {
        logger->log(Logger::INFO, Logger::AUTHENTICATION, 
                     QString("User '%1' authenticated successfully").arg(username));
    } else {
        logger->log(Logger::WARNING, Logger::AUTHENTICATION, 
                     QString("Authentication failed for user '%1': %2").arg(username).arg(result.message));
    }
    
    return result;
}

bool BankService::verifyOTP(const QString& otpCode) {
    bool verified = authManager->verifyOTP(otpCode, "LOGIN");
    
    if (verified) {
        logger->log(Logger::INFO, Logger::AUTHENTICATION, "OTP verification successful");
    } else {
        logger->log(Logger::WARNING, Logger::AUTHENTICATION, "OTP verification failed");
    }
    
    return verified;
}

bool BankService::logout() {
    bool loggedOut = authManager->logout();
    if (loggedOut) {
        logger->log(Logger::INFO, Logger::AUTHENTICATION, "User logged out");
    }
    return loggedOut;
}

bool BankService::isLoggedIn() {
    return authManager->isLoggedIn();
}

std::shared_ptr<User> BankService::getCurrentUser() {
    return authManager->getCurrentUser();
}

BankService::TransactionResult BankService::deposit(int accountId, double amount, const QString& description) {
    TransactionResult result;
    result.success = false;
    
    if (!validateTransactionPermission(accountId)) {
        result.message = "Permission denied";
        return result;
    }
    
    if (!validateAmount(amount)) {
        result.message = "Invalid amount";
        return result;
    }
    
    auto account = databaseManager->getAccountById(accountId);
    if (!account) {
        result.message = "Account not found";
        return result;
    }
    
    bool success = performAtomicTransaction([this, account, amount, description]() {
        return account->deposit(amount, description);
    });
    
    if (success) {
        databaseManager->updateAccount(*account);
        
        // Create transaction record
        Transaction transaction;
        transaction.setFromAccountId(accountId);
        transaction.setAmount(amount);
        transaction.setTransactionType("DEPOSIT");
        transaction.setDescription(description);
        transaction.setStatus("COMPLETED");
        transaction.setReferenceNumber(generateTransactionReference("DEP"));
        
        databaseManager->createTransaction(transaction);
        
        result.success = true;
        result.message = "Deposit successful";
        result.newBalance = account->getBalance();
        result.transactionId = transaction.getReferenceNumber();
        
        logTransaction(result, account->getUserId(), "DEPOSIT");
        sendTransactionNotification(account->getUserId(), "Deposit", amount, account->getAccountNumber());
    } else {
        result.message = "Deposit failed";
    }
    
    return result;
}

BankService::TransactionResult BankService::withdraw(int accountId, double amount, const QString& description) {
    TransactionResult result;
    result.success = false;
    
    if (!validateTransactionPermission(accountId)) {
        result.message = "Permission denied";
        return result;
    }
    
    if (!validateAmount(amount)) {
        result.message = "Invalid amount";
        return result;
    }
    
    auto account = databaseManager->getAccountById(accountId);
    if (!account) {
        result.message = "Account not found";
        return result;
    }
    
    if (!account->hasSufficientBalance(amount)) {
        result.message = "Insufficient balance";
        return result;
    }
    
    bool success = performAtomicTransaction([this, account, amount, description]() {
        return account->withdraw(amount, description);
    });
    
    if (success) {
        databaseManager->updateAccount(*account);
        
        // Create transaction record
        Transaction transaction;
        transaction.setFromAccountId(accountId);
        transaction.setAmount(amount);
        transaction.setTransactionType("WITHDRAWAL");
        transaction.setDescription(description);
        transaction.setStatus("COMPLETED");
        transaction.setReferenceNumber(generateTransactionReference("WTH"));
        
        databaseManager->createTransaction(transaction);
        
        result.success = true;
        result.message = "Withdrawal successful";
        result.newBalance = account->getBalance();
        result.transactionId = transaction.getReferenceNumber();
        
        logTransaction(result, account->getUserId(), "WITHDRAWAL");
        sendTransactionNotification(account->getUserId(), "Withdrawal", amount, account->getAccountNumber());
    } else {
        result.message = "Withdrawal failed";
    }
    
    return result;
}

BankService::TransactionResult BankService::transfer(int fromAccountId, int toAccountId, double amount, 
                                                       const QString& description) {
    TransactionResult result;
    result.success = false;
    
    if (!validateTransactionPermission(fromAccountId)) {
        result.message = "Permission denied";
        return result;
    }
    
    if (!validateAmount(amount)) {
        result.message = "Invalid amount";
        return result;
    }
    
    auto fromAccount = databaseManager->getAccountById(fromAccountId);
    auto toAccount = databaseManager->getAccountById(toAccountId);
    
    if (!fromAccount) {
        result.message = "Source account not found";
        return result;
    }
    
    if (!toAccount) {
        result.message = "Destination account not found";
        return result;
    }
    
    if (!fromAccount->hasSufficientBalance(amount)) {
        result.message = "Insufficient balance";
        return result;
    }
    
    bool success = performAtomicTransaction([this, fromAccount, toAccount, amount, description]() {
        return fromAccount->transfer(amount, toAccount, description);
    });
    
    if (success) {
        databaseManager->updateAccount(*fromAccount);
        databaseManager->updateAccount(*toAccount);
        
        // Create transaction record
        Transaction transaction;
        transaction.setFromAccountId(fromAccountId);
        transaction.setToAccountId(toAccountId);
        transaction.setAmount(amount);
        transaction.setTransactionType("TRANSFER");
        transaction.setDescription(description);
        transaction.setStatus("COMPLETED");
        transaction.setReferenceNumber(generateTransactionReference("TRF"));
        
        databaseManager->createTransaction(transaction);
        
        result.success = true;
        result.message = "Transfer successful";
        result.newBalance = fromAccount->getBalance();
        result.transactionId = transaction.getReferenceNumber();
        
        logTransaction(result, fromAccount->getUserId(), "TRANSFER");
        sendTransactionNotification(fromAccount->getUserId(), "Transfer Out", amount, fromAccount->getAccountNumber());
        sendTransactionNotification(toAccount->getUserId(), "Transfer In", amount, toAccount->getAccountNumber());
    } else {
        result.message = "Transfer failed";
    }
    
    return result;
}

QList<std::shared_ptr<Account>> BankService::getUserAccounts(int userId) {
    return databaseManager->getAccountsByUserId(userId);
}

QList<std::shared_ptr<Transaction>> BankService::getAccountTransactions(int accountId) {
    return databaseManager->getTransactionsByAccountId(accountId);
}

double BankService::getAccountBalance(int accountId) {
    auto account = databaseManager->getAccountById(accountId);
    return account ? account->getBalance() : 0.0;
}

BankService::CustomerCreationResult BankService::createCustomer(const QString& username, const QString& email,
                                                                const QString& password, const QString& fullName,
                                                                const QString& phoneNumber) {
    CustomerCreationResult result;
    result.success = false;
    
    if (!authManager->canAccessAdminFeatures()) {
        result.message = "Admin privileges required";
        return result;
    }
    
    if (!validateEmail(email) || !validatePhoneNumber(phoneNumber) || !validatePasswordStrength(password)) {
        result.message = "Invalid input data";
        return result;
    }
    
    // Check if username already exists
    auto existingUser = databaseManager->getUserByUsername(username);
    if (existingUser) {
        result.message = "Username already exists";
        return result;
    }
    
    Customer customer;
    customer.setUsername(username);
    customer.setEmail(email);
    customer.setPasswordHash(User::hashPassword(password));
    customer.setFullName(fullName);
    customer.setPhoneNumber(phoneNumber);
    customer.setRole("CUSTOMER");
    customer.setTransactionPin(User::hashTransactionPin("1234")); // Default PIN
    
    bool created = databaseManager->createUser(customer);
    
    if (created) {
        result.success = true;
        result.message = "Customer created successfully";
        result.customerId = customer.getUserId();
        
        logAdminAction("CREATE_CUSTOMER", QString("Created customer: %1").arg(username));
        logger->log(Logger::INFO, Logger::ADMIN_ACTION, 
                     QString("New customer created: %1 (%2)").arg(username).arg(email));
    } else {
        result.message = "Failed to create customer";
    }
    
    return result;
}

bool BankService::createAccount(int userId, const QString& accountType, double initialBalance) {
    if (!authManager->canAccessAdminFeatures()) {
        return false;
    }
    
    Account account;
    account.setUserId(userId);
    account.setAccountType(accountType);
    account.setBalance(initialBalance);
    account.setAccountNumber(Account::generateAccountNumber());
    
    bool created = databaseManager->createAccount(account);
    
    if (created) {
        logAdminAction("CREATE_ACCOUNT", QString("Created %1 account for user %2").arg(accountType).arg(userId));
    }
    
    return created;
}

bool BankService::suspendCustomer(int customerId, const QString& reason) {
    if (!authManager->canAccessAdminFeatures()) {
        return false;
    }
    
    bool suspended = databaseManager->suspendCustomer(customerId);
    
    if (suspended) {
        logAdminAction("SUSPEND_CUSTOMER", QString("Suspended customer %1: %2").arg(customerId).arg(reason));
    }
    
    return suspended;
}

bool BankService::activateCustomer(int customerId) {
    if (!authManager->canAccessAdminFeatures()) {
        return false;
    }
    
    bool activated = databaseManager->activateCustomer(customerId);
    
    if (activated) {
        logAdminAction("ACTIVATE_CUSTOMER", QString("Activated customer %1").arg(customerId));
    }
    
    return activated;
}

QList<std::shared_ptr<Customer>> BankService::getAllCustomers() {
    if (!authManager->canAccessAdminFeatures()) {
        return QList<std::shared_ptr<Customer>>();
    }
    
    QList<std::shared_ptr<Customer>> customers;
    auto users = databaseManager->getUsersByRole("CUSTOMER");
    
    for (const auto& user : users) {
        auto customer = std::make_shared<Customer>();
        // Copy user data to customer
        customer->setUserId(user->getUserId());
        customer->setUsername(user->getUsername());
        customer->setEmail(user->getEmail());
        customer->setFullName(user->getFullName());
        customer->setPhoneNumber(user->getPhoneNumber());
        customers.append(customer);
    }
    
    return customers;
}

QList<std::shared_ptr<Account>> BankService::getAllAccounts() {
    if (!authManager->canAccessAdminFeatures()) {
        return QList<std::shared_ptr<Account>>();
    }
    
    return databaseManager->getAllAccounts();
}

bool BankService::changePassword(const QString& oldPassword, const QString& newPassword) {
    return authManager->changePassword(oldPassword, newPassword);
}

bool BankService::changeTransactionPin(const QString& oldPin, const QString& newPin) {
    return authManager->changeTransactionPin(oldPin, newPin);
}

bool BankService::resetPassword(const QString& username, const QString& email, const QString& newPassword) {
    return authManager->resetPassword(username, email, newPassword);
}

bool BankService::closeAccount(int accountId) {
    if (!validateTransactionPermission(accountId)) {
        return false;
    }
    
    return databaseManager->closeAccount(accountId);
}

std::shared_ptr<Account> BankService::getAccountById(int accountId) {
    return databaseManager->getAccountById(accountId);
}

std::shared_ptr<Account> BankService::getAccountByNumber(const QString& accountNumber) {
    return databaseManager->getAccountByNumber(accountNumber);
}

std::shared_ptr<Transaction> BankService::getTransactionById(int transactionId) {
    return databaseManager->getTransactionById(transactionId);
}

QList<std::shared_ptr<Transaction>> BankService::getTransactionsByDateRange(const QDateTime& startDate, 
                                                                            const QDateTime& endDate) {
    return databaseManager->getTransactionsByDateRange(startDate, endDate);
}

QList<std::shared_ptr<Transaction>> BankService::getUserTransactions(int userId) {
    return databaseManager->getTransactionsByUserId(userId);
}

BankService::BankStatistics BankService::getBankStatistics() {
    BankStatistics stats;
    
    stats.totalCustomers = databaseManager->getTotalCustomers();
    stats.totalAccounts = databaseManager->getTotalAccounts();
    stats.totalDeposits = databaseManager->getTotalBankDeposits();
    stats.todayTransactions = databaseManager->getTotalTransactions();
    stats.todayTransactionVolume = databaseManager->getTodayTransactionsTotal();
    
    // Calculate active customers and accounts
    auto allUsers = databaseManager->getAllUsers();
    stats.activeCustomers = 0;
    for (const auto& user : allUsers) {
        if (user->isActiveStatus() && !user->isAccountLocked()) {
            stats.activeCustomers++;
        }
    }
    
    auto allAccounts = databaseManager->getAllAccounts();
    stats.activeAccounts = 0;
    for (const auto& account : allAccounts) {
        if (account->isActiveStatus()) {
            stats.activeAccounts++;
        }
    }
    
    return stats;
}

bool BankService::validateEmail(const QString& email) {
    return securityManager->validateEmail(email);
}

bool BankService::validatePhoneNumber(const QString& phone) {
    return securityManager->validatePhoneNumber(phone);
}

bool BankService::validatePasswordStrength(const QString& password) {
    return securityManager->checkPasswordStrength(password) != SecurityManager::WEAK;
}

bool BankService::validateTransactionPin(const QString& pin) {
    return securityManager->validateTransactionPin(pin);
}

bool BankService::validateAmount(double amount) {
    return securityManager->validateAmount(amount);
}

bool BankService::validateAccountNumber(const QString& accountNumber) {
    return securityManager->validateAccountNumber(accountNumber);
}

void BankService::logTransaction(const TransactionResult& result, int userId, const QString& operation) {
    if (result.success) {
        logger->log(Logger::INFO, Logger::TRANSACTION, 
                     QString("%1 successful: %2").arg(operation).arg(result.transactionId), userId);
    } else {
        logger->log(Logger::ERROR, Logger::TRANSACTION, 
                     QString("%1 failed: %2").arg(operation).arg(result.message), userId);
    }
}

void BankService::logAdminAction(const QString& action, const QString& details) {
    auto currentUser = authManager->getCurrentUser();
    if (currentUser) {
        logger->logAdminAction(currentUser->getUserId(), action, 0, details);
    }
}

bool BankService::validateTransactionPermission(int accountId) {
    auto currentUser = authManager->getCurrentUser();
    if (!currentUser) {
        return false;
    }
    
    if (currentUser->isAdmin()) {
        return true;
    }
    
    auto account = databaseManager->getAccountById(accountId);
    return account && account->getUserId() == currentUser->getUserId();
}

bool BankService::performAtomicTransaction(std::function<bool()> transaction) {
    bool success = databaseManager->beginTransaction();
    
    if (success) {
        try {
            success = transaction();
            if (success) {
                databaseManager->commitTransaction();
            } else {
                databaseManager->rollbackTransaction();
            }
        } catch (...) {
            databaseManager->rollbackTransaction();
            success = false;
        }
    }
    
    return success;
}

QString BankService::generateTransactionReference(const QString& type) {
    return QString("%1%2%3").arg(type).arg(QDateTime::currentDateTime().toString("yyyyMMdd")).arg(QRandomGenerator::global()->bounded(100000, 999999));
}

bool BankService::sendTransactionNotification(int userId, const QString& transactionType, 
                                               double amount, const QString& accountNumber) {
    QString subject = QString("Banking System - %1 Notification").arg(transactionType);
    QString body = QString(
        "Dear Customer,\n\n"
        "A %1 of %2 has been processed on your account %3.\n\n"
        "If you did not authorize this transaction, please contact our support team immediately.\n\n"
        "Best regards,\n"
        "Banking System"
    ).arg(transactionType).arg(amount).arg(accountNumber);
    
    return sendEmailNotification(userId, subject, body);
}

bool BankService::sendEmailNotification(int userId, const QString& subject, const QString& body) {
    auto user = databaseManager->getUserById(userId);
    if (!user) {
        return false;
    }
    
    return databaseManager->createEmailNotification(userId, "TRANSACTION", subject, body);
}

bool BankService::createBackup(const QString& backupPath) {
    return databaseManager->createBackup(backupPath);
}

bool BankService::restoreBackup(const QString& backupPath) {
    return databaseManager->restoreFromBackup(backupPath);
}

QString BankService::getBackupFileName() {
    return databaseManager->generateBackupFileName();
}

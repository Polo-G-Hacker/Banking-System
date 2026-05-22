#include "security/DatabaseManager.h"
#include "core/User.h"
#include "core/Customer.h"
#include "core/Admin.h"
#include "core/Account.h"
#include "core/Transaction.h"
#include <QSqlRecord>
#include <QRandomGenerator>
#include <QSqlField>
#include <QDebug>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QCryptographicHash>

DatabaseManager* DatabaseManager::instance = nullptr;

DatabaseManager::DatabaseManager() : isConnected(false) {
    const QStringList drivers = QSqlDatabase::drivers();
    if (drivers.contains("QMYSQL")) {
        database = QSqlDatabase::addDatabase("QMYSQL");
    } else {
        qDebug() << "QMYSQL driver not available. Falling back to QSQLITE.";
        database = QSqlDatabase::addDatabase("QSQLITE");
    }
}

DatabaseManager* DatabaseManager::getInstance() {
    if (!instance) {
        instance = new DatabaseManager();
    }
    return instance;
}

DatabaseManager::~DatabaseManager() {
    disconnect();
}

bool DatabaseManager::connect(const QString& hostname, const QString& databaseName, 
                              const QString& username, const QString& password, int port) {
    this->hostname = hostname;
    this->databaseName = databaseName;
    this->username = username;
    this->password = password;
    this->port = port;
    
    if (database.driverName() == "QSQLITE") {
        QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        if (dataPath.isEmpty()) {
            dataPath = QCoreApplication::applicationDirPath();
        }
        QDir().mkpath(dataPath);
        const QString sqlitePath = QDir(dataPath).filePath(databaseName + ".sqlite");
        qDebug() << "Using SQLite database:" << sqlitePath;
        database.setDatabaseName(sqlitePath);
    } else {
        database.setHostName(hostname);
        database.setDatabaseName(databaseName);
        database.setUserName(username);
        database.setPassword(password);
        database.setPort(port);
    }
    
    isConnected = database.open();
    
    if (isConnected) {
        initializeDatabase();
        qDebug() << "Database connected successfully";
    } else {
        qDebug() << "Database connection failed:" << database.lastError().text();
    }
    
    return isConnected;
}

void DatabaseManager::disconnect() {
    if (database.isOpen()) {
        database.close();
    }
    isConnected = false;
}

QString DatabaseManager::getLastDatabaseError() const {
    return database.lastError().text();
}

bool DatabaseManager::beginTransaction() {
    if (!isConnected) return false;
    return database.transaction();
}

bool DatabaseManager::commitTransaction() {
    if (!isConnected) return false;
    return database.commit();
}

bool DatabaseManager::rollbackTransaction() {
    if (!isConnected) return false;
    return database.rollback();
}

std::shared_ptr<User> DatabaseManager::authenticateUser(const QString& username, const QString& password) {
    QString hashedPassword = User::hashPassword(password);
    QSqlQuery query;
    query.prepare("SELECT * FROM users WHERE username = ? AND password_hash = ? AND is_active = TRUE");
    query.addBindValue(username);
    query.addBindValue(hashedPassword);
    
    if (query.exec() && query.next()) {
        return createUserFromQuery(query);
    }
    
    return nullptr;
}

std::shared_ptr<User> DatabaseManager::getUserById(int userId) {
    QSqlQuery query;
    query.prepare("SELECT * FROM users WHERE user_id = ?");
    query.addBindValue(userId);
    
    if (query.exec() && query.next()) {
        return createUserFromQuery(query);
    }
    
    return nullptr;
}

std::shared_ptr<User> DatabaseManager::getUserByUsername(const QString& username) {
    QSqlQuery query;
    query.prepare("SELECT * FROM users WHERE username = ?");
    query.addBindValue(username);
    
    if (query.exec() && query.next()) {
        return createUserFromQuery(query);
    }
    
    return nullptr;
}

bool DatabaseManager::createUser(const User& user) {
    QSqlQuery query;
    query.prepare("INSERT INTO users (username, email, password_hash, phone_number, full_name, role, transaction_pin) "
                 "VALUES (?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(user.getUsername());
    query.addBindValue(user.getEmail());
    query.addBindValue(user.getPasswordHash());
    query.addBindValue(encryptField(user.getPhoneNumber()));
    query.addBindValue(user.getFullName());
    query.addBindValue(user.getRole());
    query.addBindValue(user.getTransactionPin());
    
    return query.exec();
}

bool DatabaseManager::updateUser(const User& user) {
    QSqlQuery query;
    query.prepare("UPDATE users SET username = ?, email = ?, password_hash = ?, phone_number = ?, "
                 "full_name = ?, role = ?, transaction_pin = ?, account_locked = ?, lock_until = ?, "
                 "failed_login_attempts = ?, is_active = ? WHERE user_id = ?");
    query.addBindValue(user.getUsername());
    query.addBindValue(user.getEmail());
    query.addBindValue(user.getPasswordHash());
    query.addBindValue(encryptField(user.getPhoneNumber()));
    query.addBindValue(user.getFullName());
    query.addBindValue(user.getRole());
    query.addBindValue(user.getTransactionPin());
    query.addBindValue(user.isAccountLocked());
    query.addBindValue(user.getLockUntil());
    query.addBindValue(user.getFailedLoginAttempts());
    query.addBindValue(user.isActiveStatus());
    query.addBindValue(user.getUserId());
    
    return query.exec();
}

bool DatabaseManager::deleteUser(int userId) {
    QSqlQuery query;
    query.prepare("DELETE FROM users WHERE user_id = ?");
    query.addBindValue(userId);
    
    return query.exec();
}

QList<std::shared_ptr<User>> DatabaseManager::getAllUsers() {
    QList<std::shared_ptr<User>> users;
    QSqlQuery query("SELECT * FROM users ORDER BY user_id");
    
    while (query.next()) {
        users.append(createUserFromQuery(query));
    }
    
    return users;
}

QList<std::shared_ptr<User>> DatabaseManager::getUsersByRole(const QString& role) {
    QList<std::shared_ptr<User>> users;
    QSqlQuery query;
    query.prepare("SELECT * FROM users WHERE role = ? ORDER BY user_id");
    query.addBindValue(role);
    
    while (query.next()) {
        users.append(createUserFromQuery(query));
    }
    
    return users;
}

std::shared_ptr<Account> DatabaseManager::getAccountById(int accountId) {
    QSqlQuery query;
    query.prepare("SELECT * FROM accounts WHERE account_id = ?");
    query.addBindValue(accountId);
    
    if (query.exec() && query.next()) {
        return createAccountFromQuery(query);
    }
    
    return nullptr;
}

std::shared_ptr<Account> DatabaseManager::getAccountByNumber(const QString& accountNumber) {
    QSqlQuery query;
    query.prepare("SELECT * FROM accounts WHERE account_number = ?");
    query.addBindValue(encryptField(accountNumber));
    
    if (query.exec() && query.next()) {
        return createAccountFromQuery(query);
    }
    
    return nullptr;
}

QList<std::shared_ptr<Account>> DatabaseManager::getAccountsByUserId(int userId) {
    QList<std::shared_ptr<Account>> accounts;
    QSqlQuery query;
    query.prepare("SELECT * FROM accounts WHERE user_id = ? ORDER BY created_at DESC");
    query.addBindValue(userId);
    
    while (query.next()) {
        accounts.append(createAccountFromQuery(query));
    }
    
    return accounts;
}

bool DatabaseManager::createAccount(const Account& account) {
    QSqlQuery query;
    query.prepare("INSERT INTO accounts (account_number, user_id, account_type, balance, is_active) "
                 "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(encryptField(account.getAccountNumber()));
    query.addBindValue(account.getUserId());
    query.addBindValue(account.getAccountType());
    query.addBindValue(account.getBalance());
    query.addBindValue(account.isActiveStatus());
    
    return query.exec();
}

std::shared_ptr<Transaction> DatabaseManager::getTransactionById(int transactionId) {
    QSqlQuery query;
    query.prepare("SELECT * FROM transactions WHERE transaction_id = ?");
    query.addBindValue(transactionId);
    
    if (query.exec() && query.next()) {
        return createTransactionFromQuery(query);
    }
    
    return nullptr;
}

QList<std::shared_ptr<Transaction>> DatabaseManager::getTransactionsByAccountId(int accountId) {
    QList<std::shared_ptr<Transaction>> transactions;
    QSqlQuery query;
    query.prepare("SELECT * FROM transactions WHERE from_account_id = ? OR to_account_id = ? "
                 "ORDER BY transaction_date DESC");
    query.addBindValue(accountId);
    query.addBindValue(accountId);
    
    while (query.next()) {
        transactions.append(createTransactionFromQuery(query));
    }
    
    return transactions;
}

bool DatabaseManager::createTransaction(const Transaction& transaction) {
    QSqlQuery query;
    query.prepare("INSERT INTO transactions (from_account_id, to_account_id, amount, transaction_type, "
                 "description, status, reference_number) VALUES (?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(transaction.getFromAccountId());
    query.addBindValue(transaction.getToAccountId());
    query.addBindValue(transaction.getAmount());
    query.addBindValue(transaction.getTransactionType());
    query.addBindValue(transaction.getDescription());
    query.addBindValue(transaction.getStatus());
    query.addBindValue(transaction.getReferenceNumber());
    
    return query.exec();
}

bool DatabaseManager::logActivity(int userId, const QString& action, const QString& description, 
                                   const QString& status, const QString& ipAddress) {
    QSqlQuery query;
    query.prepare("INSERT INTO audit_logs (user_id, action, description, ip_address, status) "
                 "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(userId);
    query.addBindValue(action);
    query.addBindValue(description);
    query.addBindValue(ipAddress);
    query.addBindValue(status);
    
    return query.exec();
}

bool DatabaseManager::logFailedLogin(const QString& username, const QString& ipAddress, const QString& reason) {
    QSqlQuery query;
    query.prepare("INSERT INTO failed_login_attempts (username, ip_address, reason) VALUES (?, ?, ?)");
    query.addBindValue(username);
    query.addBindValue(ipAddress);
    query.addBindValue(reason);

    return query.exec();
}

bool DatabaseManager::generateOTP(int userId, const QString& purpose, QString& otpCode) {
    // Generate 6-digit OTP
    otpCode = QString("%1").arg(QRandomGenerator::global()->bounded(100000, 999999));
    
    QSqlQuery query;
    if (database.driverName() == "QSQLITE") {
        query.prepare("INSERT INTO otp_codes (user_id, otp_code, purpose, expires_at) "
                      "VALUES (?, ?, ?, datetime('now', '+10 minutes'))");
    } else {
        query.prepare("INSERT INTO otp_codes (user_id, otp_code, purpose, expires_at) "
                      "VALUES (?, ?, ?, DATE_ADD(NOW(), INTERVAL 10 MINUTE))");
    }
    query.addBindValue(userId);
    query.addBindValue(otpCode);
    query.addBindValue(purpose);
    
    return query.exec();
}

bool DatabaseManager::validateOTP(int userId, const QString& otpCode, const QString& purpose) {
    QSqlQuery query;
    if (database.driverName() == "QSQLITE") {
        query.prepare("SELECT * FROM otp_codes WHERE user_id = ? AND otp_code = ? AND purpose = ? "
                      "AND is_used = 0 AND expires_at > datetime('now')");
    } else {
        query.prepare("SELECT * FROM otp_codes WHERE user_id = ? AND otp_code = ? AND purpose = ? "
                      "AND is_used = FALSE AND expires_at > NOW()");
    }
    query.addBindValue(userId);
    query.addBindValue(otpCode);
    query.addBindValue(purpose);
    
    if (query.exec() && query.next()) {
        // Mark OTP as used
        QSqlQuery updateQuery;
        updateQuery.prepare("UPDATE otp_codes SET is_used = TRUE WHERE otp_id = ?");
        updateQuery.addBindValue(query.value("otp_id").toInt());
        updateQuery.exec();
        
        return true;
    }
    
    return false;
}

void DatabaseManager::cleanupExpiredOTPs() {
    QSqlQuery query;
    if (database.driverName() == "QSQLITE") {
        query.prepare("DELETE FROM otp_codes WHERE expires_at <= datetime('now') OR is_used = 1");
    } else {
        query.prepare("DELETE FROM otp_codes WHERE expires_at <= NOW() OR is_used = TRUE");
    }
    query.exec();
}

int DatabaseManager::getFailedLoginAttempts(const QString& username, int hours) {
    QSqlQuery query;
    if (database.driverName() == "QSQLITE") {
        query.prepare("SELECT COUNT(*) FROM failed_login_attempts "
                      "WHERE username = ? AND attempt_time >= datetime('now', ?)");
        query.addBindValue(username);
        query.addBindValue(QString("-%1 hours").arg(hours));
    } else {
        query.prepare("SELECT COUNT(*) FROM failed_login_attempts "
                      "WHERE username = ? AND attempt_time >= DATE_SUB(NOW(), INTERVAL ? HOUR)");
        query.addBindValue(username);
        query.addBindValue(hours);
    }

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

void DatabaseManager::initializeDatabase() {
    if (database.driverName() != "QSQLITE") {
        return;
    }

    const QStringList statements = {
        "CREATE TABLE IF NOT EXISTS users ("
        "user_id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "username TEXT UNIQUE NOT NULL, "
        "email TEXT UNIQUE NOT NULL, "
        "password_hash TEXT NOT NULL, "
        "phone_number TEXT, "
        "full_name TEXT NOT NULL, "
        "role TEXT NOT NULL CHECK(role IN ('CUSTOMER', 'ADMIN')), "
        "transaction_pin TEXT, "
        "account_locked INTEGER DEFAULT 0, "
        "lock_until TEXT NULL, "
        "failed_login_attempts INTEGER DEFAULT 0, "
        "created_at TEXT DEFAULT CURRENT_TIMESTAMP, "
        "updated_at TEXT DEFAULT CURRENT_TIMESTAMP, "
        "is_active INTEGER DEFAULT 1)",

        "CREATE TABLE IF NOT EXISTS accounts ("
        "account_id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "account_number TEXT UNIQUE NOT NULL, "
        "user_id INTEGER NOT NULL, "
        "account_type TEXT NOT NULL CHECK(account_type IN ('SAVINGS', 'CHECKING', 'CURRENT')), "
        "balance REAL DEFAULT 0.00, "
        "is_active INTEGER DEFAULT 1, "
        "created_at TEXT DEFAULT CURRENT_TIMESTAMP, "
        "updated_at TEXT DEFAULT CURRENT_TIMESTAMP, "
        "FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE)",

        "CREATE TABLE IF NOT EXISTS transactions ("
        "transaction_id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "from_account_id INTEGER, "
        "to_account_id INTEGER, "
        "amount REAL NOT NULL, "
        "transaction_type TEXT NOT NULL CHECK(transaction_type IN ('DEPOSIT', 'WITHDRAWAL', 'TRANSFER')), "
        "description TEXT, "
        "transaction_date TEXT DEFAULT CURRENT_TIMESTAMP, "
        "status TEXT DEFAULT 'COMPLETED' CHECK(status IN ('PENDING', 'COMPLETED', 'FAILED', 'CANCELLED')), "
        "reference_number TEXT UNIQUE NOT NULL, "
        "FOREIGN KEY (from_account_id) REFERENCES accounts(account_id) ON DELETE SET NULL, "
        "FOREIGN KEY (to_account_id) REFERENCES accounts(account_id) ON DELETE SET NULL)",

        "CREATE TABLE IF NOT EXISTS audit_logs ("
        "log_id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "user_id INTEGER, "
        "action TEXT NOT NULL, "
        "description TEXT, "
        "ip_address TEXT, "
        "user_agent TEXT, "
        "status TEXT NOT NULL CHECK(status IN ('SUCCESS', 'FAILED', 'WARNING')), "
        "timestamp TEXT DEFAULT CURRENT_TIMESTAMP)",

        "CREATE TABLE IF NOT EXISTS otp_codes ("
        "otp_id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "user_id INTEGER NOT NULL, "
        "otp_code TEXT NOT NULL, "
        "purpose TEXT NOT NULL, "
        "is_used INTEGER DEFAULT 0, "
        "expires_at TEXT NOT NULL, "
        "created_at TEXT DEFAULT CURRENT_TIMESTAMP, "
        "FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE)",

        "CREATE TABLE IF NOT EXISTS failed_login_attempts ("
        "failed_id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "username TEXT NOT NULL, "
        "ip_address TEXT, "
        "attempt_time TEXT DEFAULT CURRENT_TIMESTAMP, "
        "reason TEXT)",

        "CREATE TABLE IF NOT EXISTS email_notifications ("
        "notification_id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "user_id INTEGER NOT NULL, "
        "email_type TEXT NOT NULL, "
        "subject TEXT NOT NULL, "
        "body TEXT NOT NULL, "
        "sent_at TEXT DEFAULT CURRENT_TIMESTAMP, "
        "is_sent INTEGER DEFAULT 0, "
        "FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE)",

        "CREATE INDEX IF NOT EXISTS idx_users_username ON users(username)",
        "CREATE INDEX IF NOT EXISTS idx_accounts_user_id ON accounts(user_id)",
        "CREATE INDEX IF NOT EXISTS idx_failed_login_attempts_username ON failed_login_attempts(username)"
    };

    for (const QString& statement : statements) {
        QSqlQuery query(database);
        if (!query.exec(statement)) {
            qDebug() << "SQLite initialization failed:" << query.lastError().text();
        }
    }

    QSqlQuery adminQuery(database);
    adminQuery.prepare("INSERT OR IGNORE INTO users "
                       "(username, email, password_hash, full_name, role, transaction_pin) "
                       "VALUES (?, ?, ?, ?, ?, ?)");
    adminQuery.addBindValue("admin");
    adminQuery.addBindValue("admin@bank.com");
    adminQuery.addBindValue(User::hashPassword("Admin123!@#"));
    adminQuery.addBindValue("System Administrator");
    adminQuery.addBindValue("ADMIN");
    adminQuery.addBindValue(User::hashTransactionPin("1234"));
    adminQuery.exec();

    QSqlQuery customerQuery(database);
    customerQuery.prepare("INSERT OR IGNORE INTO users "
                          "(username, email, password_hash, phone_number, full_name, role, transaction_pin) "
                          "VALUES (?, ?, ?, ?, ?, ?, ?)");
    customerQuery.addBindValue("john_doe");
    customerQuery.addBindValue("john@example.com");
    customerQuery.addBindValue(User::hashPassword("Customer123!@#"));
    customerQuery.addBindValue(encryptField("+1234567890"));
    customerQuery.addBindValue("John Doe");
    customerQuery.addBindValue("CUSTOMER");
    customerQuery.addBindValue(User::hashTransactionPin("5678"));
    customerQuery.exec();

    QSqlQuery accountQuery(database);
    accountQuery.prepare("INSERT OR IGNORE INTO accounts "
                         "(account_number, user_id, account_type, balance) "
                         "SELECT ?, user_id, ?, ? FROM users WHERE username = ?");
    accountQuery.addBindValue(encryptField("ACC100001"));
    accountQuery.addBindValue("SAVINGS");
    accountQuery.addBindValue(5000.00);
    accountQuery.addBindValue("john_doe");
    accountQuery.exec();
}

bool DatabaseManager::updateAccount(const Account& account) {
    QSqlQuery query(database);
    query.prepare("UPDATE accounts SET account_number = ?, user_id = ?, account_type = ?, "
                  "balance = ?, is_active = ? WHERE account_id = ?");
    query.addBindValue(encryptField(account.getAccountNumber()));
    query.addBindValue(account.getUserId());
    query.addBindValue(account.getAccountType());
    query.addBindValue(account.getBalance());
    query.addBindValue(account.isActiveStatus());
    query.addBindValue(account.getAccountId());
    return query.exec();
}

bool DatabaseManager::suspendCustomer(int customerId, int minutes) {
    QSqlQuery query(database);
    if (database.driverName() == "QSQLITE") {
        query.prepare("UPDATE users SET account_locked = 1, lock_until = datetime('now', ?) "
                      "WHERE user_id = ? AND role = 'CUSTOMER'");
        query.addBindValue(QString("+%1 minutes").arg(minutes));
    } else {
        query.prepare("UPDATE users SET account_locked = TRUE, lock_until = DATE_ADD(NOW(), INTERVAL ? MINUTE) "
                      "WHERE user_id = ? AND role = 'CUSTOMER'");
        query.addBindValue(minutes);
    }
    query.addBindValue(customerId);
    return query.exec();
}

bool DatabaseManager::activateCustomer(int customerId) {
    QSqlQuery query(database);
    query.prepare("UPDATE users SET account_locked = ?, lock_until = NULL, failed_login_attempts = 0, "
                  "is_active = ? WHERE user_id = ? AND role = 'CUSTOMER'");
    query.addBindValue(false);
    query.addBindValue(true);
    query.addBindValue(customerId);
    return query.exec();
}

QList<std::shared_ptr<Account>> DatabaseManager::getAllAccounts() {
    QList<std::shared_ptr<Account>> accounts;
    QSqlQuery query(database);
    query.prepare("SELECT * FROM accounts ORDER BY account_id");

    if (query.exec()) {
        while (query.next()) {
            accounts.append(createAccountFromQuery(query));
        }
    }

    return accounts;
}

bool DatabaseManager::closeAccount(int accountId) {
    QSqlQuery query(database);
    query.prepare("UPDATE accounts SET is_active = ? WHERE account_id = ?");
    query.addBindValue(false);
    query.addBindValue(accountId);
    return query.exec();
}

QList<std::shared_ptr<Transaction>> DatabaseManager::getTransactionsByDateRange(const QDateTime& startDate,
                                                                                const QDateTime& endDate) {
    QList<std::shared_ptr<Transaction>> transactions;
    QSqlQuery query(database);
    query.prepare("SELECT * FROM transactions WHERE transaction_date BETWEEN ? AND ? "
                  "ORDER BY transaction_date DESC");
    query.addBindValue(startDate);
    query.addBindValue(endDate);

    if (query.exec()) {
        while (query.next()) {
            transactions.append(createTransactionFromQuery(query));
        }
    }

    return transactions;
}

QList<std::shared_ptr<Transaction>> DatabaseManager::getTransactionsByUserId(int userId) {
    QList<std::shared_ptr<Transaction>> transactions;
    QSqlQuery query(database);
    query.prepare("SELECT DISTINCT t.* FROM transactions t "
                  "JOIN accounts a ON a.account_id = t.from_account_id OR a.account_id = t.to_account_id "
                  "WHERE a.user_id = ? ORDER BY t.transaction_date DESC");
    query.addBindValue(userId);

    if (query.exec()) {
        while (query.next()) {
            transactions.append(createTransactionFromQuery(query));
        }
    }

    return transactions;
}

int DatabaseManager::getTotalCustomers() {
    QSqlQuery query(database);
    query.prepare("SELECT COUNT(*) FROM users WHERE role = 'CUSTOMER'");
    return query.exec() && query.next() ? query.value(0).toInt() : 0;
}

int DatabaseManager::getTotalAccounts() {
    QSqlQuery query(database);
    query.prepare("SELECT COUNT(*) FROM accounts");
    return query.exec() && query.next() ? query.value(0).toInt() : 0;
}

int DatabaseManager::getTotalTransactions() {
    QSqlQuery query(database);
    query.prepare("SELECT COUNT(*) FROM transactions");
    return query.exec() && query.next() ? query.value(0).toInt() : 0;
}

double DatabaseManager::getTotalBankDeposits() {
    QSqlQuery query(database);
    query.prepare("SELECT COALESCE(SUM(balance), 0) FROM accounts WHERE is_active = ?");
    query.addBindValue(true);
    return query.exec() && query.next() ? query.value(0).toDouble() : 0.0;
}

double DatabaseManager::getTodayTransactionsTotal() {
    QSqlQuery query(database);
    if (database.driverName() == "QSQLITE") {
        query.prepare("SELECT COALESCE(SUM(amount), 0) FROM transactions WHERE date(transaction_date) = date('now')");
    } else {
        query.prepare("SELECT COALESCE(SUM(amount), 0) FROM transactions WHERE DATE(transaction_date) = CURDATE()");
    }
    return query.exec() && query.next() ? query.value(0).toDouble() : 0.0;
}

bool DatabaseManager::createEmailNotification(int userId, const QString& emailType,
                                              const QString& subject, const QString& body) {
    QSqlQuery query(database);
    query.prepare("INSERT INTO email_notifications (user_id, email_type, subject, body) VALUES (?, ?, ?, ?)");
    query.addBindValue(userId);
    query.addBindValue(emailType);
    query.addBindValue(subject);
    query.addBindValue(body);
    return query.exec();
}

bool DatabaseManager::createBackup(const QString& backupPath) {
    if (database.driverName() == "QSQLITE") {
        return QFile::copy(database.databaseName(), backupPath);
    }
    return false;
}

bool DatabaseManager::restoreFromBackup(const QString& backupPath) {
    if (database.driverName() != "QSQLITE" || !QFile::exists(backupPath)) {
        return false;
    }

    const QString targetPath = database.databaseName();
    database.close();
    QFile::remove(targetPath);
    const bool restored = QFile::copy(backupPath, targetPath);
    isConnected = database.open();
    return restored && isConnected;
}

QString DatabaseManager::generateBackupFileName() {
    return QString("banking_system_backup_%1.sqlite")
        .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
}

std::shared_ptr<User> DatabaseManager::createUserFromQuery(QSqlQuery& query) {
    auto user = std::make_shared<User>();
    user->setUserId(query.value("user_id").toInt());
    user->setUsername(query.value("username").toString());
    user->setEmail(query.value("email").toString());
    user->setPasswordHash(query.value("password_hash").toString());
    user->setPhoneNumber(decryptField(query.value("phone_number").toString()));
    user->setFullName(query.value("full_name").toString());
    user->setRole(query.value("role").toString());
    user->setTransactionPin(query.value("transaction_pin").toString());
    user->setAccountLocked(query.value("account_locked").toBool());
    user->setLockUntil(query.value("lock_until").toDateTime());
    user->setFailedLoginAttempts(query.value("failed_login_attempts").toInt());
    user->setCreatedAt(query.value("created_at").toDateTime());
    user->setUpdatedAt(query.value("updated_at").toDateTime());
    user->setIsActive(query.value("is_active").toBool());
    
    return user;
}

std::shared_ptr<Account> DatabaseManager::createAccountFromQuery(QSqlQuery& query) {
    auto account = std::make_shared<Account>();
    account->setAccountId(query.value("account_id").toInt());
    account->setAccountNumber(decryptField(query.value("account_number").toString()));
    account->setUserId(query.value("user_id").toInt());
    account->setAccountType(query.value("account_type").toString());
    account->setBalance(query.value("balance").toDouble());
    account->setIsActive(query.value("is_active").toBool());
    account->setCreatedAt(query.value("created_at").toDateTime());
    account->setUpdatedAt(query.value("updated_at").toDateTime());
    
    return account;
}

std::shared_ptr<Transaction> DatabaseManager::createTransactionFromQuery(QSqlQuery& query) {
    auto transaction = std::make_shared<Transaction>();
    transaction->setTransactionId(query.value("transaction_id").toInt());
    transaction->setFromAccountId(query.value("from_account_id").toInt());
    transaction->setToAccountId(query.value("to_account_id").toInt());
    transaction->setAmount(query.value("amount").toDouble());
    transaction->setTransactionType(query.value("transaction_type").toString());
    transaction->setDescription(query.value("description").toString());
    transaction->setTransactionDate(query.value("transaction_date").toDateTime());
    transaction->setStatus(query.value("status").toString());
    transaction->setReferenceNumber(query.value("reference_number").toString());
    
    return transaction;
}

QString DatabaseManager::encryptField(const QString& data) {
    return User::encryptData(data);
}

QString DatabaseManager::decryptField(const QString& encryptedData) {
    return User::decryptData(encryptedData);
}

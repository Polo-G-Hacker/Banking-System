#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <QString>
#include <QDateTime>
#include <QList>
#include <memory>

class Transaction;

class Account {
private:
    int accountId;
    QString accountNumber;
    int userId;
    QString accountType;
    double balance;
    bool isActive;
    QDateTime createdAt;
    QDateTime updatedAt;
    QList<std::shared_ptr<Transaction>> transactions;

public:
    Account();
    Account(int accountId, const QString& accountNumber, int userId, 
            const QString& accountType, double balance = 0.0);
    
    virtual ~Account() = default;

    // Getters
    int getAccountId() const { return accountId; }
    QString getAccountNumber() const { return accountNumber; }
    int getUserId() const { return userId; }
    QString getAccountType() const { return accountType; }
    double getBalance() const { return balance; }
    bool isActiveStatus() const { return isActive; }
    QDateTime getCreatedAt() const { return createdAt; }
    QDateTime getUpdatedAt() const { return updatedAt; }
    QList<std::shared_ptr<Transaction>> getTransactions() const { return transactions; }

    // Setters
    void setAccountId(int id) { accountId = id; }
    void setAccountNumber(const QString& number) { accountNumber = number; }
    void setUserId(int id) { userId = id; }
    void setAccountType(const QString& type) { accountType = type; }
    void setBalance(double bal) { balance = bal; updatedAt = QDateTime::currentDateTime(); }
    void setIsActive(bool active) { isActive = active; updatedAt = QDateTime::currentDateTime(); }
    void setCreatedAt(const QDateTime& created) { createdAt = created; }
    void setUpdatedAt(const QDateTime& updated) { updatedAt = updated; }

    // Account operations
    bool deposit(double amount, const QString& description = "");
    bool withdraw(double amount, const QString& description = "");
    bool transfer(double amount, std::shared_ptr<Account> toAccount, const QString& description = "");
    
    // Transaction management
    void addTransaction(std::shared_ptr<Transaction> transaction);
    QList<std::shared_ptr<Transaction>> getRecentTransactions(int limit = 10) const;
    QList<std::shared_ptr<Transaction>> getTransactionsByDateRange(const QDateTime& startDate, 
                                                                   const QDateTime& endDate) const;
    
    // Account validation
    bool isValidForTransaction() const;
    bool hasSufficientBalance(double amount) const;
    
    // Utility methods
    QString getFormattedBalance() const;
    QString getAccountTypeDisplay() const;
    static QString generateAccountNumber();
};

#endif // ACCOUNT_H

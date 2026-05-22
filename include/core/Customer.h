#ifndef CUSTOMER_H
#define CUSTOMER_H

#include "User.h"
#include <QList>
#include <memory>

class Account;
class Transaction;

class Customer : public User {
private:
    QList<std::shared_ptr<Account>> accounts;
    QList<std::shared_ptr<Transaction>> transactions;

public:
    Customer();
    Customer(int userId, const QString& username, const QString& email, 
             const QString& passwordHash, const QString& phoneNumber, 
             const QString& fullName);
    
    virtual ~Customer() = default;

    // Account management
    void addAccount(std::shared_ptr<Account> account);
    void removeAccount(int accountId);
    QList<std::shared_ptr<Account>> getAccounts() const { return accounts; }
    std::shared_ptr<Account> getAccountById(int accountId) const;
    std::shared_ptr<Account> getAccountByNumber(const QString& accountNumber) const;
    double getTotalBalance() const;

    // Transaction management
    void addTransaction(std::shared_ptr<Transaction> transaction);
    QList<std::shared_ptr<Transaction>> getTransactions() const { return transactions; }
    QList<std::shared_ptr<Transaction>> getTransactionsByAccount(int accountId) const;
    QList<std::shared_ptr<Transaction>> getTransactionsByDateRange(const QDateTime& startDate, 
                                                                   const QDateTime& endDate) const;

    // Customer-specific operations
    bool canPerformTransaction(double amount) const;
    bool verifyTransactionPin(const QString& pin) const;
    void changeTransactionPin(const QString& oldPin, const QString& newPin);
    
    // Override User methods
    bool isAdmin() const override { return false; }
    bool isCustomer() const override { return true; }
};

#endif // CUSTOMER_H

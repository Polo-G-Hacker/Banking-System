#include "core/Customer.h"
#include "core/Account.h"
#include "core/Transaction.h"
#include <QDebug>

Customer::Customer() : User() {
    role = "CUSTOMER";
}

Customer::Customer(int userId, const QString& username, const QString& email, 
                   const QString& passwordHash, const QString& phoneNumber, 
                   const QString& fullName)
    : User(userId, username, email, passwordHash, phoneNumber, fullName, "CUSTOMER") {
}

void Customer::addAccount(std::shared_ptr<Account> account) {
    if (account && account->getUserId() == userId) {
        accounts.append(account);
    }
}

void Customer::removeAccount(int accountId) {
    for (int i = 0; i < accounts.size(); ++i) {
        if (accounts[i]->getAccountId() == accountId) {
            accounts.removeAt(i);
            break;
        }
    }
}

std::shared_ptr<Account> Customer::getAccountById(int accountId) const {
    for (const auto& account : accounts) {
        if (account->getAccountId() == accountId) {
            return account;
        }
    }
    return nullptr;
}

std::shared_ptr<Account> Customer::getAccountByNumber(const QString& accountNumber) const {
    for (const auto& account : accounts) {
        if (account->getAccountNumber() == accountNumber) {
            return account;
        }
    }
    return nullptr;
}

double Customer::getTotalBalance() const {
    double total = 0.0;
    for (const auto& account : accounts) {
        total += account->getBalance();
    }
    return total;
}

void Customer::addTransaction(std::shared_ptr<Transaction> transaction) {
    if (transaction) {
        transactions.append(transaction);
    }
}

QList<std::shared_ptr<Transaction>> Customer::getTransactionsByAccount(int accountId) const {
    QList<std::shared_ptr<Transaction>> result;
    for (const auto& transaction : transactions) {
        if (transaction->getFromAccountId() == accountId || 
            transaction->getToAccountId() == accountId) {
            result.append(transaction);
        }
    }
    return result;
}

QList<std::shared_ptr<Transaction>> Customer::getTransactionsByDateRange(
    const QDateTime& startDate, const QDateTime& endDate) const {
    QList<std::shared_ptr<Transaction>> result;
    for (const auto& transaction : transactions) {
        QDateTime transactionDate = transaction->getTransactionDate();
        if (transactionDate >= startDate && transactionDate <= endDate) {
            result.append(transaction);
        }
    }
    return result;
}

bool Customer::canPerformTransaction(double amount) const {
    if (amount <= 0) {
        return false;
    }
    
    // Check if customer has sufficient balance in any account
    for (const auto& account : accounts) {
        if (account->getBalance() >= amount && account->isActiveStatus()) {
            return true;
        }
    }
    
    return false;
}

bool Customer::verifyTransactionPin(const QString& pin) const {
    QString hashedPin = User::hashTransactionPin(pin);
    return hashedPin == transactionPin;
}

void Customer::changeTransactionPin(const QString& oldPin, const QString& newPin) {
    if (verifyTransactionPin(oldPin)) {
        transactionPin = User::hashTransactionPin(newPin);
        updatedAt = QDateTime::currentDateTime();
    }
}

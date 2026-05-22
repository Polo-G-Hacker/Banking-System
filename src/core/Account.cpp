#include "core/Account.h"
#include "core/Transaction.h"
#include <QDateTime>
#include <QLocale>
#include <QDebug>

Account::Account() 
    : accountId(0), userId(0), balance(0.0), isActive(true) {
    createdAt = QDateTime::currentDateTime();
    updatedAt = QDateTime::currentDateTime();
}

Account::Account(int accountId, const QString& accountNumber, int userId, 
                 const QString& accountType, double balance)
    : accountId(accountId), accountNumber(accountNumber), userId(userId), 
      accountType(accountType), balance(balance), isActive(true) {
    createdAt = QDateTime::currentDateTime();
    updatedAt = QDateTime::currentDateTime();
}

bool Account::deposit(double amount, const QString& description) {
    if (amount <= 0 || !isActive) {
        return false;
    }
    
    balance += amount;
    updatedAt = QDateTime::currentDateTime();
    
    // Create transaction record
    auto transaction = std::make_shared<Transaction>();
    transaction->setFromAccountId(accountId);
    transaction->setAmount(amount);
    transaction->setTransactionType("DEPOSIT");
    transaction->setDescription(description.isEmpty() ? "Deposit" : description);
    transaction->setStatus("COMPLETED");
    transaction->setReferenceNumber(QString("DEP%1").arg(QDateTime::currentMSecsSinceEpoch()));
    
    addTransaction(transaction);
    return true;
}

bool Account::withdraw(double amount, const QString& description) {
    if (amount <= 0 || !hasSufficientBalance(amount) || !isActive) {
        return false;
    }
    
    balance -= amount;
    updatedAt = QDateTime::currentDateTime();
    
    // Create transaction record
    auto transaction = std::make_shared<Transaction>();
    transaction->setFromAccountId(accountId);
    transaction->setAmount(amount);
    transaction->setTransactionType("WITHDRAWAL");
    transaction->setDescription(description.isEmpty() ? "Withdrawal" : description);
    transaction->setStatus("COMPLETED");
    transaction->setReferenceNumber(QString("WTH%1").arg(QDateTime::currentMSecsSinceEpoch()));
    
    addTransaction(transaction);
    return true;
}

bool Account::transfer(double amount, std::shared_ptr<Account> toAccount, const QString& description) {
    if (!toAccount || amount <= 0 || !hasSufficientBalance(amount) || !isActive || !toAccount->isActiveStatus()) {
        return false;
    }
    
    // Withdraw from this account
    balance -= amount;
    
    // Deposit to target account
    toAccount->balance += amount;
    
    // Update timestamps
    updatedAt = QDateTime::currentDateTime();
    toAccount->updatedAt = QDateTime::currentDateTime();
    
    // Create withdrawal transaction for this account
    auto withdrawalTransaction = std::make_shared<Transaction>();
    withdrawalTransaction->setFromAccountId(accountId);
    withdrawalTransaction->setToAccountId(toAccount->getAccountId());
    withdrawalTransaction->setAmount(amount);
    withdrawalTransaction->setTransactionType("TRANSFER");
    withdrawalTransaction->setDescription(description.isEmpty() ? "Transfer Out" : description);
    withdrawalTransaction->setStatus("COMPLETED");
    withdrawalTransaction->setReferenceNumber(QString("TRF%1").arg(QDateTime::currentMSecsSinceEpoch()));
    
    // Create deposit transaction for target account
    auto depositTransaction = std::make_shared<Transaction>();
    depositTransaction->setFromAccountId(accountId);
    depositTransaction->setToAccountId(toAccount->getAccountId());
    depositTransaction->setAmount(amount);
    depositTransaction->setTransactionType("TRANSFER");
    depositTransaction->setDescription(description.isEmpty() ? "Transfer In" : description);
    depositTransaction->setStatus("COMPLETED");
    depositTransaction->setReferenceNumber(QString("TRF%1").arg(QDateTime::currentMSecsSinceEpoch()));
    
    addTransaction(withdrawalTransaction);
    toAccount->addTransaction(depositTransaction);
    
    return true;
}

void Account::addTransaction(std::shared_ptr<Transaction> transaction) {
    if (transaction) {
        transactions.append(transaction);
    }
}

QList<std::shared_ptr<Transaction>> Account::getRecentTransactions(int limit) const {
    QList<std::shared_ptr<Transaction>> recentTransactions;
    int count = qMin(limit, transactions.size());
    
    for (int i = transactions.size() - count; i < transactions.size(); ++i) {
        recentTransactions.append(transactions[i]);
    }
    
    return recentTransactions;
}

QList<std::shared_ptr<Transaction>> Account::getTransactionsByDateRange(
    const QDateTime& startDate, const QDateTime& endDate) const {
    QList<std::shared_ptr<Transaction>> filteredTransactions;
    
    for (const auto& transaction : transactions) {
        QDateTime transactionDate = transaction->getTransactionDate();
        if (transactionDate >= startDate && transactionDate <= endDate) {
            filteredTransactions.append(transaction);
        }
    }
    
    return filteredTransactions;
}

bool Account::isValidForTransaction() const {
    return isActive && accountId > 0;
}

bool Account::hasSufficientBalance(double amount) const {
    return balance >= amount;
}

QString Account::getFormattedBalance() const {
    QLocale locale = QLocale::system();
    return locale.toString(balance, 'f', 2);
}

QString Account::getAccountTypeDisplay() const {
    if (accountType == "SAVINGS") {
        return "Savings Account";
    } else if (accountType == "CHECKING") {
        return "Checking Account";
    } else if (accountType == "CURRENT") {
        return "Current Account";
    }
    return accountType;
}

QString Account::generateAccountNumber() {
    static int counter = 100001;
    return QString("ACC%1").arg(counter++);
}

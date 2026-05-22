#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <QString>
#include <QDateTime>

class Transaction {
private:
    int transactionId;
    int fromAccountId;
    int toAccountId;
    double amount;
    QString transactionType;
    QString description;
    QDateTime transactionDate;
    QString status;
    QString referenceNumber;

public:
    Transaction();
    Transaction(int transactionId, int fromAccountId, int toAccountId, 
                double amount, const QString& transactionType, 
                const QString& description = "");
    
    virtual ~Transaction() = default;

    // Getters
    int getTransactionId() const { return transactionId; }
    int getFromAccountId() const { return fromAccountId; }
    int getToAccountId() const { return toAccountId; }
    double getAmount() const { return amount; }
    QString getTransactionType() const { return transactionType; }
    QString getDescription() const { return description; }
    QDateTime getTransactionDate() const { return transactionDate; }
    QString getStatus() const { return status; }
    QString getReferenceNumber() const { return referenceNumber; }

    // Setters
    void setTransactionId(int id) { transactionId = id; }
    void setFromAccountId(int accountId) { fromAccountId = accountId; }
    void setToAccountId(int accountId) { toAccountId = accountId; }
    void setAmount(double amt) { amount = amt; }
    void setTransactionType(const QString& type) { transactionType = type; }
    void setDescription(const QString& desc) { description = desc; }
    void setTransactionDate(const QDateTime& date) { transactionDate = date; }
    void setStatus(const QString& stat) { status = stat; }
    void setReferenceNumber(const QString& refNum) { referenceNumber = refNum; }

    // Utility methods
    QString getFormattedAmount() const;
    QString getTransactionTypeDisplay() const;
    QString getStatusDisplay() const;
    QString getFormattedDate() const;
    bool isDeposit() const { return transactionType == "DEPOSIT"; }
    bool isWithdrawal() const { return transactionType == "WITHDRAWAL"; }
    bool isTransfer() const { return transactionType == "TRANSFER"; }
    bool isCompleted() const { return status == "COMPLETED"; }
    bool isPending() const { return status == "PENDING"; }
    bool isFailed() const { return status == "FAILED"; }
    
    static QString generateReferenceNumber(const QString& prefix = "TRX");
};

#endif // TRANSACTION_H

#include "core/Transaction.h"
#include <QDateTime>
#include <QLocale>

Transaction::Transaction() 
    : transactionId(0), fromAccountId(0), toAccountId(0), amount(0.0) {
    transactionDate = QDateTime::currentDateTime();
    status = "PENDING";
    referenceNumber = generateReferenceNumber();
}

Transaction::Transaction(int transactionId, int fromAccountId, int toAccountId, 
                        double amount, const QString& transactionType, 
                        const QString& description)
    : transactionId(transactionId), fromAccountId(fromAccountId), toAccountId(toAccountId),
      amount(amount), transactionType(transactionType), description(description) {
    transactionDate = QDateTime::currentDateTime();
    status = "PENDING";
    referenceNumber = generateReferenceNumber();
}

QString Transaction::getFormattedAmount() const {
    QLocale locale = QLocale::system();
    return locale.toString(amount, 'f', 2);
}

QString Transaction::getTransactionTypeDisplay() const {
    if (transactionType == "DEPOSIT") {
        return "Deposit";
    } else if (transactionType == "WITHDRAWAL") {
        return "Withdrawal";
    } else if (transactionType == "TRANSFER") {
        return "Transfer";
    }
    return transactionType;
}

QString Transaction::getStatusDisplay() const {
    if (status == "COMPLETED") {
        return "Completed";
    } else if (status == "PENDING") {
        return "Pending";
    } else if (status == "FAILED") {
        return "Failed";
    } else if (status == "CANCELLED") {
        return "Cancelled";
    }
    return status;
}

QString Transaction::getFormattedDate() const {
    return transactionDate.toString("yyyy-MM-dd hh:mm:ss");
}

QString Transaction::generateReferenceNumber(const QString& prefix) {
    static int counter = 1;
    return QString("%1%2%3").arg(prefix).arg(QDateTime::currentDateTime().toString("yyyyMMdd")).arg(counter++, 6, 10, QChar('0'));
}

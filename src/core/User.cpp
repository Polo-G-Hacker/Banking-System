#include "core/User.h"
#include <QRegularExpression>
#include <QDebug>
#include <QCryptographicHash>

User::User() 
    : userId(0), accountLocked(false), failedLoginAttempts(0), isActive(true) {
}

User::User(int userId, const QString& username, const QString& email, 
           const QString& passwordHash, const QString& phoneNumber, 
           const QString& fullName, const QString& role)
    : userId(userId), username(username), email(email), passwordHash(passwordHash),
      phoneNumber(phoneNumber), fullName(fullName), role(role), accountLocked(false),
      failedLoginAttempts(0), isActive(true) {
    createdAt = QDateTime::currentDateTime();
    updatedAt = QDateTime::currentDateTime();
}

QString User::hashPassword(const QString& password) {
    return QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex();
}

QString User::hashTransactionPin(const QString& pin) {
    return QCryptographicHash::hash(pin.toUtf8(), QCryptographicHash::Sha256).toHex();
}

bool User::validatePasswordStrength(const QString& password) {
    if (password.length() < 8) {
        return false;
    }
    
    bool hasUpper = false;
    bool hasLower = false;
    bool hasNumber = false;
    bool hasSpecial = false;
    
    for (const QChar& ch : password) {
        if (ch.isUpper()) hasUpper = true;
        else if (ch.isLower()) hasLower = true;
        else if (ch.isNumber()) hasNumber = true;
        else if (!ch.isLetterOrNumber()) hasSpecial = true;
    }
    
    return hasUpper && hasLower && hasNumber && hasSpecial;
}

bool User::validateEmailFormat(const QString& email) {
    QRegularExpression emailRegex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    return emailRegex.match(email).hasMatch();
}

bool User::validatePhoneNumber(const QString& phone) {
    QRegularExpression phoneRegex(R"(^[\+]?[1-9][\d]{0,15}$)");
    return phoneRegex.match(phone).hasMatch();
}

QString User::encryptData(const QString& data, int shift) {
    QString encrypted;
    for (const QChar& ch : data) {
        if (ch.isLetter()) {
            QChar base = ch.isUpper() ? 'A' : 'a';
            encrypted += QChar((ch.unicode() - base.unicode() + shift) % 26 + base.unicode());
        } else if (ch.isNumber()) {
            encrypted += QChar((ch.unicode() - '0' + shift) % 10 + '0');
        } else {
            encrypted += ch;
        }
    }
    return encrypted;
}

QString User::decryptData(const QString& encryptedData, int shift) {
    QString decrypted;
    for (const QChar& ch : encryptedData) {
        if (ch.isLetter()) {
            QChar base = ch.isUpper() ? 'A' : 'a';
            decrypted += QChar((ch.unicode() - base.unicode() - shift + 26) % 26 + base.unicode());
        } else if (ch.isNumber()) {
            decrypted += QChar((ch.unicode() - '0' - shift + 10) % 10 + '0');
        } else {
            decrypted += ch;
        }
    }
    return decrypted;
}

void User::incrementFailedLoginAttempts() {
    failedLoginAttempts++;
    updatedAt = QDateTime::currentDateTime();
}

void User::resetFailedLoginAttempts() {
    failedLoginAttempts = 0;
    updatedAt = QDateTime::currentDateTime();
}

bool User::isAccountTemporarilyLocked() const {
    return accountLocked && lockUntil > QDateTime::currentDateTime();
}

void User::lockAccount(int minutes) {
    accountLocked = true;
    lockUntil = QDateTime::currentDateTime().addSecs(minutes * 60);
    updatedAt = QDateTime::currentDateTime();
}

void User::unlockAccount() {
    accountLocked = false;
    lockUntil = QDateTime();
    failedLoginAttempts = 0;
    updatedAt = QDateTime::currentDateTime();
}

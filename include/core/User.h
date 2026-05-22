#ifndef USER_H
#define USER_H

#include <QString>
#include <QDateTime>
#include <QCryptographicHash>

class User {
protected:
    int userId;
    QString username;
    QString email;
    QString passwordHash;
    QString phoneNumber;
    QString fullName;
    QString role;
    QString transactionPin;
    bool accountLocked;
    QDateTime lockUntil;
    int failedLoginAttempts;
    QDateTime createdAt;
    QDateTime updatedAt;
    bool isActive;

public:
    User();
    User(int userId, const QString& username, const QString& email, 
         const QString& passwordHash, const QString& phoneNumber, 
         const QString& fullName, const QString& role);
    
    virtual ~User() = default;

    // Getters
    int getUserId() const { return userId; }
    QString getUsername() const { return username; }
    QString getEmail() const { return email; }
    QString getPasswordHash() const { return passwordHash; }
    QString getPhoneNumber() const { return phoneNumber; }
    QString getFullName() const { return fullName; }
    QString getRole() const { return role; }
    QString getTransactionPin() const { return transactionPin; }
    bool isAccountLocked() const { return accountLocked; }
    QDateTime getLockUntil() const { return lockUntil; }
    int getFailedLoginAttempts() const { return failedLoginAttempts; }
    QDateTime getCreatedAt() const { return createdAt; }
    QDateTime getUpdatedAt() const { return updatedAt; }
    bool isActiveStatus() const { return isActive; }

    // Setters
    void setUserId(int id) { userId = id; }
    void setUsername(const QString& user) { username = user; }
    void setEmail(const QString& mail) { email = mail; }
    void setPasswordHash(const QString& hash) { passwordHash = hash; }
    void setPhoneNumber(const QString& phone) { phoneNumber = phone; }
    void setFullName(const QString& name) { fullName = name; }
    void setRole(const QString& userRole) { role = userRole; }
    void setTransactionPin(const QString& pin) { transactionPin = pin; }
    void setAccountLocked(bool locked) { accountLocked = locked; }
    void setLockUntil(const QDateTime& lockTime) { lockUntil = lockTime; }
    void setFailedLoginAttempts(int attempts) { failedLoginAttempts = attempts; }
    void setCreatedAt(const QDateTime& created) { createdAt = created; }
    void setUpdatedAt(const QDateTime& updated) { updatedAt = updated; }
    void setIsActive(bool active) { isActive = active; }

    // Utility methods
    static QString hashPassword(const QString& password);
    static QString hashTransactionPin(const QString& pin);
    static bool validatePasswordStrength(const QString& password);
    static bool validateEmailFormat(const QString& email);
    static bool validatePhoneNumber(const QString& phone);
    static QString encryptData(const QString& data, int shift = 3);
    static QString decryptData(const QString& encryptedData, int shift = 3);
    
    virtual bool isAdmin() const { return role == "ADMIN"; }
    virtual bool isCustomer() const { return role == "CUSTOMER"; }
    
    void incrementFailedLoginAttempts();
    void resetFailedLoginAttempts();
    bool isAccountTemporarilyLocked() const;
    void lockAccount(int minutes = 5);
    void unlockAccount();
};

#endif // USER_H

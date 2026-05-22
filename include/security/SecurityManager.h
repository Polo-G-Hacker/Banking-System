#ifndef SECURITYMANAGER_H
#define SECURITYMANAGER_H

#include <QString>
#include <QTimer>
#include <QDateTime>
#include <QRandomGenerator>

class SecurityManager {
private:
    static SecurityManager* instance;
    QTimer* sessionTimer;
    QDateTime lastActivity;
    int sessionTimeoutMinutes;
    bool sessionActive;
    
    SecurityManager();

public:
    static SecurityManager* getInstance();
    ~SecurityManager();
    
    // Session management
    void startSession();
    void endSession();
    void resetActivityTimer();
    bool isSessionActive() const { return sessionActive; }
    bool isSessionExpired() const;
    void setSessionTimeout(int minutes) { sessionTimeoutMinutes = minutes; }
    int getSessionTimeout() const { return sessionTimeoutMinutes; }
    
    // CAPTCHA generation
    QString generateMathCaptcha();
    QString generateAlphanumericCaptcha(int length = 6);
    bool validateMathCaptcha(const QString& question, const QString& answer);
    bool validateAlphanumericCaptcha(const QString& captcha, const QString& userInput);
    QString generateCaptcha(bool isAdmin = false);
    bool validateCaptcha(const QString& question, const QString& answer, bool isAdmin = false);
    
    // Password strength validation
    enum PasswordStrength {
        WEAK,
        MEDIUM,
        STRONG
    };
    
    PasswordStrength checkPasswordStrength(const QString& password);
    QString getPasswordStrengthDescription(PasswordStrength strength);
    QString getPasswordStrengthColor(PasswordStrength strength);
    
    // Input validation
    static bool validateEmail(const QString& email);
    static bool validatePhoneNumber(const QString& phone);
    static bool validateAmount(double amount);
    static bool validateAccountNumber(const QString& accountNumber);
    static bool validateTransactionPin(const QString& pin);
    static bool validateUsername(const QString& username);
    
    // Suspicious activity detection
    struct SuspiciousActivity {
        QString type;
        QString description;
        QString severity;
        QDateTime timestamp;
        int userId;
    };
    
    bool detectSuspiciousActivity(int userId, const QString& activity, const QString& details = "");
    QList<SuspiciousActivity> getRecentSuspiciousActivities(int userId, int hours = 24);
    void clearSuspiciousActivities(int userId);
    
    // Rate limiting
    struct RateLimit {
        int maxAttempts;
        int timeWindowMinutes;
        int currentAttempts;
        QDateTime windowStart;
    };
    
    bool isRateLimited(const QString& identifier, int maxAttempts = 5, int timeWindowMinutes = 15);
    void recordAttempt(const QString& identifier);
    void resetRateLimit(const QString& identifier);
    
    // Data encryption utilities
    static QString encryptSensitiveData(const QString& data, int shift = 3);
    static QString decryptSensitiveData(const QString& encryptedData, int shift = 3);
    
    // Security configuration
    void setSecurityLevel(const QString& level);
    QString getSecurityLevel() const;
    bool isHighSecurityMode() const;

private slots:
    void onSessionTimeout();

private:
    QMap<QString, RateLimit> rateLimits;
    QList<SuspiciousActivity> suspiciousActivities;
    QString securityLevel;
    
    QString generateRandomString(int length, bool includeNumbers = true, bool includeLetters = true);
    int calculatePasswordScore(const QString& password);
};

#endif // SECURITYMANAGER_H

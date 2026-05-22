#ifndef AUTHMANAGER_H
#define AUTHMANAGER_H

#include <QString>
#include <QTimer>
#include <QDateTime>
#include <memory>

class User;
class SecurityManager;
class DatabaseManager;
class OTPManager;

class AuthManager {
private:
    static AuthManager* instance;
    std::shared_ptr<User> currentUser;
    SecurityManager* securityManager;
    DatabaseManager* databaseManager;
    OTPManager* otpManager;
    QString sessionToken;
    QDateTime sessionStartTime;
    bool isAuthenticated;
    
    AuthManager();

public:
    static AuthManager* getInstance();
    ~AuthManager();

    void initialize();
    
    // Authentication
    struct LoginResult {
        bool success;
        QString message;
        QString captchaRequired;
        QString otpRequired;
        int userId;
    };
    
    LoginResult login(const QString& username, const QString& password, 
                       const QString& captchaAnswer = "", const QString& captchaQuestion = "");
    bool logout();
    bool isLoggedIn() const { return isAuthenticated && currentUser != nullptr; }
    
    // Two-factor authentication
    bool requestOTP(const QString& username, const QString& purpose = "LOGIN");
    bool verifyOTP(const QString& otpCode, const QString& purpose = "LOGIN");
    
    // Session management
    QString getSessionToken() const { return sessionToken; }
    QDateTime getSessionStartTime() const { return sessionStartTime; }
    int getSessionDurationMinutes() const;
    bool isSessionValid() const;
    void refreshSession();
    
    // User access
    std::shared_ptr<User> getCurrentUser() const { return currentUser; }
    int getCurrentUserId() const;
    QString getCurrentUsername() const;
    QString getCurrentUserRole() const;
    bool isCurrentUserAdmin() const;
    bool isCurrentUserCustomer() const;
    
    // Password management
    bool changePassword(const QString& oldPassword, const QString& newPassword);
    bool resetPassword(const QString& username, const QString& email, const QString& newPassword);
    bool validatePassword(const QString& password) const;
    
    // Transaction PIN management
    bool changeTransactionPin(const QString& oldPin, const QString& newPin);
    bool verifyTransactionPin(const QString& pin) const;
    
    // Account security
    bool isAccountLocked() const;
    int getFailedLoginAttempts() const;
    QDateTime getAccountLockTime() const;
    QString getAccountLockReason() const;
    
    // CAPTCHA validation
    bool validateCaptcha(const QString& question, const QString& answer, bool isAdmin = false);
    QString generateCaptcha(bool isAdmin = false);
    
    // Security checks
    bool canPerformTransaction(double amount) const;
    bool canAccessAdminFeatures() const;
    bool canAccessCustomerFeatures() const;
    
    // Audit and logging
    void logLoginAttempt(const QString& username, bool success, const QString& ipAddress = "");
    void logLogout();
    void logPasswordChange();
    void logTransactionPinChange();
    void logFailedAttempt(const QString& action, const QString& reason);
    
    // Security events
    void onSecurityEvent(const QString& eventType, const QString& details);
    void handleSuspiciousActivity(const QString& activity, const QString& details);

private:
    QString generateSessionToken();
    void setCurrentUser(std::shared_ptr<User> user);
    void clearSession();
    bool checkAccountLockStatus(const QString& username);
    void handleFailedLogin(const QString& username, const QString& reason);
    void handleSuccessfulLogin(const QString& username);
    bool validateUserCredentials(const QString& username, const QString& password);
};

#endif // AUTHMANAGER_H

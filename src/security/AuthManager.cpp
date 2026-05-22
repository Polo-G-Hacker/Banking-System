#include "security/AuthManager.h"
#include "security/SecurityManager.h"
#include "security/DatabaseManager.h"
#include "security/OTPManager.h"
#include "core/User.h"
#include <QCryptographicHash>
#include <QRandomGenerator>
#include <QDebug>

AuthManager* AuthManager::instance = nullptr;

void AuthManager::initialize() {
    isAuthenticated = false;
    currentUser = nullptr;
    sessionToken.clear();
    sessionStartTime = QDateTime();

    qDebug() << "AuthManager initialized";
}

AuthManager::AuthManager() {
    securityManager = SecurityManager::getInstance();
    databaseManager = DatabaseManager::getInstance();
    otpManager = OTPManager::getInstance();
    isAuthenticated = false;
}

AuthManager* AuthManager::getInstance() {
    if (!instance) {
        instance = new AuthManager();
    }
    return instance;
}

AuthManager::~AuthManager() {
    logout();
}

AuthManager::LoginResult AuthManager::login(const QString& username, const QString& password, 
                                              const QString& captchaAnswer, const QString& captchaQuestion) {
    LoginResult result;
    result.success = false;
    result.userId = 0;
    
    // Check if account is locked
    if (checkAccountLockStatus(username)) {
        result.message = "Account is temporarily locked. Please try again later.";
        return result;
    }
    
    // Validate CAPTCHA if provided
    if (!captchaAnswer.isEmpty() && !captchaQuestion.isEmpty()) {
        auto user = databaseManager->getUserByUsername(username);
        bool isAdminCaptcha = user && user->isAdmin();
        
        if (!securityManager->validateCaptcha(captchaQuestion, captchaAnswer, isAdminCaptcha)) {
            handleFailedLogin(username, "Invalid CAPTCHA");
            result.message = "Invalid CAPTCHA. Please try again.";
            result.captchaRequired = captchaQuestion;
            return result;
        }
    }
    
    // Validate user credentials
    if (!validateUserCredentials(username, password)) {
        handleFailedLogin(username, "Invalid credentials");
        result.message = "Invalid username or password.";
        
        // Check if we need to show CAPTCHA
        int failedAttempts = databaseManager->getFailedLoginAttempts(username, 1);
        if (failedAttempts >= 2) {
            result.captchaRequired = securityManager->generateCaptcha(
                databaseManager->getUserByUsername(username) && 
                databaseManager->getUserByUsername(username)->isAdmin());
        }
        
        return result;
    }
    
    // Get authenticated user
    auto user = databaseManager->authenticateUser(username, password);
    if (!user) {
        handleFailedLogin(username, "Authentication failed");
        result.message = "Authentication failed. Please try again.";
        return result;
    }
    
    // Check if user is active
    if (!user->isActiveStatus()) {
        result.message = "Account is deactivated. Please contact administrator.";
        return result;
    }
    
    // OTP is disabled in the current UI, so admin users continue directly after
    // password verification. Re-enable the OTP branch when OTPDialog is wired in.
    
    // For customer users, OTP is optional based on security level
    if (securityManager->isHighSecurityMode()) {
        result.otpRequired = "LOGIN";
        result.userId = user->getUserId();
        result.message = "OTP required for login.";
        
        if (!requestOTP(username, "LOGIN")) {
            result.message = "Failed to send OTP. Please try again.";
            return result;
        }
        
        return result;
    }
    
    // Complete login without OTP
    setCurrentUser(user);
    handleSuccessfulLogin(username);
    
    result.success = true;
    result.message = "Login successful.";
    result.userId = user->getUserId();
    
    return result;
}

bool AuthManager::logout() {
    if (isAuthenticated && currentUser) {
        logLogout();
        clearSession();
        return true;
    }
    return false;
}

bool AuthManager::requestOTP(const QString& username, const QString& purpose) {
    auto user = databaseManager->getUserByUsername(username);
    if (!user) {
        return false;
    }
    
    QString otpCode;
    bool generated = databaseManager->generateOTP(user->getUserId(), purpose, otpCode);
    
    if (generated) {
        // In a real implementation, this would send the OTP via email/SMS
        // For now, we'll just log it
        qDebug() << "OTP for" << username << ":" << otpCode;
        
        // Log OTP generation
        databaseManager->logActivity(user->getUserId(), "OTP_GENERATED", 
                                     QString("OTP generated for %1").arg(purpose), "SUCCESS");
        
        return true;
    }
    
    return false;
}

bool AuthManager::verifyOTP(const QString& otpCode, const QString& purpose) {
    if (!currentUser) {
        return false;
    }
    
    bool isValid = databaseManager->validateOTP(currentUser->getUserId(), otpCode, purpose);
    
    if (isValid) {
        databaseManager->logActivity(currentUser->getUserId(), "OTP_VERIFIED", 
                                     QString("OTP verified for %1").arg(purpose), "SUCCESS");
    } else {
        databaseManager->logActivity(currentUser->getUserId(), "OTP_FAILED", 
                                     QString("Invalid OTP for %1").arg(purpose), "FAILED");
    }
    
    return isValid;
}

int AuthManager::getSessionDurationMinutes() const {
    if (!isAuthenticated) {
        return 0;
    }
    
    return sessionStartTime.secsTo(QDateTime::currentDateTime()) / 60;
}

bool AuthManager::isSessionValid() const {
    return isAuthenticated && currentUser && 
           !securityManager->isSessionExpired() && 
           sessionStartTime.isValid();
}

void AuthManager::refreshSession() {
    if (isAuthenticated) {
        securityManager->resetActivityTimer();
        sessionToken = generateSessionToken();
    }
}

int AuthManager::getCurrentUserId() const {
    return currentUser ? currentUser->getUserId() : 0;
}

QString AuthManager::getCurrentUsername() const {
    return currentUser ? currentUser->getUsername() : "";
}

QString AuthManager::getCurrentUserRole() const {
    return currentUser ? currentUser->getRole() : "";
}

bool AuthManager::isCurrentUserAdmin() const {
    return currentUser && currentUser->isAdmin();
}

bool AuthManager::isCurrentUserCustomer() const {
    return currentUser && currentUser->isCustomer();
}

bool AuthManager::changePassword(const QString& oldPassword, const QString& newPassword) {
    if (!currentUser || !validatePassword(oldPassword)) {
        return false;
    }
    
    if (!User::validatePasswordStrength(newPassword)) {
        return false;
    }
    
    QString oldHashed = User::hashPassword(oldPassword);
    if (oldHashed != currentUser->getPasswordHash()) {
        logFailedAttempt("PASSWORD_CHANGE", "Invalid old password");
        return false;
    }
    
    currentUser->setPasswordHash(User::hashPassword(newPassword));
    bool updated = databaseManager->updateUser(*currentUser);
    
    if (updated) {
        logPasswordChange();
        databaseManager->logActivity(currentUser->getUserId(), "PASSWORD_CHANGED", 
                                     "User changed password", "SUCCESS");
    }
    
    return updated;
}

bool AuthManager::resetPassword(const QString& username, const QString& email, const QString& newPassword) {
    auto user = databaseManager->getUserByUsername(username);
    if (!user || user->getEmail() != email) {
        return false;
    }
    
    if (!User::validatePasswordStrength(newPassword)) {
        return false;
    }
    
    user->setPasswordHash(User::hashPassword(newPassword));
    user->resetFailedLoginAttempts();
    user->unlockAccount();
    
    bool updated = databaseManager->updateUser(*user);
    
    if (updated) {
        databaseManager->logActivity(user->getUserId(), "PASSWORD_RESET", 
                                     "Password reset completed", "SUCCESS");
    }
    
    return updated;
}

bool AuthManager::validatePassword(const QString& password) const {
    return User::validatePasswordStrength(password);
}

bool AuthManager::changeTransactionPin(const QString& oldPin, const QString& newPin) {
    if (!currentUser || !currentUser->isCustomer()) {
        return false;
    }
    
    if (!SecurityManager::validateTransactionPin(oldPin) || !SecurityManager::validateTransactionPin(newPin)) {
        return false;
    }
    
    QString oldHashed = User::hashTransactionPin(oldPin);
    if (oldHashed != currentUser->getTransactionPin()) {
        logFailedAttempt("PIN_CHANGE", "Invalid old PIN");
        return false;
    }
    
    currentUser->setTransactionPin(User::hashTransactionPin(newPin));
    bool updated = databaseManager->updateUser(*currentUser);
    
    if (updated) {
        logTransactionPinChange();
        databaseManager->logActivity(currentUser->getUserId(), "PIN_CHANGED", 
                                     "User changed transaction PIN", "SUCCESS");
    }
    
    return updated;
}

bool AuthManager::verifyTransactionPin(const QString& pin) const {
    if (!currentUser) {
        return false;
    }
    
    QString hashedPin = User::hashTransactionPin(pin);
    return hashedPin == currentUser->getTransactionPin();
}

bool AuthManager::isAccountLocked() const {
    return currentUser && currentUser->isAccountLocked();
}

int AuthManager::getFailedLoginAttempts() const {
    return currentUser ? currentUser->getFailedLoginAttempts() : 0;
}

QDateTime AuthManager::getAccountLockTime() const {
    return currentUser ? currentUser->getLockUntil() : QDateTime();
}

QString AuthManager::getAccountLockReason() const {
    if (!currentUser || !currentUser->isAccountLocked()) {
        return "";
    }
    
    return "Too many failed login attempts";
}

bool AuthManager::validateCaptcha(const QString& question, const QString& answer, bool isAdmin) {
    return securityManager->validateCaptcha(question, answer, isAdmin);
}

QString AuthManager::generateCaptcha(bool isAdmin) {
    return securityManager->generateCaptcha(isAdmin);
}

bool AuthManager::canPerformTransaction(double amount) const {
    if (!isAuthenticated || !currentUser) {
        return false;
    }
    
    return securityManager->validateAmount(amount) && currentUser->isActiveStatus();
}

bool AuthManager::canAccessAdminFeatures() const {
    return isCurrentUserAdmin() && isSessionValid();
}

bool AuthManager::canAccessCustomerFeatures() const {
    return isCurrentUserCustomer() && isSessionValid();
}

void AuthManager::logLoginAttempt(const QString& username, bool success, const QString& ipAddress) {
    databaseManager->logActivity(0, "LOGIN_ATTEMPT", 
                                QString("Login attempt for user: %1").arg(username), 
                                success ? "SUCCESS" : "FAILED", ipAddress);
    
    if (!success) {
        databaseManager->logFailedLogin(username, ipAddress, "Invalid credentials");
    }
}

void AuthManager::logLogout() {
    if (currentUser) {
        databaseManager->logActivity(currentUser->getUserId(), "LOGOUT", 
                                     "User logged out", "SUCCESS");
    }
}

void AuthManager::logPasswordChange() {
    if (currentUser) {
        databaseManager->logActivity(currentUser->getUserId(), "PASSWORD_CHANGE_ATTEMPT", 
                                     "User attempted to change password", "SUCCESS");
    }
}

void AuthManager::logTransactionPinChange() {
    if (currentUser) {
        databaseManager->logActivity(currentUser->getUserId(), "PIN_CHANGE_ATTEMPT", 
                                     "User attempted to change transaction PIN", "SUCCESS");
    }
}

void AuthManager::logFailedAttempt(const QString& action, const QString& reason) {
    if (currentUser) {
        databaseManager->logActivity(currentUser->getUserId(), action, reason, "FAILED");
    }
}

QString AuthManager::generateSessionToken() {
    QString data = QString("%1:%2:%3").arg(getCurrentUsername())
                                   .arg(QDateTime::currentDateTime().toString())
                                   .arg(QRandomGenerator::global()->bounded(100000));
    return QCryptographicHash::hash(data.toUtf8(), QCryptographicHash::Sha256).toHex();
}

void AuthManager::setCurrentUser(std::shared_ptr<User> user) {
    currentUser = user;
    isAuthenticated = true;
    sessionStartTime = QDateTime::currentDateTime();
    sessionToken = generateSessionToken();
    securityManager->startSession();
}

void AuthManager::clearSession() {
    currentUser = nullptr;
    isAuthenticated = false;
    sessionToken.clear();
    sessionStartTime = QDateTime();
    securityManager->endSession();
}

bool AuthManager::checkAccountLockStatus(const QString& username) {
    auto user = databaseManager->getUserByUsername(username);
    if (!user) {
        return false;
    }
    
    if (user->isAccountTemporarilyLocked()) {
        return true;
    }
    
    return false;
}

void AuthManager::handleFailedLogin(const QString& username, const QString& reason) {
    auto user = databaseManager->getUserByUsername(username);
    if (user) {
        user->incrementFailedLoginAttempts();
        
        if (user->getFailedLoginAttempts() >= 3) {
            user->lockAccount(5); // Lock for 5 minutes
            databaseManager->logActivity(user->getUserId(), "ACCOUNT_LOCKED", 
                                         "Account locked due to failed login attempts", "WARNING");
        }
        
        databaseManager->updateUser(*user);
    }
    
    logLoginAttempt(username, false);
}

void AuthManager::handleSuccessfulLogin(const QString& username) {
    auto user = databaseManager->getUserByUsername(username);
    if (user) {
        user->resetFailedLoginAttempts();
        user->unlockAccount();
        databaseManager->updateUser(*user);
    }
    
    logLoginAttempt(username, true);
}

bool AuthManager::validateUserCredentials(const QString& username, const QString& password) {
    auto user = databaseManager->getUserByUsername(username);
    if (!user) {
        return false;
    }
    
    QString hashedPassword = User::hashPassword(password);
    return hashedPassword == user->getPasswordHash();
}

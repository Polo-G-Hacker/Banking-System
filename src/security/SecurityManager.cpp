#include "security/SecurityManager.h"
#include <QRegularExpression>
#include <QDebug>
#include <QCryptographicHash>

SecurityManager* SecurityManager::instance = nullptr;

SecurityManager::SecurityManager() {
    sessionTimer = new QTimer();
    QObject::connect(sessionTimer, &QTimer::timeout, [this]() { onSessionTimeout(); });
    sessionTimeoutMinutes = 5;
    sessionActive = false;
    securityLevel = "MEDIUM";
}

SecurityManager* SecurityManager::getInstance() {
    if (!instance) {
        instance = new SecurityManager();
    }
    return instance;
}

SecurityManager::~SecurityManager() {
    if (sessionTimer) {
        sessionTimer->stop();
        delete sessionTimer;
    }
}

void SecurityManager::startSession() {
    sessionActive = true;
    lastActivity = QDateTime::currentDateTime();
    sessionTimer->start(sessionTimeoutMinutes * 60 * 1000); // Convert to milliseconds
}

void SecurityManager::endSession() {
    sessionActive = false;
    sessionTimer->stop();
}

void SecurityManager::resetActivityTimer() {
    lastActivity = QDateTime::currentDateTime();
    if (sessionActive) {
        sessionTimer->stop();
        sessionTimer->start(sessionTimeoutMinutes * 60 * 1000);
    }
}

bool SecurityManager::isSessionExpired() const {
    if (!sessionActive) {
        return true;
    }
    
    QDateTime now = QDateTime::currentDateTime();
    return lastActivity.secsTo(now) > (sessionTimeoutMinutes * 60);
}

QString SecurityManager::generateMathCaptcha() {
    int num1 = QRandomGenerator::global()->bounded(1, 20);
    int num2 = QRandomGenerator::global()->bounded(1, 20);
    QString operation = QRandomGenerator::global()->bounded(2) == 0 ? "+" : "-";
    
    if (operation == "-" && num1 < num2) {
        qSwap(num1, num2);
    }
    
    return QString("%1 %2 %3 = ?").arg(num1).arg(operation).arg(num2);
}

QString SecurityManager::generateAlphanumericCaptcha(int length) {
    return generateRandomString(length, true, true);
}

bool SecurityManager::validateMathCaptcha(const QString& question, const QString& answer) {
    QRegularExpression regex(R"((\d+)\s*([+-])\s*(\d+)\s*=)");
    QRegularExpressionMatch match = regex.match(question);
    
    if (!match.hasMatch()) {
        return false;
    }
    
    int num1 = match.captured(1).toInt();
    QString operation = match.captured(2);
    int num2 = match.captured(3).toInt();
    
    int correctAnswer;
    if (operation == "+") {
        correctAnswer = num1 + num2;
    } else {
        correctAnswer = num1 - num2;
    }
    
    bool ok;
    int userAnswer = answer.toInt(&ok);
    return ok && (userAnswer == correctAnswer);
}

bool SecurityManager::validateAlphanumericCaptcha(const QString& captcha, const QString& userInput) {
    return captcha.toUpper() == userInput.toUpper();
}

QString SecurityManager::generateCaptcha(bool isAdmin) {
    return isAdmin ? generateAlphanumericCaptcha() : generateMathCaptcha();
}

bool SecurityManager::validateCaptcha(const QString& question, const QString& answer, bool isAdmin) {
    return isAdmin ? validateAlphanumericCaptcha(question, answer)
                   : validateMathCaptcha(question, answer);
}

SecurityManager::PasswordStrength SecurityManager::checkPasswordStrength(const QString& password) {
    int score = calculatePasswordScore(password);
    
    if (score < 3) {
        return WEAK;
    } else if (score < 6) {
        return MEDIUM;
    } else {
        return STRONG;
    }
}

QString SecurityManager::getPasswordStrengthDescription(PasswordStrength strength) {
    switch (strength) {
        case WEAK: return "Weak";
        case MEDIUM: return "Medium";
        case STRONG: return "Strong";
        default: return "Unknown";
    }
}

QString SecurityManager::getPasswordStrengthColor(PasswordStrength strength) {
    switch (strength) {
        case WEAK: return "red";
        case MEDIUM: return "orange";
        case STRONG: return "green";
        default: return "gray";
    }
}

bool SecurityManager::validateEmail(const QString& email) {
    QRegularExpression emailRegex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    return emailRegex.match(email).hasMatch();
}

bool SecurityManager::validatePhoneNumber(const QString& phone) {
    QRegularExpression phoneRegex(R"(^[\+]?[1-9][\d]{0,15}$)");
    return phoneRegex.match(phone).hasMatch();
}

bool SecurityManager::validateAmount(double amount) {
    return amount > 0 && amount <= 1000000; // Maximum transaction limit
}

bool SecurityManager::validateAccountNumber(const QString& accountNumber) {
    QRegularExpression accountRegex(R"(^ACC\d{6,}$)");
    return accountRegex.match(accountNumber).hasMatch();
}

bool SecurityManager::validateTransactionPin(const QString& pin) {
    QRegularExpression pinRegex(R"(^\d{4}$)");
    return pinRegex.match(pin).hasMatch();
}

bool SecurityManager::validateUsername(const QString& username) {
    QRegularExpression usernameRegex(R"(^[a-zA-Z0-9_]{3,20}$)");
    return usernameRegex.match(username).hasMatch();
}

bool SecurityManager::detectSuspiciousActivity(int userId, const QString& activity, const QString& details) {
    SuspiciousActivity suspicious;
    suspicious.type = activity;
    suspicious.description = details;
    suspicious.timestamp = QDateTime::currentDateTime();
    suspicious.userId = userId;
    
    // Determine severity based on activity type
    if (activity == "MULTIPLE_FAILED_LOGINS") {
        suspicious.severity = "HIGH";
    } else if (activity == "EXCESSIVE_TRANSFERS") {
        suspicious.severity = "MEDIUM";
    } else if (activity == "LARGE_TRANSACTION") {
        suspicious.severity = "HIGH";
    } else {
        suspicious.severity = "LOW";
    }
    
    suspiciousActivities.append(suspicious);
    
    // Keep only last 100 suspicious activities
    if (suspiciousActivities.size() > 100) {
        suspiciousActivities.removeFirst();
    }
    
    return suspicious.severity == "HIGH";
}

QList<SecurityManager::SuspiciousActivity> SecurityManager::getRecentSuspiciousActivities(int userId, int hours) {
    QList<SuspiciousActivity> recent;
    QDateTime cutoff = QDateTime::currentDateTime().addSecs(-hours * 3600);
    
    for (const auto& activity : suspiciousActivities) {
        if (activity.userId == userId && activity.timestamp >= cutoff) {
            recent.append(activity);
        }
    }
    
    return recent;
}

void SecurityManager::clearSuspiciousActivities(int userId) {
    auto it = suspiciousActivities.begin();
    while (it != suspiciousActivities.end()) {
        if (it->userId == userId) {
            it = suspiciousActivities.erase(it);
        } else {
            ++it;
        }
    }
}

bool SecurityManager::isRateLimited(const QString& identifier, int maxAttempts, int timeWindowMinutes) {
    QDateTime now = QDateTime::currentDateTime();
    
    if (!rateLimits.contains(identifier)) {
        rateLimits[identifier] = {maxAttempts, timeWindowMinutes, 0, now};
        return false;
    }
    
    RateLimit& limit = rateLimits[identifier];
    
    // Reset window if expired
    if (limit.windowStart.secsTo(now) > (limit.timeWindowMinutes * 60)) {
        limit.currentAttempts = 0;
        limit.windowStart = now;
    }
    
    return limit.currentAttempts >= limit.maxAttempts;
}

void SecurityManager::recordAttempt(const QString& identifier) {
    if (rateLimits.contains(identifier)) {
        rateLimits[identifier].currentAttempts++;
    }
}

void SecurityManager::resetRateLimit(const QString& identifier) {
    rateLimits.remove(identifier);
}

QString SecurityManager::encryptSensitiveData(const QString& data, int shift) {
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

QString SecurityManager::decryptSensitiveData(const QString& encryptedData, int shift) {
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

void SecurityManager::setSecurityLevel(const QString& level) {
    securityLevel = level.toUpper();
    
    if (securityLevel == "HIGH") {
        sessionTimeoutMinutes = 3;
    } else if (securityLevel == "LOW") {
        sessionTimeoutMinutes = 10;
    } else {
        sessionTimeoutMinutes = 5;
    }
}

QString SecurityManager::getSecurityLevel() const {
    return securityLevel;
}

bool SecurityManager::isHighSecurityMode() const {
    return securityLevel == "HIGH";
}

void SecurityManager::onSessionTimeout() {
    sessionActive = false;
    sessionTimer->stop();
}

QString SecurityManager::generateRandomString(int length, bool includeNumbers, bool includeLetters) {
    QString chars;
    if (includeLetters) {
        chars += "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    }
    if (includeNumbers) {
        chars += "0123456789";
    }
    
    QString result;
    for (int i = 0; i < length; ++i) {
        int index = QRandomGenerator::global()->bounded(chars.length());
        result += chars[index];
    }
    return result;
}

int SecurityManager::calculatePasswordScore(const QString& password) {
    int score = 0;
    
    // Length bonus
    if (password.length() >= 8) score++;
    if (password.length() >= 12) score++;
    
    // Character variety
    bool hasUpper = false, hasLower = false, hasNumber = false, hasSpecial = false;
    for (const QChar& ch : password) {
        if (ch.isUpper()) hasUpper = true;
        else if (ch.isLower()) hasLower = true;
        else if (ch.isNumber()) hasNumber = true;
        else if (!ch.isLetterOrNumber()) hasSpecial = true;
    }
    
    if (hasUpper) score++;
    if (hasLower) score++;
    if (hasNumber) score++;
    if (hasSpecial) score++;
    
    // Complexity bonus
    if (password.length() >= 16 && hasUpper && hasLower && hasNumber && hasSpecial) {
        score++;
    }
    
    return score;
}

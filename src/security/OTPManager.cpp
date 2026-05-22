#include "security/OTPManager.h"
#include "security/DatabaseManager.h"
#include "core/User.h"
#include <QRandomGenerator>
#include <QDateTime>
#include <QDebug>
#include <QCryptographicHash>
#include <QRegularExpression>

OTPManager* OTPManager::instance = nullptr;

OTPManager::OTPManager() {
    databaseManager = DatabaseManager::getInstance();
    cleanupTimer = new QTimer();
    QObject::connect(cleanupTimer, &QTimer::timeout, [this]() { onCleanupTimer(); });
    cleanupTimer->start(5 * 60 * 1000); // Run every 5 minutes
    otpValidityMinutes = 10;
    maxOtpAttempts = 3;
}

OTPManager* OTPManager::getInstance() {
    if (!instance) {
        instance = new OTPManager();
    }
    return instance;
}

OTPManager::~OTPManager() {
    if (cleanupTimer) {
        cleanupTimer->stop();
        delete cleanupTimer;
    }
}

OTPManager::OTPResult OTPManager::generateOTP(int userId, OTPPurpose purpose) {
    OTPResult result;
    result.success = false;
    
    // Check rate limiting
    if (isRateLimited(userId)) {
        result.message = "Too many OTP requests. Please wait before requesting another.";
        return result;
    }
    
    // Generate secure OTP
    QString otpCode = generateSecureOTP();
    QString purposeStr = purposeToString(purpose);
    
    // Store OTP in database
    bool stored = databaseManager->generateOTP(userId, purposeStr, otpCode);
    
    if (stored) {
        // Get user details for sending OTP
        auto user = databaseManager->getUserById(userId);
        if (user) {
            // Send OTP via email (and SMS if phone number is available)
            bool emailSent = sendOTPViaEmail(user->getEmail(), otpCode, purpose);
            bool smsSent = false;
            
            if (!user->getPhoneNumber().isEmpty()) {
                smsSent = sendOTPViaSMS(user->getPhoneNumber(), otpCode, purpose);
            }
            
            if (emailSent || smsSent) {
                result.success = true;
                result.message = "OTP sent successfully.";
                result.otpCode = otpCode; // For development only
                
                // Record attempt
                recordOTPAttempt(userId);
                lastOTPRequest[userId] = QDateTime::currentDateTime();
            } else {
                result.message = "Failed to send OTP. Please try again.";
            }
        } else {
            result.message = "User not found.";
        }
    } else {
        result.message = "Failed to generate OTP. Please try again.";
    }
    
    return result;
}

bool OTPManager::validateOTP(int userId, const QString& otpCode, OTPPurpose purpose) {
    if (!isValidOTPFormat(otpCode)) {
        return false;
    }
    
    QString purposeStr = purposeToString(purpose);
    bool isValid = databaseManager->validateOTP(userId, otpCode, purposeStr);
    
    if (isValid) {
        // Reset attempt count on successful validation
        otpAttemptCounts.remove(userId);
    } else {
        // Increment failed attempts
        otpAttemptCounts[userId] = otpAttemptCounts.value(userId, 0) + 1;
    }
    
    return isValid;
}

bool OTPManager::invalidateOTP(int userId, const QString& otpCode, OTPPurpose purpose) {
    // This would mark the OTP as used in the database
    // For now, we'll use validateOTP which automatically marks as used
    return validateOTP(userId, otpCode, purpose);
}

void OTPManager::cleanupExpiredOTPs() {
    databaseManager->cleanupExpiredOTPs();
    
    // Clean up rate limiting data older than 1 hour
    QDateTime cutoff = QDateTime::currentDateTime().addSecs(-3600);
    auto it = lastOTPRequest.begin();
    while (it != lastOTPRequest.end()) {
        if (it.value() < cutoff) {
            it = lastOTPRequest.erase(it);
        } else {
            ++it;
        }
    }
}

int OTPManager::getValidOTPCount(int userId) {
    // This would query the database for valid OTPs
    // For now, return a placeholder
    return 0;
}

QList<QString> OTPManager::getActiveOTPPurposes(int userId) {
    QList<QString> purposes;
    // This would query the database for active OTP purposes
    return purposes;
}

bool OTPManager::hasPendingOTP(int userId, OTPPurpose purpose) {
    QString purposeStr = purposeToString(purpose);
    // This would check if there's a valid OTP for this purpose
    return false;
}

bool OTPManager::sendOTPViaEmail(const QString& email, const QString& otpCode, OTPPurpose purpose) {
    // In a real implementation, this would use SMTP or an email service
    // For now, we'll simulate by logging
    QString subject = generateEmailSubject(purpose);
    QString body = generateEmailBody(otpCode, purpose);
    
    qDebug() << "Email to:" << email;
    qDebug() << "Subject:" << subject;
    qDebug() << "Body:" << body;
    
    // Simulate successful email send
    return true;
}

bool OTPManager::sendOTPViaSMS(const QString& phoneNumber, const QString& otpCode, OTPPurpose purpose) {
    // In a real implementation, this would use an SMS gateway
    // For now, we'll simulate by logging
    QString message = generateSMSMessage(otpCode, purpose);
    
    qDebug() << "SMS to:" << phoneNumber;
    qDebug() << "Message:" << message;
    
    // Simulate successful SMS send
    return true;
}

bool OTPManager::isRateLimited(int userId) {
    // Check if user has exceeded maximum attempts
    int attempts = otpAttemptCounts.value(userId, 0);
    if (attempts >= maxOtpAttempts) {
        return true;
    }
    
    // Check time-based rate limiting (max 3 OTPs per hour)
    QDateTime lastRequest = lastOTPRequest.value(userId);
    if (lastRequest.isValid()) {
        int minutesSinceLastRequest = lastRequest.secsTo(QDateTime::currentDateTime()) / 60;
        if (minutesSinceLastRequest < 20) { // Minimum 20 minutes between OTP requests
            return true;
        }
    }
    
    return false;
}

void OTPManager::recordOTPAttempt(int userId) {
    otpAttemptCounts[userId] = otpAttemptCounts.value(userId, 0) + 1;
}

int OTPManager::getRemainingAttempts(int userId) {
    int attempts = otpAttemptCounts.value(userId, 0);
    return qMax(0, maxOtpAttempts - attempts);
}

int OTPManager::getTotalOTPsGenerated(int userId, int hours) {
    // This would query the database for OTP generation statistics
    return 0;
}

int OTPManager::getTotalOTPsValidated(int userId, int hours) {
    // This would query the database for OTP validation statistics
    return 0;
}

int OTPManager::getTotalFailedOTPAttempts(int userId, int hours) {
    // This would query the database for failed OTP attempts
    return 0;
}

void OTPManager::onCleanupTimer() {
    cleanupExpiredOTPs();
}

QString OTPManager::purposeToString(OTPPurpose purpose) {
    switch (purpose) {
        case LOGIN: return "LOGIN";
        case TRANSACTION: return "TRANSACTION";
        case PASSWORD_RESET: return "PASSWORD_RESET";
        case ADMIN_LOGIN: return "ADMIN_LOGIN";
        default: return "UNKNOWN";
    }
}

QString OTPManager::generateOTPPurposeMessage(OTPPurpose purpose) {
    switch (purpose) {
        case LOGIN: return "Login Verification";
        case TRANSACTION: return "Transaction Authorization";
        case PASSWORD_RESET: return "Password Reset";
        case ADMIN_LOGIN: return "Admin Login Verification";
        default: return "Verification";
    }
}

QString OTPManager::generateEmailSubject(OTPPurpose purpose) {
    QString baseMessage = generateOTPPurposeMessage(purpose);
    return QString("Banking System - %1 Code").arg(baseMessage);
}

QString OTPManager::generateEmailBody(const QString& otpCode, OTPPurpose purpose) {
    QString message = generateOTPPurposeMessage(purpose);
    
    return QString(
        "Dear Customer,\n\n"
        "Your %1 code is: %2\n\n"
        "This code will expire in %3 minutes.\n"
        "Please do not share this code with anyone.\n\n"
        "If you did not request this code, please contact our support team immediately.\n\n"
        "Best regards,\n"
        "Banking System Security Team"
    ).arg(message).arg(otpCode).arg(otpValidityMinutes);
}

QString OTPManager::generateSMSMessage(const QString& otpCode, OTPPurpose purpose) {
    QString message = generateOTPPurposeMessage(purpose);
    
    return QString(
        "Your Banking System %1 code is: %2\n"
        "Valid for %3 minutes. Do not share this code."
    ).arg(message).arg(otpCode).arg(otpValidityMinutes);
}

QString OTPManager::generateSecureOTP() {
    // Generate cryptographically secure 6-digit OTP
    return QString("%1").arg(QRandomGenerator::global()->bounded(100000, 999999));
}

bool OTPManager::isValidOTPFormat(const QString& otpCode) {
    // OTP should be exactly 6 digits
    QRegularExpression otpRegex(R"(^\d{6}$)");
    return otpRegex.match(otpCode).hasMatch();
}

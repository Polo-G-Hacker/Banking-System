#ifndef OTPMANAGER_H
#define OTPMANAGER_H

#include <QString>
#include <QDateTime>
#include <QTimer>
#include <memory>

class DatabaseManager;

class OTPManager {
private:
    static OTPManager* instance;
    DatabaseManager* databaseManager;
    QTimer* cleanupTimer;
    int otpValidityMinutes;
    int maxOtpAttempts;
    
    OTPManager();

public:
    static OTPManager* getInstance();
    ~OTPManager();
    
    // OTP generation and validation
    enum OTPPurpose {
        LOGIN,
        TRANSACTION,
        PASSWORD_RESET,
        ADMIN_LOGIN
    };
    
    struct OTPResult {
        bool success;
        QString message;
        QString otpCode; // For development/testing only
    };
    
    OTPResult generateOTP(int userId, OTPPurpose purpose);
    bool validateOTP(int userId, const QString& otpCode, OTPPurpose purpose);
    bool invalidateOTP(int userId, const QString& otpCode, OTPPurpose purpose);
    
    // OTP management
    void cleanupExpiredOTPs();
    int getValidOTPCount(int userId);
    QList<QString> getActiveOTPPurposes(int userId);
    bool hasPendingOTP(int userId, OTPPurpose purpose);
    
    // Email/SMS simulation (for development)
    bool sendOTPViaEmail(const QString& email, const QString& otpCode, OTPPurpose purpose);
    bool sendOTPViaSMS(const QString& phoneNumber, const QString& otpCode, OTPPurpose purpose);
    
    // OTP security
    bool isRateLimited(int userId);
    void recordOTPAttempt(int userId);
    int getRemainingAttempts(int userId);
    
    // Configuration
    void setOTPValidityMinutes(int minutes) { otpValidityMinutes = minutes; }
    int getOTPValidityMinutes() const { return otpValidityMinutes; }
    void setMaxOTPAttempts(int attempts) { maxOtpAttempts = attempts; }
    int getMaxOTPAttempts() const { return maxOtpAttempts; }
    
    // Statistics
    int getTotalOTPsGenerated(int userId, int hours = 24);
    int getTotalOTPsValidated(int userId, int hours = 24);
    int getTotalFailedOTPAttempts(int userId, int hours = 24);

private slots:
    void onCleanupTimer();

private:
    QString purposeToString(OTPPurpose purpose);
    QString generateOTPPurposeMessage(OTPPurpose purpose);
    QString generateEmailSubject(OTPPurpose purpose);
    QString generateEmailBody(const QString& otpCode, OTPPurpose purpose);
    QString generateSMSMessage(const QString& otpCode, OTPPurpose purpose);
    QString generateSecureOTP();
    bool isValidOTPFormat(const QString& otpCode);
    
    QMap<int, int> otpAttemptCounts; // userId -> attempt count
    QMap<int, QDateTime> lastOTPRequest; // userId -> last request time
};

#endif // OTPMANAGER_H

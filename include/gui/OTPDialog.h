#ifndef OTPDIALOG_H
#define OTPDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QTimer>
#include <QProgressBar>

class AuthManager;
class SecurityManager;

class OTPDialog : public QDialog
{
    Q_OBJECT

private:
    // UI Components
    QFrame* mainFrame;
    QLabel* titleLabel;
    QLabel* subtitleLabel;
    QLabel* instructionLabel;
    QLabel* otpLabel;
    QLineEdit* otpEdit;
    QPushButton* verifyButton;
    QPushButton* resendButton;
    QPushButton* cancelButton;
    QProgressBar* progressBar;
    QLabel* timerLabel;
    QLabel* statusLabel;
    
    // Layouts
    QVBoxLayout* mainLayout;
    QVBoxLayout* frameLayout;
    QGridLayout* formLayout;
    QHBoxLayout* buttonLayout;
    QHBoxLayout* timerLayout;
    
    // Services
    AuthManager* authManager;
    SecurityManager* securityManager;
    
    // State variables
    int remainingSeconds;
    QTimer* countdownTimer;
    bool isResendEnabled;
    int resendAttempts;
    
    // UI setup
    void setupUI();
    void setupStyles();
    void applyModernStyling();
    
    // OTP management
    void startCountdown();
    void stopCountdown();
    void updateTimerDisplay();
    void enableResendButton();
    void disableResendButton();
    
    // Validation
    bool validateOTPInput();
    void verifyOTP();
    void resendOTP();
    
    // UI helpers
    void showStatusMessage(const QString& message, bool isError = false);
    void setVerifyEnabled(bool enabled);
    void clearForm();

protected:
    void closeEvent(QCloseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

public:
    explicit OTPDialog(QWidget *parent = nullptr);
    ~OTPDialog();

private slots:
    void onVerifyClicked();
    void onResendClicked();
    void onCountdownTimerTimeout();
    void onOTPTextChanged();

signals:
    void otpVerified();
    void otpVerificationFailed(const QString& message);
};

#endif // OTPDIALOG_H

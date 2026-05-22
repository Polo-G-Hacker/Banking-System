#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QGroupBox>
#include <QCheckBox>
#include <QProgressBar>
#include <QTimer>
#include <QStackedWidget>
#include <memory>

class AuthManager;

class LoginWindow : public QMainWindow {
    Q_OBJECT

private:
    // UI Components
    QWidget* centralWidget;
    QStackedWidget* stackedWidget;
    
    // Login page widgets
    QWidget* loginPage;
    QFrame* loginFrame;
    QLabel* titleLabel;
    QLabel* subtitleLabel;
    QLabel* usernameLabel;
    QLineEdit* usernameEdit;
    QLabel* passwordLabel;
    QLineEdit* passwordEdit;
    QLabel* captchaLabel;
    QLineEdit* captchaEdit;
    QLabel* captchaQuestionLabel;
    QPushButton* refreshCaptchaButton;
    QPushButton* loginButton;
    QPushButton* forgotPasswordButton;
    QCheckBox* rememberMeCheckbox;
    QLabel* statusLabel;
    QProgressBar* loginProgressBar;
    
    // Layout widgets
    QVBoxLayout* mainLayout;
    QVBoxLayout* loginLayout;
    QGridLayout* formLayout;
    QHBoxLayout* buttonLayout;
    QHBoxLayout* captchaLayout;
    QVBoxLayout* statusLayout;
    
    // Services
    AuthManager* authManager;
    
    // State variables
    QString currentCaptchaQuestion;
    bool isAdminLogin;
    int failedAttempts;
    QTimer* loginTimer;
    
    // Styling
    void setupUI();
    void setupLoginPage();
    void setupStyles();
    void applyModernStyling();
    
    // CAPTCHA management
    void generateCaptcha();
    void refreshCaptcha();
    bool validateCaptcha();
    
    // Authentication
    void performLogin();
    void handleLoginResult(bool success, const QString& message, bool requiresOTP, 
                           bool requiresCaptcha, const QString& captchaQuestion);
    void showOTPDialog();
    void resetLoginForm();
    
    // UI helpers
    void showStatusMessage(const QString& message, bool isError = false);
    void setLoginEnabled(bool enabled);
    void showLoadingState(bool show);
    void clearForm();

protected:
    void closeEvent(QCloseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();

private slots:
    void onLoginClicked();
    void onRefreshCaptchaClicked();
    void onForgotPasswordClicked();
    void onUsernameTextChanged();
    void onPasswordTextChanged();
    void onCaptchaTextChanged();
    void onLoginTimerTimeout();

signals:
    void loginSuccessful(int userId, const QString& username, const QString& role);
    void loginFailed(const QString& message);
};

#endif // LOGINWINDOW_H

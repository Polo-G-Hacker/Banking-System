#include "gui/LoginWindow.h"
#include "security/AuthManager.h"
#include <QApplication>
#include <QMessageBox>
#include <QKeyEvent>
#include <QFont>
#include <QGraphicsDropShadowEffect>
#include <QRandomGenerator>
#include <QScreen>

LoginWindow::LoginWindow(QWidget *parent)
    : QMainWindow(parent)
    , centralWidget(nullptr)
    , stackedWidget(nullptr)
    , loginPage(nullptr)
    , loginFrame(nullptr)
    , titleLabel(nullptr)
    , subtitleLabel(nullptr)
    , usernameLabel(nullptr)
    , usernameEdit(nullptr)
    , passwordLabel(nullptr)
    , passwordEdit(nullptr)
    , captchaLabel(nullptr)
    , captchaEdit(nullptr)
    , captchaQuestionLabel(nullptr)
    , refreshCaptchaButton(nullptr)
    , loginButton(nullptr)
    , forgotPasswordButton(nullptr)
    , rememberMeCheckbox(nullptr)
    , statusLabel(nullptr)
    , loginProgressBar(nullptr)
    , mainLayout(nullptr)
    , loginLayout(nullptr)
    , formLayout(nullptr)
    , buttonLayout(nullptr)
    , captchaLayout(nullptr)
    , statusLayout(nullptr)
    , authManager(nullptr)
    , isAdminLogin(false)
    , failedAttempts(0)
    , loginTimer(nullptr)
{
    // Initialize services
    authManager = AuthManager::getInstance();
    
    // Setup UI
    setupUI();
    setupStyles();
    
    // Initialize timer
    loginTimer = new QTimer(this);
    connect(loginTimer, &QTimer::timeout, this, &LoginWindow::onLoginTimerTimeout);
    
    // Generate initial CAPTCHA
    generateCaptcha();
    
    // Set window properties
    setWindowTitle("Banking Management System - Secure Login");
    setMinimumSize(800, 600);
    resize(800, 600);
    
    // Center window on screen
    if (QScreen* screen = QApplication::primaryScreen()) {
        move(screen->availableGeometry().center() - rect().center());
    }
}

LoginWindow::~LoginWindow()
{
    if (loginTimer) {
        loginTimer->stop();
    }
}

void LoginWindow::setupUI()
{
    // Create central widget and stacked widget
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    stackedWidget = new QStackedWidget(centralWidget);
    
    // Setup login page
    setupLoginPage();
    
    // Add pages to stacked widget
    stackedWidget->addWidget(loginPage);
    stackedWidget->setCurrentWidget(loginPage);
    
    // Main layout
    mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(stackedWidget);
}

void LoginWindow::setupLoginPage()
{
    loginPage = new QWidget();
    
    // Create main login frame with modern styling
    loginFrame = new QFrame();
    loginFrame->setObjectName("loginFrame");
    loginFrame->setFrameStyle(QFrame::NoFrame);
    loginFrame->setMinimumWidth(400);
    loginFrame->setMaximumWidth(500);
    
    // Title and subtitle
    titleLabel = new QLabel("Banking Management System");
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    
    subtitleLabel = new QLabel("Secure Banking Portal");
    subtitleLabel->setObjectName("subtitleLabel");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    
    // Form fields
    usernameLabel = new QLabel("Username:");
    usernameLabel->setObjectName("formLabel");
    
    usernameEdit = new QLineEdit();
    usernameEdit->setObjectName("usernameEdit");
    usernameEdit->setPlaceholderText("Enter your username");
    usernameEdit->setMaxLength(50);
    
    passwordLabel = new QLabel("Password:");
    passwordLabel->setObjectName("formLabel");
    
    passwordEdit = new QLineEdit();
    passwordEdit->setObjectName("passwordEdit");
    passwordEdit->setPlaceholderText("Enter your password");
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setMaxLength(100);
    
    // CAPTCHA section
    captchaLabel = new QLabel("Security Verification:");
    captchaLabel->setObjectName("formLabel");
    
    captchaQuestionLabel = new QLabel();
    captchaQuestionLabel->setObjectName("captchaQuestion");
    captchaQuestionLabel->setAlignment(Qt::AlignCenter);
    captchaQuestionLabel->setMinimumHeight(40);
    
    captchaEdit = new QLineEdit();
    captchaEdit->setObjectName("captchaEdit");
    captchaEdit->setPlaceholderText("Enter the answer");
    captchaEdit->setMaxLength(10);
    
    refreshCaptchaButton = new QPushButton("🔄 Refresh");
    refreshCaptchaButton->setObjectName("refreshButton");
    refreshCaptchaButton->setMaximumWidth(100);
    
    // Remember me checkbox
    rememberMeCheckbox = new QCheckBox("Remember me");
    rememberMeCheckbox->setObjectName("rememberCheckbox");
    
    // Buttons
    loginButton = new QPushButton("Login");
    loginButton->setObjectName("loginButton");
    loginButton->setDefault(true);
    
    forgotPasswordButton = new QPushButton("Forgot Password?");
    forgotPasswordButton->setObjectName("forgotButton");
    
    // Status label and progress bar
    statusLabel = new QLabel();
    statusLabel->setObjectName("statusLabel");
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setWordWrap(true);
    statusLabel->hide();
    
    loginProgressBar = new QProgressBar();
    loginProgressBar->setObjectName("progressBar");
    loginProgressBar->setRange(0, 0); // Indeterminate progress
    loginProgressBar->hide();
    
    // Setup layouts
    loginLayout = new QVBoxLayout(loginFrame);
    loginLayout->setSpacing(20);
    loginLayout->setContentsMargins(40, 40, 40, 40);
    
    // Add widgets to login layout
    loginLayout->addWidget(titleLabel);
    loginLayout->addWidget(subtitleLabel);
    loginLayout->addSpacing(20);
    
    // Form layout
    formLayout = new QGridLayout();
    formLayout->setVerticalSpacing(15);
    
    formLayout->addWidget(usernameLabel, 0, 0);
    formLayout->addWidget(usernameEdit, 0, 1);
    
    formLayout->addWidget(passwordLabel, 1, 0);
    formLayout->addWidget(passwordEdit, 1, 1);
    
    loginLayout->addLayout(formLayout);
    
    // CAPTCHA layout
    captchaLayout = new QHBoxLayout();
    captchaLayout->addWidget(captchaQuestionLabel, 1);
    captchaLayout->addWidget(refreshCaptchaButton);
    
    loginLayout->addWidget(captchaLabel);
    loginLayout->addLayout(captchaLayout);
    loginLayout->addWidget(captchaEdit);
    
    // Checkbox
    loginLayout->addWidget(rememberMeCheckbox);
    
    // Button layout
    buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(forgotPasswordButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(loginButton);
    
    loginLayout->addLayout(buttonLayout);
    
    // Status layout
    statusLayout = new QVBoxLayout();
    statusLayout->addWidget(loginProgressBar);
    statusLayout->addWidget(statusLabel);
    
    loginLayout->addLayout(statusLayout);
    loginLayout->addStretch();
    
    // Main page layout
    QVBoxLayout* pageLayout = new QVBoxLayout(loginPage);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    
    // Add spacer for centering
    pageLayout->addStretch();
    
    // Center the login frame
    QHBoxLayout* centerLayout = new QHBoxLayout();
    centerLayout->addStretch();
    centerLayout->addWidget(loginFrame);
    centerLayout->addStretch();
    
    pageLayout->addLayout(centerLayout);
    pageLayout->addStretch();
    
    // Connect signals
    connect(loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
    connect(refreshCaptchaButton, &QPushButton::clicked, this, &LoginWindow::onRefreshCaptchaClicked);
    connect(forgotPasswordButton, &QPushButton::clicked, this, &LoginWindow::onForgotPasswordClicked);
    connect(usernameEdit, &QLineEdit::textChanged, this, &LoginWindow::onUsernameTextChanged);
    connect(passwordEdit, &QLineEdit::textChanged, this, &LoginWindow::onPasswordTextChanged);
    connect(captchaEdit, &QLineEdit::textChanged, this, &LoginWindow::onCaptchaTextChanged);
    
    // Enable login button only when fields are filled
    setLoginEnabled(false);
}

void LoginWindow::setupStyles()
{
    QString styleSheet = R"(
        /* Main window background */
        LoginWindow {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #1a237e, stop:1 #283593);
        }
        
        /* Login frame */
        QFrame#loginFrame {
            background: rgba(255, 255, 255, 0.95);
            border-radius: 15px;
            border: 1px solid rgba(255, 255, 255, 0.3);
        }
        
        /* Title labels */
        QLabel#titleLabel {
            font-size: 24px;
            font-weight: bold;
            color: #1a237e;
            margin-bottom: 10px;
        }
        
        QLabel#subtitleLabel {
            font-size: 14px;
            color: #5c6bc0;
            margin-bottom: 20px;
        }
        
        /* Form labels */
        QLabel#formLabel {
            font-size: 12px;
            font-weight: 600;
            color: #37474f;
        }
        
        /* Line edits */
        QLineEdit {
            background: white;
            border: 2px solid #e0e0e0;
            border-radius: 8px;
            padding: 12px;
            font-size: 14px;
            color: #263238;
        }
        
        QLineEdit:focus {
            border-color: #1a237e;
            outline: none;
        }
        
        /* CAPTCHA question */
        QLabel#captchaQuestion {
            background: #f5f5f5;
            border: 2px solid #e0e0e0;
            border-radius: 8px;
            padding: 10px;
            font-size: 16px;
            font-weight: bold;
            color: #1a237e;
        }
        
        /* Buttons */
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #1a237e, stop:1 #283593);
            color: white;
            border: none;
            border-radius: 8px;
            padding: 12px 24px;
            font-size: 14px;
            font-weight: 600;
        }
        
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #283593, stop:1 #3949ab);
        }
        
        QPushButton:pressed {
            background: #1a237e;
        }
        
        QPushButton:disabled {
            background: #b0bec5;
            color: #78909c;
        }
        
        QPushButton#refreshButton {
            background: #78909c;
            padding: 8px 16px;
            font-size: 12px;
        }
        
        QPushButton#refreshButton:hover {
            background: #607d8b;
        }
        
        QPushButton#forgotButton {
            background: transparent;
            color: #1a237e;
            border: none;
            text-decoration: underline;
            padding: 8px;
        }
        
        QPushButton#forgotButton:hover {
            color: #283593;
        }
        
        /* Checkbox */
        QCheckBox {
            color: #37474f;
            font-size: 12px;
        }
        
        QCheckBox::indicator {
            width: 16px;
            height: 16px;
            border: 2px solid #e0e0e0;
            border-radius: 4px;
            background: white;
        }
        
        QCheckBox::indicator:checked {
            background: #1a237e;
            border-color: #1a237e;
        }
        
        /* Status label */
        QLabel#statusLabel {
            color: #d32f2f;
            font-size: 12px;
            padding: 8px;
            border-radius: 6px;
            background: rgba(211, 47, 47, 0.1);
        }
        
        /* Progress bar */
        QProgressBar {
            border: none;
            background: transparent;
            height: 3px;
        }
        
        QProgressBar::chunk {
            background: #1a237e;
            border-radius: 3px;
        }
    )";
    
    setStyleSheet(styleSheet);
    applyModernStyling();
}

void LoginWindow::applyModernStyling()
{
    // Add drop shadow effect to login frame
    if (loginFrame) {
        QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
        shadow->setBlurRadius(20);
        shadow->setColor(QColor(0, 0, 0, 30));
        shadow->setOffset(0, 5);
        loginFrame->setGraphicsEffect(shadow);
    }
}

void LoginWindow::generateCaptcha()
{
    // Simple mathematical CAPTCHA for now
    int num1 = QRandomGenerator::global()->bounded(1, 10);
    int num2 = QRandomGenerator::global()->bounded(1, 10);
    int answer = num1 + num2;
    currentCaptchaQuestion = QString("%1 + %2 = ?").arg(num1).arg(num2);
    captchaQuestionLabel->setText(currentCaptchaQuestion);
    captchaEdit->clear();
}

void LoginWindow::refreshCaptcha()
{
    generateCaptcha();
    showStatusMessage("CAPTCHA refreshed", false);
}

bool LoginWindow::validateCaptcha()
{
    QString answer = captchaEdit->text().trimmed();
    // Simple validation for the mathematical CAPTCHA
    QStringList parts = currentCaptchaQuestion.split(" ");
    if (parts.size() >= 4) {
        int num1 = parts[0].toInt();
        int num2 = parts[2].toInt();
        int expectedAnswer = num1 + num2;
        return answer.toInt() == expectedAnswer;
    }
    return false;
}

void LoginWindow::performLogin()
{
    QString username = usernameEdit->text().trimmed();
    QString password = passwordEdit->text();
    QString captchaAnswer = captchaEdit->text().trimmed();
    
    // Validate input
    if (username.isEmpty() || password.isEmpty()) {
        showStatusMessage("Please enter username and password", true);
        return;
    }
    
    if (!validateCaptcha()) {
        showStatusMessage("Invalid security verification. Please try again.", true);
        refreshCaptcha();
        failedAttempts++;
        return;
    }
    
    showLoadingState(true);
    setLoginEnabled(false);
    
    // Perform authentication using AuthManager
    auto result = authManager->login(username, password);
    
    handleLoginResult(result.success, result.message, false, false, "");
}

void LoginWindow::handleLoginResult(bool success, const QString& message, bool requiresOTP, 
                                   bool requiresCaptcha, const QString& captchaQuestion)
{
    showLoadingState(false);
    
    if (success) {
        showStatusMessage("Login successful!", false);
        emit loginSuccessful(authManager->getCurrentUserId(), 
                           authManager->getCurrentUsername(), 
                           authManager->getCurrentUserRole());
        
        // Close login window after a short delay
        loginTimer->start(1000);
    } else {
        showStatusMessage(message, true);
        refreshCaptcha();
        failedAttempts++;
        setLoginEnabled(true);
        
        // Lock account after 3 failed attempts
        if (failedAttempts >= 3) {
            showStatusMessage("Too many failed attempts. Please try again later.", true);
            setLoginEnabled(false);
            loginTimer->start(300000); // 5 minutes
        }
    }
}

void LoginWindow::showOTPDialog()
{
    // OTP functionality temporarily disabled for basic version
    showStatusMessage("OTP verification not available in basic version", true);
    setLoginEnabled(true);
}

void LoginWindow::resetLoginForm()
{
    clearForm();
    failedAttempts = 0;
    setLoginEnabled(true);
    generateCaptcha();
}

void LoginWindow::showStatusMessage(const QString& message, bool isError)
{
    if (message.isEmpty()) {
        statusLabel->hide();
        return;
    }
    
    statusLabel->setText(message);
    statusLabel->show();
    
    if (isError) {
        statusLabel->setStyleSheet("QLabel#statusLabel { color: #d32f2f; background: rgba(211, 47, 47, 0.1); }");
    } else {
        statusLabel->setStyleSheet("QLabel#statusLabel { color: #388e3c; background: rgba(56, 142, 60, 0.1); }");
    }
    
    // Auto-hide success messages after 3 seconds
    if (!isError) {
        loginTimer->start(3000);
    }
}

void LoginWindow::setLoginEnabled(bool enabled)
{
    loginButton->setEnabled(enabled && !usernameEdit->text().isEmpty() && 
                           !passwordEdit->text().isEmpty() && !captchaEdit->text().isEmpty());
}

void LoginWindow::showLoadingState(bool show)
{
    if (show) {
        loginProgressBar->show();
        statusLabel->setText("Authenticating...");
        statusLabel->show();
        statusLabel->setStyleSheet("QLabel#statusLabel { color: #1976d2; background: rgba(25, 118, 210, 0.1); }");
    } else {
        loginProgressBar->hide();
    }
}

void LoginWindow::clearForm()
{
    usernameEdit->clear();
    passwordEdit->clear();
    captchaEdit->clear();
    statusLabel->hide();
}

void LoginWindow::closeEvent(QCloseEvent* event)
{
    // Clean up before closing
    if (loginTimer) {
        loginTimer->stop();
    }
    QMainWindow::closeEvent(event);
}

void LoginWindow::keyPressEvent(QKeyEvent* event)
{
    // Handle Enter key for login
    if (event->key() == Qt::Key_Return && loginButton->isEnabled()) {
        onLoginClicked();
    }
    QMainWindow::keyPressEvent(event);
}

// Slot implementations
void LoginWindow::onLoginClicked()
{
    performLogin();
}

void LoginWindow::onRefreshCaptchaClicked()
{
    refreshCaptcha();
}

void LoginWindow::onForgotPasswordClicked()
{
    QMessageBox::information(this, "Password Reset", 
                             "Please contact your administrator to reset your password.\n\n"
                             "For security reasons, password resets must be handled "
                             "through proper administrative channels.");
}

void LoginWindow::onUsernameTextChanged()
{
    setLoginEnabled(true);
}

void LoginWindow::onPasswordTextChanged()
{
    setLoginEnabled(true);
}

void LoginWindow::onCaptchaTextChanged()
{
    setLoginEnabled(true);
}

void LoginWindow::onLoginTimerTimeout()
{
    loginTimer->stop();
    
    if (failedAttempts >= 3) {
        // Reset after lockout period
        failedAttempts = 0;
        setLoginEnabled(true);
        showStatusMessage("You may now try logging in again.", false);
    } else if (statusLabel->text().contains("successful")) {
        // Close window after successful login
        close();
    } else {
        // Hide success message
        statusLabel->hide();
    }
}

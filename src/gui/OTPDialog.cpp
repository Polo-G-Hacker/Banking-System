#include "gui/OTPDialog.h"
#include "security/AuthManager.h"
#include "security/SecurityManager.h"
#include <QKeyEvent>
#include <QRegularExpression>
#include <QGraphicsDropShadowEffect>

OTPDialog::OTPDialog(QWidget *parent)
    : QDialog(parent)
    , mainFrame(nullptr)
    , titleLabel(nullptr)
    , subtitleLabel(nullptr)
    , instructionLabel(nullptr)
    , otpLabel(nullptr)
    , otpEdit(nullptr)
    , verifyButton(nullptr)
    , resendButton(nullptr)
    , cancelButton(nullptr)
    , progressBar(nullptr)
    , timerLabel(nullptr)
    , statusLabel(nullptr)
    , mainLayout(nullptr)
    , frameLayout(nullptr)
    , formLayout(nullptr)
    , buttonLayout(nullptr)
    , timerLayout(nullptr)
    , authManager(nullptr)
    , securityManager(nullptr)
    , remainingSeconds(600) // 10 minutes
    , countdownTimer(nullptr)
    , isResendEnabled(false)
    , resendAttempts(0)
{
    // Initialize services
    authManager = AuthManager::getInstance();
    securityManager = SecurityManager::getInstance();
    
    // Setup dialog properties
    setModal(true);
    setWindowTitle("OTP Verification");
    setFixedSize(450, 400);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    
    // Setup UI
    setupUI();
    setupStyles();
    
    // Initialize timer
    countdownTimer = new QTimer(this);
    connect(countdownTimer, &QTimer::timeout, this, &OTPDialog::onCountdownTimerTimeout);
    
    // Start countdown
    startCountdown();
    
    // Center dialog on parent
    if (parent) {
        move(parent->window()->frameGeometry().center() - frameGeometry().center());
    }
}

OTPDialog::~OTPDialog()
{
    if (countdownTimer) {
        countdownTimer->stop();
    }
}

void OTPDialog::setupUI()
{
    // Create main frame
    mainFrame = new QFrame();
    mainFrame->setObjectName("mainFrame");
    
    // Title and subtitle
    titleLabel = new QLabel("Two-Factor Authentication");
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    
    subtitleLabel = new QLabel("Enter the verification code sent to your registered email");
    subtitleLabel->setObjectName("subtitleLabel");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setWordWrap(true);
    
    // Instructions
    instructionLabel = new QLabel("A 6-digit verification code has been sent to your email address. "
                                 "Please enter it below to complete the authentication process.");
    instructionLabel->setObjectName("instructionLabel");
    instructionLabel->setWordWrap(true);
    instructionLabel->setAlignment(Qt::AlignLeft);
    
    // OTP input
    otpLabel = new QLabel("Verification Code:");
    otpLabel->setObjectName("otpLabel");
    
    otpEdit = new QLineEdit();
    otpEdit->setObjectName("otpEdit");
    otpEdit->setPlaceholderText("Enter 6-digit code");
    otpEdit->setMaxLength(6);
    otpEdit->setAlignment(Qt::AlignCenter);
    
    // Timer and progress
    timerLabel = new QLabel();
    timerLabel->setObjectName("timerLabel");
    timerLabel->setAlignment(Qt::AlignCenter);
    
    progressBar = new QProgressBar();
    progressBar->setObjectName("progressBar");
    progressBar->setRange(0, 600); // 10 minutes in seconds
    progressBar->setValue(600);
    
    // Buttons
    verifyButton = new QPushButton("Verify");
    verifyButton->setObjectName("verifyButton");
    verifyButton->setDefault(true);
    
    resendButton = new QPushButton("Resend Code");
    resendButton->setObjectName("resendButton");
    resendButton->setEnabled(false);
    
    cancelButton = new QPushButton("Cancel");
    cancelButton->setObjectName("cancelButton");
    
    // Status label
    statusLabel = new QLabel();
    statusLabel->setObjectName("statusLabel");
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setWordWrap(true);
    statusLabel->hide();
    
    // Setup layouts
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    
    frameLayout = new QVBoxLayout(mainFrame);
    frameLayout->setSpacing(20);
    frameLayout->setContentsMargins(40, 40, 40, 40);
    
    // Add widgets to frame layout
    frameLayout->addWidget(titleLabel);
    frameLayout->addWidget(subtitleLabel);
    frameLayout->addSpacing(10);
    frameLayout->addWidget(instructionLabel);
    frameLayout->addSpacing(20);
    
    // Form layout
    formLayout = new QGridLayout();
    formLayout->setVerticalSpacing(15);
    
    formLayout->addWidget(otpLabel, 0, 0);
    formLayout->addWidget(otpEdit, 1, 0);
    
    frameLayout->addLayout(formLayout);
    
    // Timer layout
    timerLayout = new QHBoxLayout();
    timerLayout->addWidget(timerLabel);
    
    frameLayout->addLayout(timerLayout);
    frameLayout->addWidget(progressBar);
    
    // Button layout
    buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(resendButton);
    buttonLayout->addWidget(verifyButton);
    
    frameLayout->addLayout(buttonLayout);
    frameLayout->addWidget(statusLabel);
    frameLayout->addStretch();
    
    // Add frame to main layout
    QHBoxLayout* centerLayout = new QHBoxLayout();
    centerLayout->addStretch();
    centerLayout->addWidget(mainFrame);
    centerLayout->addStretch();
    
    mainLayout->addLayout(centerLayout);
    
    // Connect signals
    connect(verifyButton, &QPushButton::clicked, this, &OTPDialog::onVerifyClicked);
    connect(resendButton, &QPushButton::clicked, this, &OTPDialog::onResendClicked);
    connect(cancelButton, &QPushButton::clicked, this, &OTPDialog::reject);
    connect(otpEdit, &QLineEdit::textChanged, this, &OTPDialog::onOTPTextChanged);
    
    // Enable verify button only when OTP is entered
    setVerifyEnabled(false);
}

void OTPDialog::setupStyles()
{
    QString styleSheet = R"(
        /* Dialog background */
        OTPDialog {
            background: rgba(0, 0, 0, 150);
        }
        
        /* Main frame */
        QFrame#mainFrame {
            background: white;
            border-radius: 15px;
            border: 2px solid #e0e0e0;
        }
        
        /* Title labels */
        QLabel#titleLabel {
            font-size: 20px;
            font-weight: bold;
            color: #1a237e;
            margin-bottom: 5px;
        }
        
        QLabel#subtitleLabel {
            font-size: 14px;
            color: #5c6bc0;
            margin-bottom: 10px;
        }
        
        QLabel#instructionLabel {
            font-size: 12px;
            color: #607d8b;
            background: #f5f5f5;
            padding: 15px;
            border-radius: 8px;
            border-left: 4px solid #1a237e;
        }
        
        /* Form labels */
        QLabel#otpLabel {
            font-size: 14px;
            font-weight: 600;
            color: #37474f;
        }
        
        /* OTP input */
        QLineEdit#otpEdit {
            background: white;
            border: 2px solid #e0e0e0;
            border-radius: 8px;
            padding: 15px;
            font-size: 18px;
            font-weight: bold;
            letter-spacing: 8px;
            color: #1a237e;
            text-align: center;
        }
        
        QLineEdit#otpEdit:focus {
            border-color: #1a237e;
            outline: none;
        }
        
        /* Timer label */
        QLabel#timerLabel {
            font-size: 14px;
            color: #f57c00;
            font-weight: 600;
        }
        
        /* Progress bar */
        QProgressBar {
            border: none;
            background: #f5f5f5;
            border-radius: 10px;
            height: 8px;
        }
        
        QProgressBar::chunk {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #4caf50, stop:0.5 #8bc34a, stop:1 #cddc39);
            border-radius: 10px;
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
            min-width: 100px;
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
        
        QPushButton#resendButton {
            background: #ff9800;
        }
        
        QPushButton#resendButton:hover {
            background: #f57c00;
        }
        
        QPushButton#resendButton:disabled {
            background: #ffcc80;
            color: #ffe0b2;
        }
        
        QPushButton#cancelButton {
            background: #757575;
        }
        
        QPushButton#cancelButton:hover {
            background: #616161;
        }
        
        /* Status label */
        QLabel#statusLabel {
            font-size: 12px;
            padding: 10px;
            border-radius: 6px;
        }
    )";
    
    setStyleSheet(styleSheet);
    applyModernStyling();
}

void OTPDialog::applyModernStyling()
{
    // Add drop shadow effect to main frame
    if (mainFrame) {
        QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
        shadow->setBlurRadius(20);
        shadow->setColor(QColor(0, 0, 0, 50));
        shadow->setOffset(0, 10);
        mainFrame->setGraphicsEffect(shadow);
    }
}

void OTPDialog::startCountdown()
{
    remainingSeconds = 600; // 10 minutes
    countdownTimer->start(1000); // Update every second
    updateTimerDisplay();
    disableResendButton();
}

void OTPDialog::stopCountdown()
{
    if (countdownTimer) {
        countdownTimer->stop();
    }
}

void OTPDialog::updateTimerDisplay()
{
    int minutes = remainingSeconds / 60;
    int seconds = remainingSeconds % 60;
    
    timerLabel->setText(QString("Code expires in: %1:%2")
                       .arg(minutes, 2, 10, QChar('0'))
                       .arg(seconds, 2, 10, QChar('0')));
    
    progressBar->setValue(remainingSeconds);
    
    // Change color based on remaining time
    if (remainingSeconds <= 60) {
        timerLabel->setStyleSheet("QLabel#timerLabel { color: #d32f2f; }");
        progressBar->setStyleSheet(
            "QProgressBar::chunk { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
            "stop:0 #f44336, stop:1 #ff9800); }");
    } else if (remainingSeconds <= 180) {
        timerLabel->setStyleSheet("QLabel#timerLabel { color: #ff9800; }");
        progressBar->setStyleSheet(
            "QProgressBar::chunk { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
            "stop:0 #ff9800, stop:1 #ffc107); }");
    }
}

void OTPDialog::enableResendButton()
{
    isResendEnabled = true;
    resendButton->setEnabled(true);
    resendButton->setText("Resend Code");
}

void OTPDialog::disableResendButton()
{
    isResendEnabled = false;
    resendButton->setEnabled(false);
    if (resendAttempts > 0) {
        resendButton->setText(QString("Resend (%1)").arg(3 - resendAttempts));
    } else {
        resendButton->setText("Resend Code");
    }
}

bool OTPDialog::validateOTPInput()
{
    QString otp = otpEdit->text().trimmed();
    
    if (otp.isEmpty()) {
        showStatusMessage("Please enter the verification code", true);
        return false;
    }
    
    if (otp.length() != 6) {
        showStatusMessage("Verification code must be 6 digits", true);
        return false;
    }
    
    QRegularExpression otpRegex(R"(^\d{6}$)");
    if (!otpRegex.match(otp).hasMatch()) {
        showStatusMessage("Verification code must contain only numbers", true);
        return false;
    }
    
    return true;
}

void OTPDialog::verifyOTP()
{
    if (!validateOTPInput()) {
        return;
    }
    
    QString otp = otpEdit->text().trimmed();
    
    // Show loading state
    verifyButton->setEnabled(false);
    verifyButton->setText("Verifying...");
    showStatusMessage("Verifying code...", false);
    
    // Verify OTP through auth manager
    bool verified = authManager->verifyOTP(otp, "LOGIN");
    
    if (verified) {
        showStatusMessage("Verification successful!", false);
        emit otpVerified();
        
        // Close dialog after short delay
        QTimer::singleShot(1000, this, &OTPDialog::accept);
    } else {
        showStatusMessage("Invalid verification code. Please try again.", true);
        verifyButton->setEnabled(true);
        verifyButton->setText("Verify");
        otpEdit->clear();
        otpEdit->setFocus();
    }
}

void OTPDialog::resendOTP()
{
    if (!isResendEnabled || resendAttempts >= 3) {
        return;
    }
    
    resendAttempts++;
    
    // Request new OTP
    bool requested = authManager->requestOTP(authManager->getCurrentUsername(), "LOGIN");
    
    if (requested) {
        showStatusMessage("New verification code sent to your email", false);
        startCountdown(); // Restart countdown
        clearForm();
        
        if (resendAttempts >= 3) {
            disableResendButton();
            resendButton->setText("Resend limit reached");
        }
    } else {
        showStatusMessage("Failed to send new verification code", true);
    }
}

void OTPDialog::showStatusMessage(const QString& message, bool isError)
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
}

void OTPDialog::setVerifyEnabled(bool enabled)
{
    verifyButton->setEnabled(enabled && !otpEdit->text().isEmpty());
}

void OTPDialog::clearForm()
{
    otpEdit->clear();
    statusLabel->hide();
    verifyButton->setText("Verify");
    verifyButton->setEnabled(false);
}

void OTPDialog::closeEvent(QCloseEvent* event)
{
    stopCountdown();
    QDialog::closeEvent(event);
}

void OTPDialog::keyPressEvent(QKeyEvent* event)
{
    // Handle Enter key for verification
    if (event->key() == Qt::Key_Return && verifyButton->isEnabled()) {
        onVerifyClicked();
    }
    // Handle Escape key to cancel
    else if (event->key() == Qt::Key_Escape) {
        reject();
    }
    QDialog::keyPressEvent(event);
}

// Slot implementations
void OTPDialog::onVerifyClicked()
{
    verifyOTP();
}

void OTPDialog::onResendClicked()
{
    resendOTP();
}

void OTPDialog::onCountdownTimerTimeout()
{
    remainingSeconds--;
    
    if (remainingSeconds <= 0) {
        stopCountdown();
        showStatusMessage("Verification code expired. Please request a new code.", true);
        enableResendButton();
        timerLabel->setText("Code expired");
        progressBar->setValue(0);
    } else {
        updateTimerDisplay();
    }
}

void OTPDialog::onOTPTextChanged()
{
    // Auto-enable verify button when 6 digits are entered
    setVerifyEnabled(true);
    
    // Auto-verify when 6 digits are entered
    if (otpEdit->text().length() == 6) {
        QTimer::singleShot(500, this, &OTPDialog::onVerifyClicked);
    }
}

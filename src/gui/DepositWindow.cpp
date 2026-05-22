#include "gui/DepositWindow.h"
#include "core/BankService.h"
#include "core/Account.h"
#include "security/AuthManager.h"
#include <QKeyEvent>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>

DepositWindow::DepositWindow(int userId, QWidget *parent)
    : QDialog(parent)
    , mainFrame(nullptr)
    , titleLabel(nullptr)
    , subtitleLabel(nullptr)
    , accountLabel(nullptr)
    , accountComboBox(nullptr)
    , amountLabel(nullptr)
    , amountSpinBox(nullptr)
    , descriptionLabel(nullptr)
    , descriptionEdit(nullptr)
    , pinLabel(nullptr)
    , pinEdit(nullptr)
    , depositButton(nullptr)
    , cancelButton(nullptr)
    , progressBar(nullptr)
    , statusLabel(nullptr)
    , mainLayout(nullptr)
    , frameLayout(nullptr)
    , formLayout(nullptr)
    , buttonLayout(nullptr)
    , bankService(nullptr)
    , authManager(nullptr)
    , currentUserId(userId)
    , isProcessing(false)
{
    // Initialize services
    bankService = BankService::getInstance();
    authManager = AuthManager::getInstance();
    
    // Setup dialog properties
    setModal(true);
    setWindowTitle("Deposit Funds");
    setFixedSize(500, 600);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    
    // Setup UI
    setupUI();
    setupStyles();
    
    // Load data
    loadAccounts();
    
    // Center dialog on parent
    if (parent) {
        move(parent->window()->frameGeometry().center() - frameGeometry().center());
    }
}

DepositWindow::~DepositWindow()
{
}

void DepositWindow::setupUI()
{
    // Create main frame
    mainFrame = new QFrame();
    mainFrame->setObjectName("mainFrame");
    
    // Title and subtitle
    titleLabel = new QLabel("Deposit Funds");
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    
    subtitleLabel = new QLabel("Add money to your account securely");
    subtitleLabel->setObjectName("subtitleLabel");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    
    // Account selection
    accountLabel = new QLabel("Select Account:");
    accountLabel->setObjectName("formLabel");
    
    accountComboBox = new QComboBox();
    accountComboBox->setObjectName("accountComboBox");
    
    // Amount input
    amountLabel = new QLabel("Deposit Amount:");
    amountLabel->setObjectName("formLabel");
    
    amountSpinBox = new QDoubleSpinBox();
    amountSpinBox->setObjectName("amountSpinBox");
    amountSpinBox->setRange(0.01, 100000.00);
    amountSpinBox->setDecimals(2);
    amountSpinBox->setPrefix("$ ");
    amountSpinBox->setSingleStep(10.00);
    amountSpinBox->setAlignment(Qt::AlignRight);
    
    // Description
    descriptionLabel = new QLabel("Description (Optional):");
    descriptionLabel->setObjectName("formLabel");
    
    descriptionEdit = new QLineEdit();
    descriptionEdit->setObjectName("descriptionEdit");
    descriptionEdit->setPlaceholderText("Enter deposit description");
    descriptionEdit->setMaxLength(100);
    
    // Transaction PIN
    pinLabel = new QLabel("Transaction PIN:");
    pinLabel->setObjectName("formLabel");
    
    pinEdit = new QLineEdit();
    pinEdit->setObjectName("pinEdit");
    pinEdit->setPlaceholderText("Enter 4-digit PIN");
    pinEdit->setEchoMode(QLineEdit::Password);
    pinEdit->setMaxLength(4);
    pinEdit->setAlignment(Qt::AlignCenter);
    
    // Buttons
    depositButton = new QPushButton("Deposit Funds");
    depositButton->setObjectName("depositButton");
    depositButton->setDefault(true);
    
    cancelButton = new QPushButton("Cancel");
    cancelButton->setObjectName("cancelButton");
    
    // Progress bar and status
    progressBar = new QProgressBar();
    progressBar->setObjectName("progressBar");
    progressBar->setRange(0, 0); // Indeterminate progress
    progressBar->hide();
    
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
    frameLayout->addSpacing(20);
    
    // Form layout
    formLayout = new QGridLayout();
    formLayout->setVerticalSpacing(15);
    
    formLayout->addWidget(accountLabel, 0, 0);
    formLayout->addWidget(accountComboBox, 0, 1);
    
    formLayout->addWidget(amountLabel, 1, 0);
    formLayout->addWidget(amountSpinBox, 1, 1);
    
    formLayout->addWidget(descriptionLabel, 2, 0);
    formLayout->addWidget(descriptionEdit, 2, 1);
    
    formLayout->addWidget(pinLabel, 3, 0);
    formLayout->addWidget(pinEdit, 3, 1);
    
    frameLayout->addLayout(formLayout);
    
    // Button layout
    buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(depositButton);
    
    frameLayout->addLayout(buttonLayout);
    frameLayout->addWidget(progressBar);
    frameLayout->addWidget(statusLabel);
    frameLayout->addStretch();
    
    // Add frame to main layout
    QHBoxLayout* centerLayout = new QHBoxLayout();
    centerLayout->addStretch();
    centerLayout->addWidget(mainFrame);
    centerLayout->addStretch();
    
    mainLayout->addLayout(centerLayout);
    
    // Connect signals
    connect(depositButton, &QPushButton::clicked, this, &DepositWindow::onDepositClicked);
    connect(cancelButton, &QPushButton::clicked, this, &DepositWindow::reject);
    connect(accountComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &DepositWindow::onAccountChanged);
    connect(amountSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &DepositWindow::onAmountChanged);
    connect(descriptionEdit, &QLineEdit::textChanged, this, &DepositWindow::onDescriptionChanged);
    connect(pinEdit, &QLineEdit::textChanged, this, &DepositWindow::onPinChanged);
    
    // Initial state
    setProcessingState(false);
}

void DepositWindow::setupStyles()
{
    QString styleSheet = R"(
        /* Dialog background */
        DepositWindow {
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
        
        /* Form labels */
        QLabel#formLabel {
            font-size: 12px;
            font-weight: 600;
            color: #37474f;
        }
        
        /* Combo box */
        QComboBox#accountComboBox {
            background: white;
            border: 2px solid #e0e0e0;
            border-radius: 8px;
            padding: 8px;
            font-size: 14px;
            color: #263238;
        }
        
        QComboBox#accountComboBox:focus {
            border-color: #1a237e;
            outline: none;
        }
        
        QComboBox#accountComboBox::drop-down {
            border: none;
            width: 30px;
        }
        
        QComboBox#accountComboBox::down-arrow {
            image: none;
            border-left: 5px solid transparent;
            border-right: 5px solid transparent;
            border-top: 5px solid #1a237e;
            margin-right: 5px;
        }
        
        /* Double spin box */
        QDoubleSpinBox#amountSpinBox {
            background: white;
            border: 2px solid #e0e0e0;
            border-radius: 8px;
            padding: 8px;
            font-size: 16px;
            font-weight: bold;
            color: #1a237e;
            text-align: right;
        }
        
        QDoubleSpinBox#amountSpinBox:focus {
            border-color: #1a237e;
            outline: none;
        }
        
        /* Line edit */
        QLineEdit#descriptionEdit, QLineEdit#pinEdit {
            background: white;
            border: 2px solid #e0e0e0;
            border-radius: 8px;
            padding: 8px;
            font-size: 14px;
            color: #263238;
        }
        
        QLineEdit#descriptionEdit:focus, QLineEdit#pinEdit:focus {
            border-color: #1a237e;
            outline: none;
        }
        
        QLineEdit#pinEdit {
            font-size: 16px;
            font-weight: bold;
            letter-spacing: 4px;
            text-align: center;
        }
        
        /* Buttons */
        QPushButton#depositButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #4caf50, stop:1 #388e3c);
            color: white;
            border: none;
            border-radius: 8px;
            padding: 12px 24px;
            font-size: 14px;
            font-weight: 600;
            min-width: 120px;
        }
        
        QPushButton#depositButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #66bb6a, stop:1 #4caf50);
        }
        
        QPushButton#depositButton:pressed {
            background: #388e3c;
        }
        
        QPushButton#depositButton:disabled {
            background: #b0bec5;
            color: #78909c;
        }
        
        QPushButton#cancelButton {
            background: #757575;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 12px 24px;
            font-size: 14px;
            font-weight: 600;
            min-width: 100px;
        }
        
        QPushButton#cancelButton:hover {
            background: #616161;
        }
        
        /* Progress bar */
        QProgressBar {
            border: none;
            background: transparent;
            height: 3px;
        }
        
        QProgressBar::chunk {
            background: #4caf50;
            border-radius: 3px;
        }
        
        /* Status label */
        QLabel#statusLabel {
            font-size: 12px;
            padding: 8px;
            border-radius: 6px;
        }
    )";
    
    setStyleSheet(styleSheet);
    applyModernStyling();
}

void DepositWindow::applyModernStyling()
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

void DepositWindow::loadAccounts()
{
    auto accounts = bankService->getUserAccounts(currentUserId);
    accountMap.clear();
    accountComboBox->clear();
    
    for (const auto& account : accounts) {
        if (account->isActiveStatus()) {
            QString displayText = QString("%1 - %2 ($%3)")
                                .arg(account->getAccountNumber())
                                .arg(account->getAccountTypeDisplay())
                                .arg(QLocale().toString(account->getBalance(), 'f', 2));
            
            accountComboBox->addItem(displayText, account->getAccountId());
            accountMap[account->getAccountId()] = account->getAccountNumber();
        }
    }
    
    if (accountComboBox->count() == 0) {
        accountComboBox->addItem("No active accounts available", -1);
        depositButton->setEnabled(false);
    }
}

bool DepositWindow::validateInput()
{
    // Validate account selection
    int accountId = accountComboBox->currentData().toInt();
    if (accountId <= 0) {
        showStatusMessage("Please select a valid account", true);
        return false;
    }
    
    // Validate amount
    double amount = amountSpinBox->value();
    if (amount <= 0) {
        showStatusMessage("Please enter a valid deposit amount", true);
        return false;
    }
    
    if (amount > 100000) {
        showStatusMessage("Maximum deposit amount is $100,000", true);
        return false;
    }
    
    // Validate PIN
    QString pin = pinEdit->text().trimmed();
    if (pin.isEmpty()) {
        showStatusMessage("Please enter your transaction PIN", true);
        return false;
    }
    
    if (pin.length() != 4) {
        showStatusMessage("Transaction PIN must be 4 digits", true);
        return false;
    }
    
    QRegularExpression pinRegex(R"(^\d{4}$)");
    if (!pinRegex.match(pin).hasMatch()) {
        showStatusMessage("Transaction PIN must contain only numbers", true);
        return false;
    }
    
    // Verify PIN with auth manager
    if (!authManager->verifyTransactionPin(pin)) {
        showStatusMessage("Invalid transaction PIN", true);
        return false;
    }
    
    return true;
}

void DepositWindow::processDeposit()
{
    if (!validateInput()) {
        return;
    }
    
    setProcessingState(true);
    
    int accountId = accountComboBox->currentData().toInt();
    double amount = amountSpinBox->value();
    QString description = descriptionEdit->text().trimmed();
    if (description.isEmpty()) {
        description = "Deposit via ATM/Online";
    }
    
    // Process deposit
    auto result = bankService->deposit(accountId, amount, description);
    
    setProcessingState(false);
    
    if (result.success) {
        showStatusMessage("Deposit completed successfully!", false);
        emit depositCompleted(accountMap[accountId], amount);
        
        // Close dialog after short delay
        QTimer::singleShot(2000, this, &DepositWindow::accept);
    } else {
        showStatusMessage(QString("Deposit failed: %1").arg(result.message), true);
    }
}

void DepositWindow::showStatusMessage(const QString& message, bool isError)
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

void DepositWindow::setProcessingState(bool processing)
{
    isProcessing = processing;
    
    if (processing) {
        progressBar->show();
        depositButton->setEnabled(false);
        depositButton->setText("Processing...");
        showStatusMessage("Processing deposit...", false);
    } else {
        progressBar->hide();
        depositButton->setEnabled(true);
        depositButton->setText("Deposit Funds");
    }
}

void DepositWindow::clearForm()
{
    amountSpinBox->setValue(0.00);
    descriptionEdit->clear();
    pinEdit->clear();
    statusLabel->hide();
}

void DepositWindow::closeEvent(QCloseEvent* event)
{
    if (isProcessing) {
        event->ignore();
        return;
    }
    QDialog::closeEvent(event);
}

void DepositWindow::keyPressEvent(QKeyEvent* event)
{
    // Handle Enter key for deposit
    if (event->key() == Qt::Key_Return && depositButton->isEnabled()) {
        onDepositClicked();
    }
    // Handle Escape key to cancel
    else if (event->key() == Qt::Key_Escape && !isProcessing) {
        reject();
    }
    QDialog::keyPressEvent(event);
}

// Slot implementations
void DepositWindow::onDepositClicked()
{
    processDeposit();
}

void DepositWindow::onAccountChanged()
{
    // Enable/disable deposit button based on account selection
    int accountId = accountComboBox->currentData().toInt();
    depositButton->setEnabled(accountId > 0 && !isProcessing);
}

void DepositWindow::onAmountChanged()
{
    // Enable/disable deposit button based on amount
    double amount = amountSpinBox->value();
    depositButton->setEnabled(amount > 0 && !isProcessing && accountComboBox->currentData().toInt() > 0);
}

void DepositWindow::onDescriptionChanged()
{
    // Description is optional, no validation needed
}

void DepositWindow::onPinChanged()
{
    // Enable/disable deposit button based on PIN
    QString pin = pinEdit->text();
    bool validPin = pin.length() == 4 && QRegularExpression(R"(^\d{4}$)").match(pin).hasMatch();
    depositButton->setEnabled(validPin && !isProcessing && 
                           amountSpinBox->value() > 0 && 
                           accountComboBox->currentData().toInt() > 0);
}

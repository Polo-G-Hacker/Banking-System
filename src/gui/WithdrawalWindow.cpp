#include "gui/WithdrawalWindow.h"
#include "core/BankService.h"
#include "core/Account.h"
#include "security/AuthManager.h"
#include <QKeyEvent>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>

WithdrawalWindow::WithdrawalWindow(int userId, QWidget *parent)
    : QDialog(parent)
    , mainFrame(nullptr)
    , titleLabel(nullptr)
    , subtitleLabel(nullptr)
    , accountLabel(nullptr)
    , accountComboBox(nullptr)
    , amountLabel(nullptr)
    , amountSpinBox(nullptr)
    , balanceLabel(nullptr)
    , balanceValue(nullptr)
    , descriptionLabel(nullptr)
    , descriptionEdit(nullptr)
    , pinLabel(nullptr)
    , pinEdit(nullptr)
    , withdrawButton(nullptr)
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
    setWindowTitle("Withdraw Funds");
    setFixedSize(500, 650);
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

WithdrawalWindow::~WithdrawalWindow()
{
}

void WithdrawalWindow::setupUI()
{
    // Create main frame
    mainFrame = new QFrame();
    mainFrame->setObjectName("mainFrame");
    
    // Title and subtitle
    titleLabel = new QLabel("Withdraw Funds");
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    
    subtitleLabel = new QLabel("Withdraw money from your account securely");
    subtitleLabel->setObjectName("subtitleLabel");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    
    // Account selection
    accountLabel = new QLabel("Select Account:");
    accountLabel->setObjectName("formLabel");
    
    accountComboBox = new QComboBox();
    accountComboBox->setObjectName("accountComboBox");
    
    // Amount input
    amountLabel = new QLabel("Withdrawal Amount:");
    amountLabel->setObjectName("formLabel");
    
    amountSpinBox = new QDoubleSpinBox();
    amountSpinBox->setObjectName("amountSpinBox");
    amountSpinBox->setRange(0.01, 100000.00);
    amountSpinBox->setDecimals(2);
    amountSpinBox->setPrefix("$ ");
    amountSpinBox->setSingleStep(10.00);
    amountSpinBox->setAlignment(Qt::AlignRight);
    
    // Balance display
    balanceLabel = new QLabel("Available Balance:");
    balanceLabel->setObjectName("balanceLabel");
    
    balanceValue = new QLabel("$0.00");
    balanceValue->setObjectName("balanceValue");
    
    // Description
    descriptionLabel = new QLabel("Description (Optional):");
    descriptionLabel->setObjectName("formLabel");
    
    descriptionEdit = new QLineEdit();
    descriptionEdit->setObjectName("descriptionEdit");
    descriptionEdit->setPlaceholderText("Enter withdrawal description");
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
    withdrawButton = new QPushButton("Withdraw Funds");
    withdrawButton->setObjectName("withdrawButton");
    withdrawButton->setDefault(true);
    
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
    
    formLayout->addWidget(balanceLabel, 2, 0);
    formLayout->addWidget(balanceValue, 2, 1);
    
    formLayout->addWidget(descriptionLabel, 3, 0);
    formLayout->addWidget(descriptionEdit, 3, 1);
    
    formLayout->addWidget(pinLabel, 4, 0);
    formLayout->addWidget(pinEdit, 4, 1);
    
    frameLayout->addLayout(formLayout);
    
    // Button layout
    buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(withdrawButton);
    
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
    connect(withdrawButton, &QPushButton::clicked, this, &WithdrawalWindow::onWithdrawClicked);
    connect(cancelButton, &QPushButton::clicked, this, &WithdrawalWindow::reject);
    connect(accountComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &WithdrawalWindow::onAccountChanged);
    connect(amountSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &WithdrawalWindow::onAmountChanged);
    connect(descriptionEdit, &QLineEdit::textChanged, this, &WithdrawalWindow::onDescriptionChanged);
    connect(pinEdit, &QLineEdit::textChanged, this, &WithdrawalWindow::onPinChanged);
    
    // Initial state
    setProcessingState(false);
}

void WithdrawalWindow::setupStyles()
{
    QString styleSheet = R"(
        /* Dialog background */
        WithdrawalWindow {
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
            color: #d32f2f;
            margin-bottom: 5px;
        }
        
        QLabel#subtitleLabel {
            font-size: 14px;
            color: #ef5350;
            margin-bottom: 10px;
        }
        
        /* Form labels */
        QLabel#formLabel {
            font-size: 12px;
            font-weight: 600;
            color: #37474f;
        }
        
        QLabel#balanceLabel {
            font-size: 12px;
            font-weight: 600;
            color: #2e7d32;
        }
        
        QLabel#balanceValue {
            font-size: 16px;
            font-weight: bold;
            color: #2e7d32;
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
            border-color: #d32f2f;
            outline: none;
        }
        
        /* Double spin box */
        QDoubleSpinBox#amountSpinBox {
            background: white;
            border: 2px solid #e0e0e0;
            border-radius: 8px;
            padding: 8px;
            font-size: 16px;
            font-weight: bold;
            color: #d32f2f;
            text-align: right;
        }
        
        QDoubleSpinBox#amountSpinBox:focus {
            border-color: #d32f2f;
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
            border-color: #d32f2f;
            outline: none;
        }
        
        QLineEdit#pinEdit {
            font-size: 16px;
            font-weight: bold;
            letter-spacing: 4px;
            text-align: center;
        }
        
        /* Buttons */
        QPushButton#withdrawButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #d32f2f, stop:1 #c62828);
            color: white;
            border: none;
            border-radius: 8px;
            padding: 12px 24px;
            font-size: 14px;
            font-weight: 600;
            min-width: 120px;
        }
        
        QPushButton#withdrawButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #ef5350, stop:1 #d32f2f);
        }
        
        QPushButton#withdrawButton:pressed {
            background: #c62828;
        }
        
        QPushButton#withdrawButton:disabled {
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
            background: #d32f2f;
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

void WithdrawalWindow::applyModernStyling()
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

void WithdrawalWindow::loadAccounts()
{
    auto accounts = bankService->getUserAccounts(currentUserId);
    accountMap.clear();
    accountBalances.clear();
    accountComboBox->clear();
    
    for (const auto& account : accounts) {
        if (account->isActiveStatus()) {
            QString displayText = QString("%1 - %2 ($%3)")
                                .arg(account->getAccountNumber())
                                .arg(account->getAccountTypeDisplay())
                                .arg(QLocale().toString(account->getBalance(), 'f', 2));
            
            accountComboBox->addItem(displayText, account->getAccountId());
            accountMap[account->getAccountId()] = account->getAccountNumber();
            accountBalances[account->getAccountId()] = account->getBalance();
        }
    }
    
    if (accountComboBox->count() == 0) {
        accountComboBox->addItem("No active accounts available", -1);
        withdrawButton->setEnabled(false);
    }
    
    updateBalanceDisplay();
}

void WithdrawalWindow::updateBalanceDisplay()
{
    int accountId = accountComboBox->currentData().toInt();
    
    if (accountId > 0 && accountBalances.contains(accountId)) {
        double balance = accountBalances[accountId];
        balanceValue->setText(QString("$%1").arg(QLocale().toString(balance, 'f', 2)));
        
        // Set maximum withdrawal amount to available balance
        amountSpinBox->setMaximum(balance);
    } else {
        balanceValue->setText("$0.00");
        amountSpinBox->setMaximum(0.00);
    }
}

bool WithdrawalWindow::validateInput()
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
        showStatusMessage("Please enter a valid withdrawal amount", true);
        return false;
    }
    
    // Check sufficient balance
    double availableBalance = accountBalances.value(accountId, 0.0);
    if (amount > availableBalance) {
        showStatusMessage("Insufficient balance for this withdrawal", true);
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

void WithdrawalWindow::processWithdrawal()
{
    if (!validateInput()) {
        return;
    }
    
    // Confirmation dialog
    int accountId = accountComboBox->currentData().toInt();
    double amount = amountSpinBox->value();
    QString accountNumber = accountMap[accountId];
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirm Withdrawal",
        QString("Are you sure you want to withdraw $%1 from account %2?\n\n"
                "This action cannot be undone.")
        .arg(QLocale().toString(amount, 'f', 2))
        .arg(accountNumber),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );
    
    if (reply != QMessageBox::Yes) {
        return;
    }
    
    setProcessingState(true);
    
    QString description = descriptionEdit->text().trimmed();
    if (description.isEmpty()) {
        description = "Withdrawal via ATM/Online";
    }
    
    // Process withdrawal
    auto result = bankService->withdraw(accountId, amount, description);
    
    setProcessingState(false);
    
    if (result.success) {
        showStatusMessage("Withdrawal completed successfully!", false);
        emit withdrawalCompleted(accountNumber, amount);
        
        // Update balance display
        accountBalances[accountId] = result.newBalance;
        updateBalanceDisplay();
        
        // Close dialog after short delay
        QTimer::singleShot(2000, this, &WithdrawalWindow::accept);
    } else {
        showStatusMessage(QString("Withdrawal failed: %1").arg(result.message), true);
    }
}

void WithdrawalWindow::showStatusMessage(const QString& message, bool isError)
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

void WithdrawalWindow::setProcessingState(bool processing)
{
    isProcessing = processing;
    
    if (processing) {
        progressBar->show();
        withdrawButton->setEnabled(false);
        withdrawButton->setText("Processing...");
        showStatusMessage("Processing withdrawal...", false);
    } else {
        progressBar->hide();
        withdrawButton->setEnabled(true);
        withdrawButton->setText("Withdraw Funds");
    }
}

void WithdrawalWindow::clearForm()
{
    amountSpinBox->setValue(0.00);
    descriptionEdit->clear();
    pinEdit->clear();
    statusLabel->hide();
}

void WithdrawalWindow::closeEvent(QCloseEvent* event)
{
    if (isProcessing) {
        event->ignore();
        return;
    }
    QDialog::closeEvent(event);
}

void WithdrawalWindow::keyPressEvent(QKeyEvent* event)
{
    // Handle Enter key for withdrawal
    if (event->key() == Qt::Key_Return && withdrawButton->isEnabled()) {
        onWithdrawClicked();
    }
    // Handle Escape key to cancel
    else if (event->key() == Qt::Key_Escape && !isProcessing) {
        reject();
    }
    QDialog::keyPressEvent(event);
}

// Slot implementations
void WithdrawalWindow::onWithdrawClicked()
{
    processWithdrawal();
}

void WithdrawalWindow::onAccountChanged()
{
    updateBalanceDisplay();
    
    // Enable/disable withdraw button based on account selection
    int accountId = accountComboBox->currentData().toInt();
    withdrawButton->setEnabled(accountId > 0 && !isProcessing);
}

void WithdrawalWindow::onAmountChanged()
{
    // Enable/disable withdraw button based on amount
    double amount = amountSpinBox->value();
    int accountId = accountComboBox->currentData().toInt();
    double availableBalance = accountBalances.value(accountId, 0.0);
    
    withdrawButton->setEnabled(amount > 0 && amount <= availableBalance && 
                           !isProcessing && accountId > 0);
}

void WithdrawalWindow::onDescriptionChanged()
{
    // Description is optional, no validation needed
}

void WithdrawalWindow::onPinChanged()
{
    // Enable/disable withdraw button based on PIN
    QString pin = pinEdit->text();
    bool validPin = pin.length() == 4 && QRegularExpression(R"(^\d{4}$)").match(pin).hasMatch();
    
    double amount = amountSpinBox->value();
    int accountId = accountComboBox->currentData().toInt();
    double availableBalance = accountBalances.value(accountId, 0.0);
    
    withdrawButton->setEnabled(validPin && amount > 0 && amount <= availableBalance && 
                           !isProcessing && accountId > 0);
}

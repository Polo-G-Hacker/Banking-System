#include "gui/TransferWindow.h"
#include "core/BankService.h"
#include "core/Account.h"
#include "security/AuthManager.h"
#include <QKeyEvent>
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>

TransferWindow::TransferWindow(int userId, QWidget *parent)
    : QDialog(parent)
    , mainFrame(nullptr)
    , titleLabel(nullptr)
    , subtitleLabel(nullptr)
    , fromAccountLabel(nullptr)
    , fromAccountComboBox(nullptr)
    , toAccountLabel(nullptr)
    , toAccountEdit(nullptr)
    , amountLabel(nullptr)
    , amountSpinBox(nullptr)
    , fromBalanceLabel(nullptr)
    , fromBalanceValue(nullptr)
    , descriptionLabel(nullptr)
    , descriptionEdit(nullptr)
    , pinLabel(nullptr)
    , pinEdit(nullptr)
    , transferButton(nullptr)
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
    setWindowTitle("Transfer Funds");
    setFixedSize(500, 700);
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

TransferWindow::~TransferWindow()
{
}

void TransferWindow::setupUI()
{
    // Create main frame
    mainFrame = new QFrame();
    mainFrame->setObjectName("mainFrame");
    
    // Title and subtitle
    titleLabel = new QLabel("Transfer Funds");
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    
    subtitleLabel = new QLabel("Transfer money securely to another account");
    subtitleLabel->setObjectName("subtitleLabel");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    
    // From account selection
    fromAccountLabel = new QLabel("From Account:");
    fromAccountLabel->setObjectName("formLabel");
    
    fromAccountComboBox = new QComboBox();
    fromAccountComboBox->setObjectName("accountComboBox");
    
    // To account input
    toAccountLabel = new QLabel("To Account Number:");
    toAccountLabel->setObjectName("formLabel");
    
    toAccountEdit = new QLineEdit();
    toAccountEdit->setObjectName("accountEdit");
    toAccountEdit->setPlaceholderText("Enter destination account number");
    toAccountEdit->setMaxLength(20);
    
    // Amount input
    amountLabel = new QLabel("Transfer Amount:");
    amountLabel->setObjectName("formLabel");
    
    amountSpinBox = new QDoubleSpinBox();
    amountSpinBox->setObjectName("amountSpinBox");
    amountSpinBox->setRange(0.01, 100000.00);
    amountSpinBox->setDecimals(2);
    amountSpinBox->setPrefix("$ ");
    amountSpinBox->setSingleStep(10.00);
    amountSpinBox->setAlignment(Qt::AlignRight);
    
    // Balance display
    fromBalanceLabel = new QLabel("Available Balance:");
    fromBalanceLabel->setObjectName("balanceLabel");
    
    fromBalanceValue = new QLabel("$0.00");
    fromBalanceValue->setObjectName("balanceValue");
    
    // Description
    descriptionLabel = new QLabel("Description (Optional):");
    descriptionLabel->setObjectName("formLabel");
    
    descriptionEdit = new QLineEdit();
    descriptionEdit->setObjectName("descriptionEdit");
    descriptionEdit->setPlaceholderText("Enter transfer description");
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
    transferButton = new QPushButton("Transfer Funds");
    transferButton->setObjectName("transferButton");
    transferButton->setDefault(true);
    
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
    
    formLayout->addWidget(fromAccountLabel, 0, 0);
    formLayout->addWidget(fromAccountComboBox, 0, 1);
    
    formLayout->addWidget(toAccountLabel, 1, 0);
    formLayout->addWidget(toAccountEdit, 1, 1);
    
    formLayout->addWidget(amountLabel, 2, 0);
    formLayout->addWidget(amountSpinBox, 2, 1);
    
    formLayout->addWidget(fromBalanceLabel, 3, 0);
    formLayout->addWidget(fromBalanceValue, 3, 1);
    
    formLayout->addWidget(descriptionLabel, 4, 0);
    formLayout->addWidget(descriptionEdit, 4, 1);
    
    formLayout->addWidget(pinLabel, 5, 0);
    formLayout->addWidget(pinEdit, 5, 1);
    
    frameLayout->addLayout(formLayout);
    
    // Button layout
    buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(transferButton);
    
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
    connect(transferButton, &QPushButton::clicked, this, &TransferWindow::onTransferClicked);
    connect(cancelButton, &QPushButton::clicked, this, &TransferWindow::reject);
    connect(fromAccountComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &TransferWindow::onFromAccountChanged);
    connect(toAccountEdit, &QLineEdit::textChanged, this, &TransferWindow::onToAccountChanged);
    connect(amountSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), 
            this, &TransferWindow::onAmountChanged);
    connect(descriptionEdit, &QLineEdit::textChanged, this, &TransferWindow::onDescriptionChanged);
    connect(pinEdit, &QLineEdit::textChanged, this, &TransferWindow::onPinChanged);
    
    // Initial state
    setProcessingState(false);
}

void TransferWindow::setupStyles()
{
    QString styleSheet = R"(
        /* Dialog background */
        TransferWindow {
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
            color: #1976d2;
            margin-bottom: 5px;
        }
        
        QLabel#subtitleLabel {
            font-size: 14px;
            color: #42a5f5;
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
            border-color: #1976d2;
            outline: none;
        }
        
        /* Line edits */
        QLineEdit#accountEdit, QLineEdit#descriptionEdit, QLineEdit#pinEdit {
            background: white;
            border: 2px solid #e0e0e0;
            border-radius: 8px;
            padding: 8px;
            font-size: 14px;
            color: #263238;
        }
        
        QLineEdit#accountEdit:focus, QLineEdit#descriptionEdit:focus, QLineEdit#pinEdit:focus {
            border-color: #1976d2;
            outline: none;
        }
        
        QLineEdit#pinEdit {
            font-size: 16px;
            font-weight: bold;
            letter-spacing: 4px;
            text-align: center;
        }
        
        /* Double spin box */
        QDoubleSpinBox#amountSpinBox {
            background: white;
            border: 2px solid #e0e0e0;
            border-radius: 8px;
            padding: 8px;
            font-size: 16px;
            font-weight: bold;
            color: #1976d2;
            text-align: right;
        }
        
        QDoubleSpinBox#amountSpinBox:focus {
            border-color: #1976d2;
            outline: none;
        }
        
        /* Buttons */
        QPushButton#transferButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #1976d2, stop:1 #1565c0);
            color: white;
            border: none;
            border-radius: 8px;
            padding: 12px 24px;
            font-size: 14px;
            font-weight: 600;
            min-width: 120px;
        }
        
        QPushButton#transferButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #42a5f5, stop:1 #1976d2);
        }
        
        QPushButton#transferButton:pressed {
            background: #1565c0;
        }
        
        QPushButton#transferButton:disabled {
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
            background: #1976d2;
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

void TransferWindow::applyModernStyling()
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

void TransferWindow::loadAccounts()
{
    auto accounts = bankService->getUserAccounts(currentUserId);
    accountMap.clear();
    accountBalances.clear();
    fromAccountComboBox->clear();
    
    for (const auto& account : accounts) {
        if (account->isActiveStatus()) {
            QString displayText = QString("%1 - %2 ($%3)")
                                .arg(account->getAccountNumber())
                                .arg(account->getAccountTypeDisplay())
                                .arg(QLocale().toString(account->getBalance(), 'f', 2));
            
            fromAccountComboBox->addItem(displayText, account->getAccountId());
            accountMap[account->getAccountId()] = account->getAccountNumber();
            accountBalances[account->getAccountId()] = account->getBalance();
        }
    }
    
    if (fromAccountComboBox->count() == 0) {
        fromAccountComboBox->addItem("No active accounts available", -1);
        transferButton->setEnabled(false);
    }
    
    updateBalanceDisplay();
}

void TransferWindow::updateBalanceDisplay()
{
    int accountId = fromAccountComboBox->currentData().toInt();
    
    if (accountId > 0 && accountBalances.contains(accountId)) {
        double balance = accountBalances[accountId];
        fromBalanceValue->setText(QString("$%1").arg(QLocale().toString(balance, 'f', 2)));
        
        // Set maximum transfer amount to available balance
        amountSpinBox->setMaximum(balance);
    } else {
        fromBalanceValue->setText("$0.00");
        amountSpinBox->setMaximum(0.00);
    }
}

bool TransferWindow::validateInput()
{
    // Validate from account selection
    int fromAccountId = fromAccountComboBox->currentData().toInt();
    if (fromAccountId <= 0) {
        showStatusMessage("Please select a valid source account", true);
        return false;
    }
    
    // Validate to account
    QString toAccountNumber = toAccountEdit->text().trimmed();
    if (toAccountNumber.isEmpty()) {
        showStatusMessage("Please enter destination account number", true);
        return false;
    }
    
    if (!validateTargetAccount(toAccountNumber)) {
        return false;
    }
    
    // Check if transferring to same account
    QString fromAccountNumber = accountMap[fromAccountId];
    if (toAccountNumber == fromAccountNumber) {
        showStatusMessage("Cannot transfer to the same account", true);
        return false;
    }
    
    // Validate amount
    double amount = amountSpinBox->value();
    if (amount <= 0) {
        showStatusMessage("Please enter a valid transfer amount", true);
        return false;
    }
    
    // Check sufficient balance
    double availableBalance = accountBalances.value(fromAccountId, 0.0);
    if (amount > availableBalance) {
        showStatusMessage("Insufficient balance for this transfer", true);
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

bool TransferWindow::validateTargetAccount(const QString& accountNumber)
{
    // Validate account number format
    QRegularExpression accountRegex(R"(^ACC\d{6,}$)");
    if (!accountRegex.match(accountNumber).hasMatch()) {
        showStatusMessage("Invalid account number format. Account numbers start with 'ACC'", true);
        return false;
    }
    
    // Check if account exists
    auto targetAccount = bankService->getAccountByNumber(accountNumber);
    if (!targetAccount) {
        showStatusMessage("Destination account not found", true);
        return false;
    }
    
    if (!targetAccount->isActiveStatus()) {
        showStatusMessage("Destination account is not active", true);
        return false;
    }
    
    return true;
}

void TransferWindow::processTransfer()
{
    if (!validateInput()) {
        return;
    }
    
    int fromAccountId = fromAccountComboBox->currentData().toInt();
    QString fromAccountNumber = accountMap[fromAccountId];
    QString toAccountNumber = toAccountEdit->text().trimmed();
    double amount = amountSpinBox->value();
    QString description = descriptionEdit->text().trimmed();
    
    // Get target account
    auto targetAccount = bankService->getAccountByNumber(toAccountNumber);
    if (!targetAccount) {
        showStatusMessage("Destination account not found", true);
        return;
    }
    
    int toAccountId = targetAccount->getAccountId();
    
    // Confirmation dialog
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirm Transfer",
        QString("Are you sure you want to transfer $%1 from account %2 to account %3?\n\n"
                "This action cannot be undone.")
        .arg(QLocale().toString(amount, 'f', 2))
        .arg(fromAccountNumber)
        .arg(toAccountNumber),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );
    
    if (reply != QMessageBox::Yes) {
        return;
    }
    
    setProcessingState(true);
    
    if (description.isEmpty()) {
        description = QString("Transfer to account %1").arg(toAccountNumber);
    }
    
    // Process transfer
    auto result = bankService->transfer(fromAccountId, toAccountId, amount, description);
    
    setProcessingState(false);
    
    if (result.success) {
        showStatusMessage("Transfer completed successfully!", false);
        emit transferCompleted(fromAccountNumber, toAccountNumber, amount);
        
        // Update balance display
        accountBalances[fromAccountId] = result.newBalance;
        updateBalanceDisplay();
        
        // Clear form and close after short delay
        QTimer::singleShot(2000, this, &TransferWindow::accept);
    } else {
        showStatusMessage(QString("Transfer failed: %1").arg(result.message), true);
    }
}

void TransferWindow::showStatusMessage(const QString& message, bool isError)
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

void TransferWindow::setProcessingState(bool processing)
{
    isProcessing = processing;
    
    if (processing) {
        progressBar->show();
        transferButton->setEnabled(false);
        transferButton->setText("Processing...");
        showStatusMessage("Processing transfer...", false);
    } else {
        progressBar->hide();
        transferButton->setEnabled(true);
        transferButton->setText("Transfer Funds");
    }
}

void TransferWindow::clearForm()
{
    toAccountEdit->clear();
    amountSpinBox->setValue(0.00);
    descriptionEdit->clear();
    pinEdit->clear();
    statusLabel->hide();
}

void TransferWindow::closeEvent(QCloseEvent* event)
{
    if (isProcessing) {
        event->ignore();
        return;
    }
    QDialog::closeEvent(event);
}

void TransferWindow::keyPressEvent(QKeyEvent* event)
{
    // Handle Enter key for transfer
    if (event->key() == Qt::Key_Return && transferButton->isEnabled()) {
        onTransferClicked();
    }
    // Handle Escape key to cancel
    else if (event->key() == Qt::Key_Escape && !isProcessing) {
        reject();
    }
    QDialog::keyPressEvent(event);
}

// Slot implementations
void TransferWindow::onTransferClicked()
{
    processTransfer();
}

void TransferWindow::onFromAccountChanged()
{
    updateBalanceDisplay();
    
    // Enable/disable transfer button based on account selection
    int accountId = fromAccountComboBox->currentData().toInt();
    transferButton->setEnabled(accountId > 0 && !isProcessing);
}

void TransferWindow::onToAccountChanged()
{
    // Basic validation as user types
    QString accountNumber = toAccountEdit->text().trimmed();
    bool validFormat = QRegularExpression(R"(^ACC\d{0,}$)").match(accountNumber).hasMatch();
    
    if (!validFormat && !accountNumber.isEmpty()) {
        toAccountEdit->setStyleSheet("QLineEdit#accountEdit { border-color: #d32f2f; }");
    } else {
        toAccountEdit->setStyleSheet("QLineEdit#accountEdit { border-color: #e0e0e0; }");
    }
    
    updateTransferButtonState();
}

void TransferWindow::onAmountChanged()
{
    // Enable/disable transfer button based on amount
    double amount = amountSpinBox->value();
    int accountId = fromAccountComboBox->currentData().toInt();
    double availableBalance = accountBalances.value(accountId, 0.0);
    
    if (amount > availableBalance) {
        amountSpinBox->setStyleSheet("QDoubleSpinBox#amountSpinBox { border-color: #d32f2f; }");
    } else {
        amountSpinBox->setStyleSheet("QDoubleSpinBox#amountSpinBox { border-color: #e0e0e0; }");
    }
    
    updateTransferButtonState();
}

void TransferWindow::onDescriptionChanged()
{
    // Description is optional, no validation needed
}

void TransferWindow::onPinChanged()
{
    // Enable/disable transfer button based on PIN
    QString pin = pinEdit->text();
    bool validPin = pin.length() == 4 && QRegularExpression(R"(^\d{4}$)").match(pin).hasMatch();
    
    if (!validPin && !pin.isEmpty()) {
        pinEdit->setStyleSheet("QLineEdit#pinEdit { border-color: #d32f2f; }");
    } else {
        pinEdit->setStyleSheet("QLineEdit#pinEdit { border-color: #e0e0e0; }");
    }
    
    updateTransferButtonState();
}

void TransferWindow::updateTransferButtonState()
{
    QString accountNumber = toAccountEdit->text().trimmed();
    QString pin = pinEdit->text();
    double amount = amountSpinBox->value();
    int accountId = fromAccountComboBox->currentData().toInt();
    double availableBalance = accountBalances.value(accountId, 0.0);
    
    bool validAccount = QRegularExpression(R"(^ACC\d{6,}$)").match(accountNumber).hasMatch();
    bool validPin = pin.length() == 4 && QRegularExpression(R"(^\d{4}$)").match(pin).hasMatch();
    bool validAmount = amount > 0 && amount <= availableBalance;
    
    transferButton->setEnabled(validAccount && validPin && validAmount && 
                           !isProcessing && accountId > 0);
}

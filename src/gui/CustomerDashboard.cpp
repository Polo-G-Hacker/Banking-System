#include "gui/CustomerDashboard.h"
#include "core/BankService.h"
#include "security/AuthManager.h"
#include "security/SecurityManager.h"
#include "gui/DepositWindow.h"
#include "gui/WithdrawalWindow.h"
#include "gui/TransferWindow.h"
#include "gui/TransactionHistoryWindow.h"
#include "core/Account.h"
#include "core/Transaction.h"
#include <QApplication>
#include <QMessageBox>
#include <QKeyEvent>
#include <QHeaderView>
#include <QDateTime>
#include <QGuiApplication>
#include <QInputDialog>
#include <QLocale>
#include <QScreen>
#include <QSplitter>
#include <QGraphicsDropShadowEffect>

CustomerDashboard::CustomerDashboard(int userId, const QString& username, QWidget *parent)
    : QMainWindow(parent)
    , menuBar(nullptr)
    , statusBar(nullptr)
    , fileMenu(nullptr)
    , accountMenu(nullptr)
    , transactionMenu(nullptr)
    , securityMenu(nullptr)
    , helpMenu(nullptr)
    , centralWidget(nullptr)
    , mainSplitter(nullptr)
    , leftPanel(nullptr)
    , leftScrollArea(nullptr)
    , leftPanelContent(nullptr)
    , leftPanelLayout(nullptr)
    , userInfoFrame(nullptr)
    , welcomeLabel(nullptr)
    , usernameLabel(nullptr)
    , userIdLabel(nullptr)
    , lastLoginLabel(nullptr)
    , accountSummaryFrame(nullptr)
    , totalBalanceLabel(nullptr)
    , totalBalanceValue(nullptr)
    , balanceProgressBar(nullptr)
    , accountsTable(nullptr)
    , quickActionsFrame(nullptr)
    , depositButton(nullptr)
    , withdrawButton(nullptr)
    , transferButton(nullptr)
    , historyButton(nullptr)
    , changePasswordButton(nullptr)
    , changePinButton(nullptr)
    , rightPanel(nullptr)
    , rightScrollArea(nullptr)
    , rightPanelContent(nullptr)
    , rightPanelLayout(nullptr)
    , recentTransactionsFrame(nullptr)
    , recentTransactionsTable(nullptr)
    , viewAllTransactionsButton(nullptr)
    , notificationsFrame(nullptr)
    , notificationsLabel(nullptr)
    , notificationsContent(nullptr)
    , clearNotificationsButton(nullptr)
    , sessionStatusLabel(nullptr)
    , securityStatusLabel(nullptr)
    , updateTimer(nullptr)
    , bankService(nullptr)
    , authManager(nullptr)
    , securityManager(nullptr)
    , currentUserId(userId)
    , currentUsername(username)
    , isDataLoaded(false)
{
    // Initialize services
    bankService = BankService::getInstance();
    authManager = AuthManager::getInstance();
    securityManager = SecurityManager::getInstance();
    
    // Setup UI
    setupUI();
    setupStyles();
    
    // Load initial data
    loadUserData();
    
    // Setup update timer
    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &CustomerDashboard::onUpdateTimer);
    updateTimer->start(30000); // Update every 30 seconds
    
    // Window properties
    setWindowTitle("Banking Management System - Customer Dashboard");
    setMinimumSize(1200, 800);
    resize(1400, 900);
    
    // Center window on screen
    if (QScreen* screen = QGuiApplication::primaryScreen()) {
        move(screen->availableGeometry().center() - rect().center());
    }
}

CustomerDashboard::~CustomerDashboard()
{
    if (updateTimer) {
        updateTimer->stop();
    }
}

void CustomerDashboard::setupUI()
{
    // Setup menu bar and status bar
    setupMenuBar();
    setupStatusBar();
    
    // Create central widget
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // Create main splitter
    mainSplitter = new QSplitter(Qt::Horizontal, centralWidget);
    
    // Setup panels
    setupLeftPanel();
    setupRightPanel();
    
    // Add panels to splitter
    mainSplitter->addWidget(leftPanel);
    mainSplitter->addWidget(rightPanel);
    mainSplitter->setStretchFactor(0, 1);
    mainSplitter->setStretchFactor(1, 1);
    mainSplitter->setChildrenCollapsible(false);
    
    // Main layout
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->addWidget(mainSplitter);
}

void CustomerDashboard::setupMenuBar()
{
    menuBar = new QMenuBar(this);
    
    // File menu
    fileMenu = menuBar->addMenu("&File");
    QAction* refreshAction = fileMenu->addAction("&Refresh Data");
    refreshAction->setShortcut(QKeySequence::Refresh);
    connect(refreshAction, &QAction::triggered, this, &CustomerDashboard::onRefreshData);
    
    fileMenu->addSeparator();
    QAction* logoutAction = fileMenu->addAction("&Logout");
    logoutAction->setShortcut(QKeySequence("Ctrl+L"));
    connect(logoutAction, &QAction::triggered, this, &CustomerDashboard::onLogout);
    
    QAction* exitAction = fileMenu->addAction("E&xit");
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &CustomerDashboard::onExit);
    
    // Account menu
    accountMenu = menuBar->addMenu("&Account");
    QAction* depositAction = accountMenu->addAction("&Deposit");
    depositAction->setShortcut(QKeySequence("Ctrl+D"));
    connect(depositAction, &QAction::triggered, this, &CustomerDashboard::onDepositClicked);
    
    QAction* withdrawAction = accountMenu->addAction("&Withdraw");
    withdrawAction->setShortcut(QKeySequence("Ctrl+W"));
    connect(withdrawAction, &QAction::triggered, this, &CustomerDashboard::onWithdrawClicked);
    
    QAction* transferAction = accountMenu->addAction("&Transfer");
    transferAction->setShortcut(QKeySequence("Ctrl+T"));
    connect(transferAction, &QAction::triggered, this, &CustomerDashboard::onTransferClicked);
    
    accountMenu->addSeparator();
    QAction* changePasswordAction = accountMenu->addAction("Change &Password");
    connect(changePasswordAction, &QAction::triggered, this, &CustomerDashboard::onChangePasswordClicked);
    
    QAction* changePinAction = accountMenu->addAction("Change Transaction &PIN");
    connect(changePinAction, &QAction::triggered, this, &CustomerDashboard::onChangePinClicked);
    
    // Transaction menu
    transactionMenu = menuBar->addMenu("&Transactions");
    QAction* historyAction = transactionMenu->addAction("&Transaction History");
    historyAction->setShortcut(QKeySequence("Ctrl+H"));
    connect(historyAction, &QAction::triggered, this, &CustomerDashboard::onHistoryClicked);
    
    // Security menu
    securityMenu = menuBar->addMenu("&Security");
    QAction* sessionStatusAction = securityMenu->addAction("Session Status");
    connect(sessionStatusAction, &QAction::triggered, this, [this]() {
        QMessageBox::information(this, "Session Status", 
                                 QString("Session Active: %1\nSession Duration: %2 minutes")
                                 .arg(authManager->isSessionValid() ? "Yes" : "No")
                                 .arg(authManager->getSessionDurationMinutes()));
    });
    
    // Help menu
    helpMenu = menuBar->addMenu("&Help");
    QAction* aboutAction = helpMenu->addAction("&About");
    connect(aboutAction, &QAction::triggered, this, &CustomerDashboard::onAbout);
    
    QAction* helpAction = helpMenu->addAction("&Help");
    helpAction->setShortcut(QKeySequence(Qt::Key_F1));
    connect(helpAction, &QAction::triggered, this, &CustomerDashboard::onHelp);
    
    setMenuBar(menuBar);
}

void CustomerDashboard::setupStatusBar()
{
    statusBar = new QStatusBar(this);
    
    sessionStatusLabel = new QLabel("Session: Active");
    securityStatusLabel = new QLabel("Security: Normal");
    
    statusBar->addPermanentWidget(sessionStatusLabel);
    statusBar->addPermanentWidget(securityStatusLabel);
    
    setStatusBar(statusBar);
}

void CustomerDashboard::setupLeftPanel()
{
    leftPanel = new QFrame();
    leftPanel->setObjectName("leftPanel");
    leftPanel->setMaximumWidth(600);
    
    leftPanelLayout = new QVBoxLayout(leftPanel);
    leftPanelLayout->setSpacing(15);
    leftPanelLayout->setContentsMargins(20, 20, 20, 20);
    
    // User info section
    userInfoFrame = new QFrame();
    userInfoFrame->setObjectName("infoFrame");
    QVBoxLayout* userInfoLayout = new QVBoxLayout(userInfoFrame);
    
    welcomeLabel = new QLabel("Welcome back!");
    welcomeLabel->setObjectName("welcomeLabel");
    
    usernameLabel = new QLabel();
    usernameLabel->setObjectName("infoLabel");
    
    userIdLabel = new QLabel();
    userIdLabel->setObjectName("infoLabel");
    
    lastLoginLabel = new QLabel();
    lastLoginLabel->setObjectName("infoLabel");
    
    userInfoLayout->addWidget(welcomeLabel);
    userInfoLayout->addWidget(usernameLabel);
    userInfoLayout->addWidget(userIdLabel);
    userInfoLayout->addWidget(lastLoginLabel);
    
    // Account summary section
    accountSummaryFrame = new QFrame();
    accountSummaryFrame->setObjectName("summaryFrame");
    QVBoxLayout* summaryLayout = new QVBoxLayout(accountSummaryFrame);
    
    totalBalanceLabel = new QLabel("Total Balance");
    totalBalanceLabel->setObjectName("summaryTitleLabel");
    
    totalBalanceValue = new QLabel("$0.00");
    totalBalanceValue->setObjectName("balanceValueLabel");
    
    balanceProgressBar = new QProgressBar();
    balanceProgressBar->setObjectName("balanceProgressBar");
    balanceProgressBar->setRange(0, 100000); // Max $100,000 for visual representation
    balanceProgressBar->setTextVisible(false);
    
    accountsTable = new QTableWidget();
    accountsTable->setObjectName("accountsTable");
    accountsTable->setMaximumHeight(200);
    
    summaryLayout->addWidget(totalBalanceLabel);
    summaryLayout->addWidget(totalBalanceValue);
    summaryLayout->addWidget(balanceProgressBar);
    summaryLayout->addWidget(new QLabel("Your Accounts:"));
    summaryLayout->addWidget(accountsTable);
    
    // Quick actions section
    quickActionsFrame = new QFrame();
    quickActionsFrame->setObjectName("actionsFrame");
    QGridLayout* actionsLayout = new QGridLayout(quickActionsFrame);
    
    depositButton = new QPushButton("💰 Deposit");
    depositButton->setObjectName("actionButton");
    connect(depositButton, &QPushButton::clicked, this, &CustomerDashboard::onDepositClicked);
    
    withdrawButton = new QPushButton("💸 Withdraw");
    withdrawButton->setObjectName("actionButton");
    connect(withdrawButton, &QPushButton::clicked, this, &CustomerDashboard::onWithdrawClicked);
    
    transferButton = new QPushButton("🔄 Transfer");
    transferButton->setObjectName("actionButton");
    connect(transferButton, &QPushButton::clicked, this, &CustomerDashboard::onTransferClicked);
    
    historyButton = new QPushButton("📊 History");
    historyButton->setObjectName("actionButton");
    connect(historyButton, &QPushButton::clicked, this, &CustomerDashboard::onHistoryClicked);
    
    changePasswordButton = new QPushButton("🔐 Password");
    changePasswordButton->setObjectName("secondaryActionButton");
    connect(changePasswordButton, &QPushButton::clicked, this, &CustomerDashboard::onChangePasswordClicked);
    
    changePinButton = new QPushButton("🔑 PIN");
    changePinButton->setObjectName("secondaryActionButton");
    connect(changePinButton, &QPushButton::clicked, this, &CustomerDashboard::onChangePinClicked);
    
    actionsLayout->addWidget(depositButton, 0, 0);
    actionsLayout->addWidget(withdrawButton, 0, 1);
    actionsLayout->addWidget(transferButton, 1, 0);
    actionsLayout->addWidget(historyButton, 1, 1);
    actionsLayout->addWidget(changePasswordButton, 2, 0);
    actionsLayout->addWidget(changePinButton, 2, 1);
    
    // Add all sections to left panel
    leftPanelLayout->addWidget(userInfoFrame);
    leftPanelLayout->addWidget(accountSummaryFrame);
    leftPanelLayout->addWidget(quickActionsFrame);
    leftPanelLayout->addStretch();
}

void CustomerDashboard::setupRightPanel()
{
    rightPanel = new QFrame();
    rightPanel->setObjectName("rightPanel");
    
    rightPanelLayout = new QVBoxLayout(rightPanel);
    rightPanelLayout->setSpacing(15);
    rightPanelLayout->setContentsMargins(20, 20, 20, 20);
    
    // Recent transactions section
    recentTransactionsFrame = new QFrame();
    recentTransactionsFrame->setObjectName("transactionsFrame");
    QVBoxLayout* transactionsLayout = new QVBoxLayout(recentTransactionsFrame);
    
    QLabel* transactionsTitle = new QLabel("Recent Transactions");
    transactionsTitle->setObjectName("sectionTitleLabel");
    
    recentTransactionsTable = new QTableWidget();
    recentTransactionsTable->setObjectName("transactionsTable");
    recentTransactionsTable->setMaximumHeight(300);
    
    viewAllTransactionsButton = new QPushButton("View All Transactions");
    viewAllTransactionsButton->setObjectName("viewAllButton");
    connect(viewAllTransactionsButton, &QPushButton::clicked, this, &CustomerDashboard::onViewAllTransactionsClicked);
    
    transactionsLayout->addWidget(transactionsTitle);
    transactionsLayout->addWidget(recentTransactionsTable);
    transactionsLayout->addWidget(viewAllTransactionsButton);
    
    // Notifications section
    notificationsFrame = new QFrame();
    notificationsFrame->setObjectName("notificationsFrame");
    QVBoxLayout* notificationsLayout = new QVBoxLayout(notificationsFrame);
    
    notificationsLabel = new QLabel("Notifications");
    notificationsLabel->setObjectName("sectionTitleLabel");
    
    notificationsContent = new QLabel("No new notifications");
    notificationsContent->setObjectName("notificationsContent");
    notificationsContent->setWordWrap(true);
    notificationsContent->setAlignment(Qt::AlignTop);
    
    clearNotificationsButton = new QPushButton("Clear Notifications");
    clearNotificationsButton->setObjectName("clearButton");
    connect(clearNotificationsButton, &QPushButton::clicked, this, &CustomerDashboard::onClearNotificationsClicked);
    
    notificationsLayout->addWidget(notificationsLabel);
    notificationsLayout->addWidget(notificationsContent);
    notificationsLayout->addWidget(clearNotificationsButton);
    
    // Add sections to right panel
    rightPanelLayout->addWidget(recentTransactionsFrame);
    rightPanelLayout->addWidget(notificationsFrame);
    rightPanelLayout->addStretch();
}

void CustomerDashboard::setupStyles()
{
    QString styleSheet = R"(
        /* Main window */
        CustomerDashboard {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #f5f5f5, stop:1 #e8eaf6);
        }
        
        /* Panels */
        QFrame#leftPanel, QFrame#rightPanel {
            background: white;
            border-radius: 12px;
            border: 1px solid #e0e0e0;
        }
        
        /* Info frames */
        QFrame#infoFrame, QFrame#summaryFrame, QFrame#actionsFrame, 
        QFrame#transactionsFrame, QFrame#notificationsFrame {
            background: #fafafa;
            border-radius: 8px;
            border: 1px solid #e0e0e0;
            padding: 15px;
        }
        
        /* Labels */
        QLabel#welcomeLabel {
            font-size: 18px;
            font-weight: bold;
            color: #1a237e;
            margin-bottom: 10px;
        }
        
        QLabel#infoLabel {
            font-size: 12px;
            color: #546e7a;
            margin: 2px 0;
        }
        
        QLabel#summaryTitleLabel, QLabel#sectionTitleLabel {
            font-size: 14px;
            font-weight: bold;
            color: #37474f;
            margin-bottom: 10px;
        }
        
        QLabel#balanceValueLabel {
            font-size: 24px;
            font-weight: bold;
            color: #2e7d32;
            margin: 5px 0;
        }
        
        QLabel#notificationsContent {
            font-size: 12px;
            color: #546e7a;
            background: white;
            border: 1px solid #e0e0e0;
            border-radius: 6px;
            padding: 10px;
            min-height: 100px;
        }
        
        /* Tables */
        QTableWidget#accountsTable, QTableWidget#transactionsTable {
            background: white;
            border: 1px solid #e0e0e0;
            border-radius: 6px;
            gridline-color: #f0f0f0;
            selection-background-color: #e3f2fd;
        }
        
        QTableWidget#accountsTable::item, QTableWidget#transactionsTable::item {
            padding: 8px;
            border-bottom: 1px solid #f0f0f0;
        }
        
        QTableWidget#accountsTable::item:selected, QTableWidget#transactionsTable::item:selected {
            background: #e3f2fd;
            color: #1565c0;
        }
        
        /* Progress bar */
        QProgressBar#balanceProgressBar {
            border: none;
            background: #f0f0f0;
            border-radius: 10px;
            height: 8px;
        }
        
        QProgressBar#balanceProgressBar::chunk {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #4caf50, stop:0.5 #66bb6a, stop:1 #81c784);
            border-radius: 10px;
        }
        
        /* Buttons */
        QPushButton#actionButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #1a237e, stop:1 #283593);
            color: white;
            border: none;
            border-radius: 8px;
            padding: 12px;
            font-size: 12px;
            font-weight: 600;
            min-height: 40px;
        }
        
        QPushButton#actionButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #283593, stop:1 #3949ab);
        }
        
        QPushButton#secondaryActionButton {
            background: #78909c;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 10px;
            font-size: 11px;
            font-weight: 600;
            min-height: 35px;
        }
        
        QPushButton#secondaryActionButton:hover {
            background: #607d8b;
        }
        
        QPushButton#viewAllButton, QPushButton#clearButton {
            background: #5c6bc0;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 8px 16px;
            font-size: 11px;
        }
        
        QPushButton#viewAllButton:hover, QPushButton#clearButton:hover {
            background: #3f51b5;
        }
        
        /* Menu bar */
        QMenuBar {
            background: #1a237e;
            color: white;
            border: none;
        }
        
        QMenuBar::item {
            background: transparent;
            padding: 8px 16px;
        }
        
        QMenuBar::item:selected {
            background: #283593;
        }
        
        QMenu {
            background: white;
            border: 1px solid #e0e0e0;
            border-radius: 4px;
        }
        
        QMenu::item {
            padding: 8px 20px;
        }
        
        QMenu::item:selected {
            background: #e3f2fd;
        }
        
        /* Status bar */
        QStatusBar {
            background: #37474f;
            color: white;
            border: none;
        }
    )";
    
    setStyleSheet(styleSheet);
    applyModernStyling();
}

void CustomerDashboard::applyModernStyling()
{
    // Add drop shadow effects to frames
    QList<QFrame*> frames = {userInfoFrame, accountSummaryFrame, quickActionsFrame, 
                            recentTransactionsFrame, notificationsFrame};
    
    for (QFrame* frame : frames) {
        if (frame) {
            QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
            shadow->setBlurRadius(10);
            shadow->setColor(QColor(0, 0, 0, 20));
            shadow->setOffset(0, 2);
            frame->setGraphicsEffect(shadow);
        }
    }
}

void CustomerDashboard::loadUserData()
{
    updateWelcomeMessage();
    loadAccountSummary();
    loadRecentTransactions();
    loadNotifications();
    updateSessionStatus();
    updateSecurityStatus();
    isDataLoaded = true;
}

void CustomerDashboard::updateWelcomeMessage()
{
    welcomeLabel->setText(QString("Welcome back, %1!").arg(currentUsername));
    usernameLabel->setText(QString("Username: %1").arg(currentUsername));
    userIdLabel->setText(QString("User ID: %1").arg(currentUserId));
    
    QDateTime now = QDateTime::currentDateTime();
    lastLoginLabel->setText(QString("Last login: %1").arg(now.toString("yyyy-MM-dd hh:mm:ss")));
}

void CustomerDashboard::loadAccountSummary()
{
    loadAccounts();
    updateAccountDisplay();
    
    double totalBalance = getTotalBalance();
    totalBalanceValue->setText(QString("$%1").arg(QLocale().toString(totalBalance, 'f', 2)));
    balanceProgressBar->setValue(static_cast<int>(totalBalance));
}

void CustomerDashboard::loadAccounts()
{
    auto accounts = bankService->getUserAccounts(currentUserId);
    
    accountsTable->setRowCount(accounts.size());
    accountsTable->setColumnCount(4);
    accountsTable->setHorizontalHeaderLabels({"Account Number", "Type", "Balance", "Status"});
    
    accountsTable->horizontalHeader()->setStretchLastSection(true);
    accountsTable->verticalHeader()->setVisible(false);
    accountsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    
    for (int i = 0; i < accounts.size(); ++i) {
        auto account = accounts[i];
        
        accountsTable->setItem(i, 0, new QTableWidgetItem(account->getAccountNumber()));
        accountsTable->setItem(i, 1, new QTableWidgetItem(account->getAccountTypeDisplay()));
        accountsTable->setItem(i, 2, new QTableWidgetItem(QString("$%1").arg(QLocale().toString(account->getBalance(), 'f', 2))));
        accountsTable->setItem(i, 3, new QTableWidgetItem(account->isActiveStatus() ? "Active" : "Inactive"));
    }
    
    connect(accountsTable, &QTableWidget::cellDoubleClicked, this, &CustomerDashboard::onAccountDoubleClicked);
}

void CustomerDashboard::updateAccountDisplay()
{
    // Additional account display updates can be added here
}

double CustomerDashboard::getTotalBalance()
{
    double total = 0.0;
    auto accounts = bankService->getUserAccounts(currentUserId);
    
    for (const auto& account : accounts) {
        if (account->isActiveStatus()) {
            total += account->getBalance();
        }
    }
    
    return total;
}

void CustomerDashboard::loadRecentTransactions()
{
    formatTransactionTable();
    loadRecentTransactionsData();
}

void CustomerDashboard::formatTransactionTable()
{
    recentTransactionsTable->setRowCount(0);
    recentTransactionsTable->setColumnCount(5);
    recentTransactionsTable->setHorizontalHeaderLabels({"Date", "Type", "Amount", "Account", "Status"});
    
    recentTransactionsTable->horizontalHeader()->setStretchLastSection(true);
    recentTransactionsTable->verticalHeader()->setVisible(false);
    recentTransactionsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    recentTransactionsTable->setAlternatingRowColors(true);
    
    // Set column widths
    recentTransactionsTable->setColumnWidth(0, 120); // Date
    recentTransactionsTable->setColumnWidth(1, 80);   // Type
    recentTransactionsTable->setColumnWidth(2, 100);  // Amount
    recentTransactionsTable->setColumnWidth(3, 120);  // Account
    
    connect(recentTransactionsTable, &QTableWidget::cellDoubleClicked, this, &CustomerDashboard::onTransactionDoubleClicked);
}

void CustomerDashboard::loadRecentTransactionsData()
{
    auto accounts = bankService->getUserAccounts(currentUserId);
    QList<std::shared_ptr<Transaction>> allTransactions;
    
    // Get transactions from all accounts
    for (const auto& account : accounts) {
        auto transactions = bankService->getAccountTransactions(account->getAccountId());
        allTransactions.append(transactions);
    }
    
    // Sort by date (most recent first)
    std::sort(allTransactions.begin(), allTransactions.end(), 
              [](const std::shared_ptr<Transaction>& a, const std::shared_ptr<Transaction>& b) {
                  return a->getTransactionDate() > b->getTransactionDate();
              });
    
    // Display only the 10 most recent transactions
    int displayCount = qMin(10, allTransactions.size());
    recentTransactionsTable->setRowCount(displayCount);
    
    for (int i = 0; i < displayCount; ++i) {
        auto transaction = allTransactions[i];
        
        recentTransactionsTable->setItem(i, 0, new QTableWidgetItem(transaction->getFormattedDate()));
        recentTransactionsTable->setItem(i, 1, new QTableWidgetItem(transaction->getTransactionTypeDisplay()));
        recentTransactionsTable->setItem(i, 2, new QTableWidgetItem(QString("$%1").arg(QLocale().toString(transaction->getAmount(), 'f', 2))));
        
        // Find the account number
        QString accountNumber = "Unknown";
        for (const auto& account : accounts) {
            if (account->getAccountId() == transaction->getFromAccountId() || 
                account->getAccountId() == transaction->getToAccountId()) {
                accountNumber = account->getAccountNumber();
                break;
            }
        }
        
        recentTransactionsTable->setItem(i, 3, new QTableWidgetItem(accountNumber));
        recentTransactionsTable->setItem(i, 4, new QTableWidgetItem(transaction->getStatusDisplay()));
    }
}

void CustomerDashboard::loadNotifications()
{
    // For now, add some sample notifications
    addNotification("Welcome to the Banking Management System!");
    addNotification("Your account is secure and ready to use.");
}

void CustomerDashboard::addNotification(const QString& message)
{
    QString currentText = notificationsContent->text();
    if (currentText == "No new notifications") {
        currentText = "";
    }
    
    QDateTime timestamp = QDateTime::currentDateTime();
    QString formattedMessage = QString("[%1] %2\n")
                              .arg(timestamp.toString("hh:mm:ss"))
                              .arg(message);
    
    notificationsContent->setText(currentText + formattedMessage);
}

void CustomerDashboard::clearNotifications()
{
    notificationsContent->setText("No new notifications");
}

void CustomerDashboard::updateSessionStatus()
{
    if (authManager->isSessionValid()) {
        sessionStatusLabel->setText("Session: Active");
        sessionStatusLabel->setStyleSheet("color: #4caf50;");
    } else {
        sessionStatusLabel->setText("Session: Expired");
        sessionStatusLabel->setStyleSheet("color: #f44336;");
    }
}

void CustomerDashboard::updateSecurityStatus()
{
    if (authManager->isLoggedIn()) {
        securityStatusLabel->setText("Security: Normal");
        securityStatusLabel->setStyleSheet("color: #4caf50;");
    } else {
        securityStatusLabel->setText("Security: Alert");
        securityStatusLabel->setStyleSheet("color: #ff9800;");
    }
}

void CustomerDashboard::refreshAllData()
{
    loadAccountSummary();
    loadRecentTransactions();
    updateSessionStatus();
    updateSecurityStatus();
}

void CustomerDashboard::showDepositDialog()
{
    DepositWindow dialog(currentUserId, this);
    dialog.setModal(true);
    
    if (dialog.exec() == QDialog::Accepted) {
        loadAccountSummary();
        loadRecentTransactions();
        addNotification("Deposit completed successfully");
    }
}

void CustomerDashboard::showWithdrawalDialog()
{
    WithdrawalWindow dialog(currentUserId, this);
    dialog.setModal(true);
    
    if (dialog.exec() == QDialog::Accepted) {
        loadAccountSummary();
        loadRecentTransactions();
        addNotification("Withdrawal completed successfully");
    }
}

void CustomerDashboard::showTransferDialog()
{
    TransferWindow dialog(currentUserId, this);
    dialog.setModal(true);
    
    if (dialog.exec() == QDialog::Accepted) {
        loadAccountSummary();
        loadRecentTransactions();
        addNotification("Transfer completed successfully");
    }
}

void CustomerDashboard::showTransactionHistory()
{
    TransactionHistoryWindow dialog(currentUserId, this);
    dialog.setModal(true);
    dialog.exec();
}

void CustomerDashboard::showChangePasswordDialog()
{
    bool ok;
    QString oldPassword = QInputDialog::getText(this, "Change Password", 
                                               "Enter current password:", 
                                               QLineEdit::Password, "", &ok);
    
    if (!ok || oldPassword.isEmpty()) return;
    
    QString newPassword = QInputDialog::getText(this, "Change Password", 
                                               "Enter new password:", 
                                               QLineEdit::Password, "", &ok);
    
    if (!ok || newPassword.isEmpty()) return;
    
    QString confirmPassword = QInputDialog::getText(this, "Change Password", 
                                                  "Confirm new password:", 
                                                  QLineEdit::Password, "", &ok);
    
    if (!ok || confirmPassword.isEmpty()) return;
    
    if (newPassword != confirmPassword) {
        QMessageBox::warning(this, "Error", "Passwords do not match!");
        return;
    }
    
    if (bankService->changePassword(oldPassword, newPassword)) {
        QMessageBox::information(this, "Success", "Password changed successfully!");
        addNotification("Password changed successfully");
    } else {
        QMessageBox::warning(this, "Error", "Failed to change password. Please check your current password.");
    }
}

void CustomerDashboard::showChangePinDialog()
{
    bool ok;
    QString oldPin = QInputDialog::getText(this, "Change Transaction PIN", 
                                          "Enter current PIN:", 
                                          QLineEdit::Password, "", &ok);
    
    if (!ok || oldPin.isEmpty()) return;
    
    QString newPin = QInputDialog::getText(this, "Change Transaction PIN", 
                                          "Enter new 4-digit PIN:", 
                                          QLineEdit::Password, "", &ok);
    
    if (!ok || newPin.isEmpty()) return;
    
    QString confirmPin = QInputDialog::getText(this, "Change Transaction PIN", 
                                             "Confirm new PIN:", 
                                             QLineEdit::Password, "", &ok);
    
    if (!ok || confirmPin.isEmpty()) return;
    
    if (newPin != confirmPin) {
        QMessageBox::warning(this, "Error", "PINs do not match!");
        return;
    }
    
    if (bankService->changeTransactionPin(oldPin, newPin)) {
        QMessageBox::information(this, "Success", "Transaction PIN changed successfully!");
        addNotification("Transaction PIN changed successfully");
    } else {
        QMessageBox::warning(this, "Error", "Failed to change PIN. Please check your current PIN.");
    }
}

void CustomerDashboard::showStatusMessage(const QString& message, bool isError)
{
    if (isError) {
        statusBar->showMessage(message, 5000);
    } else {
        statusBar->showMessage(message, 3000);
    }
}

void CustomerDashboard::closeEvent(QCloseEvent* event)
{
    emit logoutRequested();
    QMainWindow::closeEvent(event);
}

void CustomerDashboard::keyPressEvent(QKeyEvent* event)
{
    // Handle keyboard shortcuts
    if (event->key() == Qt::Key_F5) {
        onRefreshData();
    } else if (event->key() == Qt::Key_L && event->modifiers() & Qt::ControlModifier) {
        onLogout();
    }
    QMainWindow::keyPressEvent(event);
}

// Slot implementations
void CustomerDashboard::onDepositClicked()
{
    showDepositDialog();
}

void CustomerDashboard::onWithdrawClicked()
{
    showWithdrawalDialog();
}

void CustomerDashboard::onTransferClicked()
{
    showTransferDialog();
}

void CustomerDashboard::onHistoryClicked()
{
    showTransactionHistory();
}

void CustomerDashboard::onChangePasswordClicked()
{
    showChangePasswordDialog();
}

void CustomerDashboard::onChangePinClicked()
{
    showChangePinDialog();
}

void CustomerDashboard::onViewAllTransactionsClicked()
{
    showTransactionHistory();
}

void CustomerDashboard::onClearNotificationsClicked()
{
    clearNotifications();
}

void CustomerDashboard::onUpdateTimer()
{
    updateSessionStatus();
    updateSecurityStatus();
    
    // Check if session has expired
    if (!authManager->isSessionValid()) {
        emit sessionExpired();
        QMessageBox::warning(this, "Session Expired", 
                             "Your session has expired due to inactivity. Please login again.");
        onLogout();
    }
}

void CustomerDashboard::onAccountDoubleClicked(int row, int column)
{
    Q_UNUSED(column);
    
    QString accountNumber = accountsTable->item(row, 0)->text();
    QMessageBox::information(this, "Account Details", 
                             QString("Account Number: %1\n\n"
                                     "Double-clicked account details would be shown here.")
                             .arg(accountNumber));
}

void CustomerDashboard::onTransactionDoubleClicked(int row, int column)
{
    Q_UNUSED(column);
    
    QString date = recentTransactionsTable->item(row, 0)->text();
    QString type = recentTransactionsTable->item(row, 1)->text();
    QString amount = recentTransactionsTable->item(row, 2)->text();
    
    QMessageBox::information(this, "Transaction Details", 
                             QString("Transaction Details:\n\n"
                                     "Date: %1\n"
                                     "Type: %2\n"
                                     "Amount: %3\n\n"
                                     "Full transaction details would be shown here.")
                             .arg(date).arg(type).arg(amount));
}

void CustomerDashboard::onLogout()
{
    emit logoutRequested();
}

void CustomerDashboard::onExit()
{
    close();
}

void CustomerDashboard::onRefreshData()
{
    refreshAllData();
    showStatusMessage("Data refreshed", false);
}

void CustomerDashboard::onAbout()
{
    QMessageBox::about(this, "About Banking Management System", 
                       "Banking Management System v1.0.0\n\n"
                       "A secure and modern banking application\n"
                       "developed with Qt and C++.\n\n"
                       "© 2026 Banking System");
}

void CustomerDashboard::onHelp()
{
    QMessageBox::information(this, "Help", 
                             "Banking Management System Help\n\n"
                             "• Use the menu bar or quick action buttons for banking operations\n"
                             "• Double-click on accounts or transactions for details\n"
                             "• Press F5 to refresh data\n"
                             "• Press Ctrl+L to logout\n\n"
                             "For technical support, contact your system administrator.");
}

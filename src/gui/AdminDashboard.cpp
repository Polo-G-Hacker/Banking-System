#include "gui/AdminDashboard.h"
#include "core/BankService.h"
#include "security/AuthManager.h"
#include "security/SecurityManager.h"
#include "gui/AdminManagementWindow.h"
#include "gui/ReportsWindow.h"
#include <QApplication>
#include <QFileDialog>
#include <QGuiApplication>
#include <QInputDialog>
#include <QMessageBox>
#include <QKeyEvent>
#include <QHeaderView>
#include <QDateTime>
#include <QLocale>
#include <QScreen>
#include <QSplitter>
#include <QGraphicsDropShadowEffect>

AdminDashboard::AdminDashboard(int userId, const QString& username, QWidget *parent)
    : QMainWindow(parent)
    , menuBar(nullptr)
    , statusBar(nullptr)
    , fileMenu(nullptr)
    , userMenu(nullptr)
    , accountMenu(nullptr)
    , reportMenu(nullptr)
    , securityMenu(nullptr)
    , helpMenu(nullptr)
    , centralWidget(nullptr)
    , mainSplitter(nullptr)
    , leftPanel(nullptr)
    , leftScrollArea(nullptr)
    , leftPanelContent(nullptr)
    , leftPanelLayout(nullptr)
    , adminInfoFrame(nullptr)
    , welcomeLabel(nullptr)
    , adminNameLabel(nullptr)
    , adminRoleLabel(nullptr)
    , lastLoginLabel(nullptr)
    , statisticsFrame(nullptr)
    , statisticsTitle(nullptr)
    , statsLayout(nullptr)
    , totalCustomersLabel(nullptr)
    , totalCustomersValue(nullptr)
    , activeCustomersLabel(nullptr)
    , activeCustomersValue(nullptr)
    , totalAccountsLabel(nullptr)
    , totalAccountsValue(nullptr)
    , totalDepositsLabel(nullptr)
    , totalDepositsValue(nullptr)
    , todayTransactionsLabel(nullptr)
    , todayTransactionsValue(nullptr)
    , quickActionsFrame(nullptr)
    , manageCustomersButton(nullptr)
    , manageAccountsButton(nullptr)
    , viewReportsButton(nullptr)
    , createCustomerButton(nullptr)
    , backupDatabaseButton(nullptr)
    , viewLogsButton(nullptr)
    , rightPanel(nullptr)
    , rightScrollArea(nullptr)
    , rightPanelContent(nullptr)
    , rightPanelLayout(nullptr)
    , recentActivitiesFrame(nullptr)
    , recentActivitiesTable(nullptr)
    , viewAllActivitiesButton(nullptr)
    , systemStatusFrame(nullptr)
    , systemStatusLabel(nullptr)
    , databaseStatusLabel(nullptr)
    , securityStatusLabel(nullptr)
    , sessionStatusLabel(nullptr)
    , systemHealthBar(nullptr)
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
    loadAdminData();
    
    // Setup update timer
    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &AdminDashboard::onUpdateTimer);
    updateTimer->start(30000); // Update every 30 seconds
    
    // Window properties
    setWindowTitle("Banking Management System - Admin Dashboard");
    setMinimumSize(1200, 800);
    resize(1400, 900);
    
    // Center window on screen
    if (QScreen* screen = QGuiApplication::primaryScreen()) {
        move(screen->availableGeometry().center() - rect().center());
    }
}

AdminDashboard::~AdminDashboard()
{
    if (updateTimer) {
        updateTimer->stop();
    }
}

void AdminDashboard::setupUI()
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

void AdminDashboard::setupMenuBar()
{
    menuBar = new QMenuBar(this);
    
    // File menu
    fileMenu = menuBar->addMenu("&File");
    QAction* refreshAction = fileMenu->addAction("&Refresh Data");
    refreshAction->setShortcut(QKeySequence::Refresh);
    connect(refreshAction, &QAction::triggered, this, &AdminDashboard::onRefreshData);
    
    fileMenu->addSeparator();
    QAction* backupAction = fileMenu->addAction("&Backup Database");
    connect(backupAction, &QAction::triggered, this, &AdminDashboard::onBackupDatabaseClicked);
    
    fileMenu->addSeparator();
    QAction* logoutAction = fileMenu->addAction("&Logout");
    logoutAction->setShortcut(QKeySequence("Ctrl+L"));
    connect(logoutAction, &QAction::triggered, this, &AdminDashboard::onLogout);
    
    QAction* exitAction = fileMenu->addAction("E&xit");
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &AdminDashboard::onExit);
    
    // User management menu
    userMenu = menuBar->addMenu("&User Management");
    QAction* manageCustomersAction = userMenu->addAction("&Manage Customers");
    manageCustomersAction->setShortcut(QKeySequence("Ctrl+C"));
    connect(manageCustomersAction, &QAction::triggered, this, &AdminDashboard::onManageCustomersClicked);
    
    QAction* createCustomerAction = userMenu->addAction("&Create New Customer");
    createCustomerAction->setShortcut(QKeySequence("Ctrl+N"));
    connect(createCustomerAction, &QAction::triggered, this, &AdminDashboard::onCreateCustomerClicked);
    
    // Account management menu
    accountMenu = menuBar->addMenu("&Account Management");
    QAction* manageAccountsAction = accountMenu->addAction("&Manage Accounts");
    manageAccountsAction->setShortcut(QKeySequence("Ctrl+A"));
    connect(manageAccountsAction, &QAction::triggered, this, &AdminDashboard::onManageAccountsClicked);
    
    // Report menu
    reportMenu = menuBar->addMenu("&Reports");
    QAction* viewReportsAction = reportMenu->addAction("&View Reports");
    viewReportsAction->setShortcut(QKeySequence("Ctrl+R"));
    connect(viewReportsAction, &QAction::triggered, this, &AdminDashboard::onViewReportsClicked);
    
    // Security menu
    securityMenu = menuBar->addMenu("&Security");
    QAction* viewLogsAction = securityMenu->addAction("&View System Logs");
    viewLogsAction->setShortcut(QKeySequence("Ctrl+L"));
    connect(viewLogsAction, &QAction::triggered, this, &AdminDashboard::onViewLogsClicked);
    
    QAction* sessionStatusAction = securityMenu->addAction("Session &Status");
    connect(sessionStatusAction, &QAction::triggered, this, [this]() {
        QMessageBox::information(this, "Session Status", 
                                 QString("Session Active: %1\nSession Duration: %2 minutes")
                                 .arg(authManager->isSessionValid() ? "Yes" : "No")
                                 .arg(authManager->getSessionDurationMinutes()));
    });
    
    // Help menu
    helpMenu = menuBar->addMenu("&Help");
    QAction* aboutAction = helpMenu->addAction("&About");
    connect(aboutAction, &QAction::triggered, this, &AdminDashboard::onAbout);
    
    QAction* helpAction = helpMenu->addAction("&Help");
    helpAction->setShortcut(QKeySequence(Qt::Key_F1));
    connect(helpAction, &QAction::triggered, this, &AdminDashboard::onHelp);
    
    setMenuBar(menuBar);
}

void AdminDashboard::setupStatusBar()
{
    statusBar = new QStatusBar(this);
    
    sessionStatusLabel = new QLabel("Session: Active");
    systemStatusLabel = new QLabel("System: Normal");
    securityStatusLabel = new QLabel("Security: Normal");
    
    statusBar->addPermanentWidget(sessionStatusLabel);
    statusBar->addPermanentWidget(systemStatusLabel);
    statusBar->addPermanentWidget(securityStatusLabel);
    
    setStatusBar(statusBar);
}

void AdminDashboard::setupLeftPanel()
{
    leftPanel = new QFrame();
    leftPanel->setObjectName("leftPanel");
    leftPanel->setMaximumWidth(600);
    
    leftPanelLayout = new QVBoxLayout(leftPanel);
    leftPanelLayout->setSpacing(15);
    leftPanelLayout->setContentsMargins(20, 20, 20, 20);
    
    // Admin info section
    adminInfoFrame = new QFrame();
    adminInfoFrame->setObjectName("infoFrame");
    QVBoxLayout* adminInfoLayout = new QVBoxLayout(adminInfoFrame);
    
    welcomeLabel = new QLabel("Welcome, Administrator!");
    welcomeLabel->setObjectName("welcomeLabel");
    
    adminNameLabel = new QLabel();
    adminNameLabel->setObjectName("infoLabel");
    
    adminRoleLabel = new QLabel();
    adminRoleLabel->setObjectName("infoLabel");
    
    lastLoginLabel = new QLabel();
    lastLoginLabel->setObjectName("infoLabel");
    
    adminInfoLayout->addWidget(welcomeLabel);
    adminInfoLayout->addWidget(adminNameLabel);
    adminInfoLayout->addWidget(adminRoleLabel);
    adminInfoLayout->addWidget(lastLoginLabel);
    
    // Statistics section
    statisticsFrame = new QFrame();
    statisticsFrame->setObjectName("statsFrame");
    QVBoxLayout* statsFrameLayout = new QVBoxLayout(statisticsFrame);
    
    statisticsTitle = new QLabel("System Statistics");
    statisticsTitle->setObjectName("sectionTitleLabel");
    
    statsLayout = new QGridLayout();
    statsLayout->setVerticalSpacing(10);
    statsLayout->setHorizontalSpacing(20);
    
    totalCustomersLabel = new QLabel("Total Customers:");
    totalCustomersLabel->setObjectName("statLabel");
    totalCustomersValue = new QLabel("0");
    totalCustomersValue->setObjectName("statValue");
    
    activeCustomersLabel = new QLabel("Active Customers:");
    activeCustomersLabel->setObjectName("statLabel");
    activeCustomersValue = new QLabel("0");
    activeCustomersValue->setObjectName("statValue");
    
    totalAccountsLabel = new QLabel("Total Accounts:");
    totalAccountsLabel->setObjectName("statLabel");
    totalAccountsValue = new QLabel("0");
    totalAccountsValue->setObjectName("statValue");
    
    totalDepositsLabel = new QLabel("Total Deposits:");
    totalDepositsLabel->setObjectName("statLabel");
    totalDepositsValue = new QLabel("$0.00");
    totalDepositsValue->setObjectName("statValue");
    
    todayTransactionsLabel = new QLabel("Today's Transactions:");
    todayTransactionsLabel->setObjectName("statLabel");
    todayTransactionsValue = new QLabel("0");
    todayTransactionsValue->setObjectName("statValue");
    
    statsLayout->addWidget(totalCustomersLabel, 0, 0);
    statsLayout->addWidget(totalCustomersValue, 0, 1);
    statsLayout->addWidget(activeCustomersLabel, 0, 2);
    statsLayout->addWidget(activeCustomersValue, 0, 3);
    statsLayout->addWidget(totalAccountsLabel, 1, 0);
    statsLayout->addWidget(totalAccountsValue, 1, 1);
    statsLayout->addWidget(totalDepositsLabel, 1, 2);
    statsLayout->addWidget(totalDepositsValue, 1, 3);
    statsLayout->addWidget(todayTransactionsLabel, 2, 0, 1, 2);
    statsLayout->addWidget(todayTransactionsValue, 2, 2, 1, 2);
    
    // Quick actions section
    quickActionsFrame = new QFrame();
    quickActionsFrame->setObjectName("actionsFrame");
    QGridLayout* actionsLayout = new QGridLayout(quickActionsFrame);
    
    manageCustomersButton = new QPushButton("👥 Manage Customers");
    manageCustomersButton->setObjectName("primaryActionButton");
    connect(manageCustomersButton, &QPushButton::clicked, this, &AdminDashboard::onManageCustomersClicked);
    
    manageAccountsButton = new QPushButton("💳 Manage Accounts");
    manageAccountsButton->setObjectName("primaryActionButton");
    connect(manageAccountsButton, &QPushButton::clicked, this, &AdminDashboard::onManageAccountsClicked);
    
    viewReportsButton = new QPushButton("📊 View Reports");
    viewReportsButton->setObjectName("primaryActionButton");
    connect(viewReportsButton, &QPushButton::clicked, this, &AdminDashboard::onViewReportsClicked);
    
    createCustomerButton = new QPushButton("➕ Create Customer");
    createCustomerButton->setObjectName("secondaryActionButton");
    connect(createCustomerButton, &QPushButton::clicked, this, &AdminDashboard::onCreateCustomerClicked);
    
    backupDatabaseButton = new QPushButton("💾 Backup Database");
    backupDatabaseButton->setObjectName("secondaryActionButton");
    connect(backupDatabaseButton, &QPushButton::clicked, this, &AdminDashboard::onBackupDatabaseClicked);
    
    viewLogsButton = new QPushButton("📋 View Logs");
    viewLogsButton->setObjectName("secondaryActionButton");
    connect(viewLogsButton, &QPushButton::clicked, this, &AdminDashboard::onViewLogsClicked);
    
    actionsLayout->addWidget(manageCustomersButton, 0, 0);
    actionsLayout->addWidget(manageAccountsButton, 0, 1);
    actionsLayout->addWidget(viewReportsButton, 1, 0);
    actionsLayout->addWidget(createCustomerButton, 1, 1);
    actionsLayout->addWidget(backupDatabaseButton, 2, 0);
    actionsLayout->addWidget(viewLogsButton, 2, 1);
    
    // Add sections to left panel
    statsFrameLayout->addWidget(statisticsTitle);
    statsFrameLayout->addLayout(statsLayout);
    
    leftPanelLayout->addWidget(adminInfoFrame);
    leftPanelLayout->addWidget(statisticsFrame);
    leftPanelLayout->addWidget(quickActionsFrame);
    leftPanelLayout->addStretch();
}

void AdminDashboard::setupRightPanel()
{
    rightPanel = new QFrame();
    rightPanel->setObjectName("rightPanel");
    
    rightPanelLayout = new QVBoxLayout(rightPanel);
    rightPanelLayout->setSpacing(15);
    rightPanelLayout->setContentsMargins(20, 20, 20, 20);
    
    // Recent activities section
    recentActivitiesFrame = new QFrame();
    recentActivitiesFrame->setObjectName("activitiesFrame");
    QVBoxLayout* activitiesLayout = new QVBoxLayout(recentActivitiesFrame);
    
    QLabel* activitiesTitle = new QLabel("Recent System Activities");
    activitiesTitle->setObjectName("sectionTitleLabel");
    
    recentActivitiesTable = new QTableWidget();
    recentActivitiesTable->setObjectName("activitiesTable");
    recentActivitiesTable->setMaximumHeight(300);
    
    viewAllActivitiesButton = new QPushButton("View All Activities");
    viewAllActivitiesButton->setObjectName("viewAllButton");
    
    activitiesLayout->addWidget(activitiesTitle);
    activitiesLayout->addWidget(recentActivitiesTable);
    activitiesLayout->addWidget(viewAllActivitiesButton);
    
    // System status section
    systemStatusFrame = new QFrame();
    systemStatusFrame->setObjectName("statusFrame");
    QVBoxLayout* statusLayout = new QVBoxLayout(systemStatusFrame);
    
    QLabel* statusTitle = new QLabel("System Status");
    statusTitle->setObjectName("sectionTitleLabel");
    
    systemStatusLabel = new QLabel("System Health");
    systemStatusLabel->setObjectName("statusTitleLabel");
    
    databaseStatusLabel = new QLabel("Database: Connected");
    databaseStatusLabel->setObjectName("statusLabel");
    
    securityStatusLabel = new QLabel("Security: Normal");
    securityStatusLabel->setObjectName("statusLabel");
    
    sessionStatusLabel = new QLabel("Session: Active");
    sessionStatusLabel->setObjectName("statusLabel");
    
    systemHealthBar = new QProgressBar();
    systemHealthBar->setObjectName("healthBar");
    systemHealthBar->setRange(0, 100);
    systemHealthBar->setValue(95);
    systemHealthBar->setTextVisible(true);
    systemHealthBar->setFormat("System Health: %p%");
    
    statusLayout->addWidget(statusTitle);
    statusLayout->addWidget(systemStatusLabel);
    statusLayout->addWidget(databaseStatusLabel);
    statusLayout->addWidget(securityStatusLabel);
    statusLayout->addWidget(sessionStatusLabel);
    statusLayout->addWidget(systemHealthBar);
    
    // Add sections to right panel
    rightPanelLayout->addWidget(recentActivitiesFrame);
    rightPanelLayout->addWidget(systemStatusFrame);
    rightPanelLayout->addStretch();
}

void AdminDashboard::setupStyles()
{
    QString styleSheet = R"(
        /* Main window */
        AdminDashboard {
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
        QFrame#infoFrame, QFrame#statsFrame, QFrame#actionsFrame, 
        QFrame#activitiesFrame, QFrame#statusFrame {
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
        
        QLabel#sectionTitleLabel {
            font-size: 14px;
            font-weight: bold;
            color: #37474f;
            margin-bottom: 10px;
        }
        
        QLabel#statLabel {
            font-size: 12px;
            font-weight: 600;
            color: #546e7a;
        }
        
        QLabel#statValue {
            font-size: 16px;
            font-weight: bold;
            color: #1a237e;
        }
        
        QLabel#statusTitleLabel {
            font-size: 14px;
            font-weight: bold;
            color: #37474f;
            margin-bottom: 8px;
        }
        
        QLabel#statusLabel {
            font-size: 12px;
            color: #546e7a;
            margin: 3px 0;
        }
        
        /* Tables */
        QTableWidget#activitiesTable {
            background: white;
            border: 1px solid #e0e0e0;
            border-radius: 6px;
            gridline-color: #f0f0f0;
            selection-background-color: #e3f2fd;
        }
        
        QTableWidget#activitiesTable::item {
            padding: 6px;
            border-bottom: 1px solid #f0f0f0;
        }
        
        QTableWidget#activitiesTable::item:selected {
            background: #e3f2fd;
            color: #1565c0;
        }
        
        /* Progress bar */
        QProgressBar#healthBar {
            border: none;
            background: #f0f0f0;
            border-radius: 10px;
            height: 12px;
            text-align: center;
        }
        
        QProgressBar#healthBar::chunk {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #4caf50, stop:0.5 #8bc34a, stop:1 #cddc39);
            border-radius: 10px;
        }
        
        /* Buttons */
        QPushButton#primaryActionButton {
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
        
        QPushButton#primaryActionButton:hover {
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
        
        QPushButton#viewAllButton {
            background: #5c6bc0;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 8px 16px;
            font-size: 11px;
        }
        
        QPushButton#viewAllButton:hover {
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

void AdminDashboard::applyModernStyling()
{
    // Add drop shadow effects to frames
    QList<QFrame*> frames = {adminInfoFrame, statisticsFrame, quickActionsFrame, 
                            recentActivitiesFrame, systemStatusFrame};
    
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

void AdminDashboard::loadAdminData()
{
    updateWelcomeMessage();
    loadStatistics();
    loadRecentActivities();
    updateSystemStatus();
    isDataLoaded = true;
}

void AdminDashboard::updateWelcomeMessage()
{
    welcomeLabel->setText("Welcome, Administrator!");
    adminNameLabel->setText(QString("Admin: %1").arg(currentUsername));
    adminRoleLabel->setText("Role: System Administrator");
    
    QDateTime now = QDateTime::currentDateTime();
    lastLoginLabel->setText(QString("Last login: %1").arg(now.toString("yyyy-MM-dd hh:mm:ss")));
}

void AdminDashboard::loadStatistics()
{
    auto stats = bankService->getBankStatistics();
    
    totalCustomersValue->setText(QString::number(stats.totalCustomers));
    activeCustomersValue->setText(QString::number(stats.activeCustomers));
    totalAccountsValue->setText(QString::number(stats.totalAccounts));
    totalDepositsValue->setText(QString("$%1").arg(QLocale().toString(stats.totalDeposits, 'f', 2)));
    todayTransactionsValue->setText(QString::number(stats.todayTransactions));
}

void AdminDashboard::loadRecentActivities()
{
    formatActivitiesTable();
    loadRecentActivitiesData();
}

void AdminDashboard::formatActivitiesTable()
{
    recentActivitiesTable->setRowCount(0);
    recentActivitiesTable->setColumnCount(4);
    recentActivitiesTable->setHorizontalHeaderLabels({"Time", "User", "Action", "Status"});
    
    recentActivitiesTable->horizontalHeader()->setStretchLastSection(true);
    recentActivitiesTable->verticalHeader()->setVisible(false);
    recentActivitiesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    recentActivitiesTable->setAlternatingRowColors(true);
    
    // Set column widths
    recentActivitiesTable->setColumnWidth(0, 80);  // Time
    recentActivitiesTable->setColumnWidth(1, 100); // User
    recentActivitiesTable->setColumnWidth(2, 150); // Action
    
    connect(recentActivitiesTable, &QTableWidget::cellDoubleClicked, 
            this, &AdminDashboard::onActivityDoubleClicked);
}

void AdminDashboard::loadRecentActivitiesData()
{
    // For now, add sample activities
    recentActivitiesTable->setRowCount(5);
    
    QList<QStringList> activities = {
        {"10:30", "john_doe", "Login", "Success"},
        {"10:25", "jane_smith", "Transfer", "Completed"},
        {"10:20", "admin", "Customer Created", "Success"},
        {"10:15", "bob_wilson", "Deposit", "Completed"},
        {"10:10", "alice_brown", "Password Change", "Success"}
    };
    
    for (int i = 0; i < activities.size(); ++i) {
        const QStringList& activityData = activities[i];
        for (int j = 0; j < activityData.size(); ++j) {
            QTableWidgetItem* item = new QTableWidgetItem(activityData[j]);
            
            // Color code based on status
            if (j == 3) { // Status column
                if (activityData[j] == "Success" || activityData[j] == "Completed") {
                    item->setForeground(QBrush(QColor("#2e7d32")));
                } else if (activityData[j] == "Failed") {
                    item->setForeground(QBrush(QColor("#d32f2f")));
                } else {
                    item->setForeground(QBrush(QColor("#f57c00")));
                }
            }
            
            recentActivitiesTable->setItem(i, j, item);
        }
    }
}

void AdminDashboard::updateSystemStatus()
{
    // Update system status indicators
    if (bankService->isInitialized()) {
        databaseStatusLabel->setText("Database: Connected");
        databaseStatusLabel->setStyleSheet("QLabel#statusLabel { color: #2e7d32; }");
    } else {
        databaseStatusLabel->setText("Database: Disconnected");
        databaseStatusLabel->setStyleSheet("QLabel#statusLabel { color: #d32f2f; }");
    }
    
    if (authManager->isSessionValid()) {
        sessionStatusLabel->setText("Session: Active");
        sessionStatusLabel->setStyleSheet("QLabel#statusLabel { color: #2e7d32; }");
    } else {
        sessionStatusLabel->setText("Session: Expired");
        sessionStatusLabel->setStyleSheet("QLabel#statusLabel { color: #d32f2f; }");
    }
    
    // Update system health bar
    int health = 95;
    if (!bankService->isInitialized()) health -= 30;
    if (!authManager->isSessionValid()) health -= 20;
    
    systemHealthBar->setValue(health);
    
    if (health >= 80) {
        systemHealthBar->setStyleSheet(
            "QProgressBar#healthBar::chunk { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
            "stop:0 #4caf50, stop:0.5 #8bc34a, stop:1 #cddc39); }");
    } else if (health >= 60) {
        systemHealthBar->setStyleSheet(
            "QProgressBar#healthBar::chunk { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
            "stop:0 #ff9800, stop:0.5 #ffa726, stop:1 #ffca28); }");
    } else {
        systemHealthBar->setStyleSheet(
            "QProgressBar#healthBar::chunk { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
            "stop:0 #f44336, stop:0.5 #ef5350, stop:1 #e57373); }");
    }
}

void AdminDashboard::refreshAllData()
{
    loadStatistics();
    loadRecentActivities();
    updateSystemStatus();
}

void AdminDashboard::showManageCustomersDialog()
{
    AdminManagementWindow dialog(this);
    dialog.setModal(true);
    dialog.exec();
}

void AdminDashboard::showManageAccountsDialog()
{
    QMessageBox::information(this, "Manage Accounts", 
                             "Account management functionality would be implemented here.\n\n"
                             "This would include:\n"
                             "• View all accounts\n"
                             "• Suspend/activate accounts\n"
                             "• Account details\n"
                             "• Balance adjustments");
}

void AdminDashboard::showReportsDialog()
{
    ReportsWindow dialog(this);
    dialog.setModal(true);
    dialog.exec();
}

void AdminDashboard::showCreateCustomerDialog()
{
    // Simple create customer dialog for now
    bool ok;
    QString username = QInputDialog::getText(this, "Create Customer", 
                                               "Username:", QLineEdit::Normal, "", &ok);
    if (!ok || username.isEmpty()) return;
    
    QString email = QInputDialog::getText(this, "Create Customer", 
                                           "Email:", QLineEdit::Normal, "", &ok);
    if (!ok || email.isEmpty()) return;
    
    QString fullName = QInputDialog::getText(this, "Create Customer", 
                                              "Full Name:", QLineEdit::Normal, "", &ok);
    if (!ok || fullName.isEmpty()) return;
    
    QString phone = QInputDialog::getText(this, "Create Customer", 
                                             "Phone Number:", QLineEdit::Normal, "", &ok);
    if (!ok || phone.isEmpty()) return;
    
    QString password = QInputDialog::getText(this, "Create Customer", 
                                              "Password:", QLineEdit::Password, "", &ok);
    if (!ok || password.isEmpty()) return;
    
    // Create customer
    auto result = bankService->createCustomer(username, email, password, fullName, phone);
    
    if (result.success) {
        QMessageBox::information(this, "Success", 
                                 QString("Customer created successfully!\nCustomer ID: %1")
                                 .arg(result.customerId));
        loadStatistics(); // Refresh statistics
    } else {
        QMessageBox::warning(this, "Error", 
                              QString("Failed to create customer: %1")
                              .arg(result.message));
    }
}

void AdminDashboard::showBackupDialog()
{
    QString fileName = QFileDialog::getSaveFileName(
        this, "Backup Database", 
        QString("backup_%1.sql").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
        "SQL Files (*.sql)"
    );
    
    if (fileName.isEmpty()) {
        return;
    }
    
    bool success = bankService->createBackup(fileName);
    
    if (success) {
        QMessageBox::information(this, "Backup Complete", 
                                 QString("Database backup created successfully:\n%1")
                                 .arg(fileName));
    } else {
        QMessageBox::warning(this, "Backup Failed", 
                             "Failed to create database backup. Please check permissions and try again.");
    }
}

void AdminDashboard::showLogsDialog()
{
    QMessageBox::information(this, "System Logs", 
                             "System logs functionality would be implemented here.\n\n"
                             "This would include:\n"
                             "• View all system activities\n"
                             "• Filter by date, user, action\n"
                             "• Export logs\n"
                             "• Search logs\n"
                             "• Security event monitoring");
}

void AdminDashboard::showStatusMessage(const QString& message, bool isError)
{
    if (isError) {
        statusBar->showMessage(message, 5000);
    } else {
        statusBar->showMessage(message, 3000);
    }
}

void AdminDashboard::closeEvent(QCloseEvent* event)
{
    emit logoutRequested();
    QMainWindow::closeEvent(event);
}

void AdminDashboard::keyPressEvent(QKeyEvent* event)
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
void AdminDashboard::onManageCustomersClicked()
{
    showManageCustomersDialog();
}

void AdminDashboard::onManageAccountsClicked()
{
    showManageAccountsDialog();
}

void AdminDashboard::onViewReportsClicked()
{
    showReportsDialog();
}

void AdminDashboard::onCreateCustomerClicked()
{
    showCreateCustomerDialog();
}

void AdminDashboard::onBackupDatabaseClicked()
{
    showBackupDialog();
}

void AdminDashboard::onViewLogsClicked()
{
    showLogsDialog();
}

void AdminDashboard::onUpdateTimer()
{
    updateSystemStatus();
    
    // Check if session has expired
    if (!authManager->isSessionValid()) {
        emit sessionExpired();
        QMessageBox::warning(this, "Session Expired", 
                             "Your session has expired due to inactivity. Please login again.");
        onLogout();
    }
}

void AdminDashboard::onActivityDoubleClicked(int row, int column)
{
    Q_UNUSED(column);
    
    QString time = recentActivitiesTable->item(row, 0)->text();
    QString user = recentActivitiesTable->item(row, 1)->text();
    QString action = recentActivitiesTable->item(row, 2)->text();
    QString status = recentActivitiesTable->item(row, 3)->text();
    
    QMessageBox::information(this, "Activity Details", 
                             QString("Activity Details:\n\n"
                                     "Time: %1\n"
                                     "User: %2\n"
                                     "Action: %3\n"
                                     "Status: %4\n\n"
                                     "Full activity details would be shown here.")
                             .arg(time).arg(user).arg(action).arg(status));
}

void AdminDashboard::onLogout()
{
    emit logoutRequested();
}

void AdminDashboard::onExit()
{
    close();
}

void AdminDashboard::onRefreshData()
{
    refreshAllData();
    showStatusMessage("Data refreshed", false);
}

void AdminDashboard::onAbout()
{
    QMessageBox::about(this, "About Banking Management System", 
                       "Banking Management System v1.0.0\n\n"
                       "A secure and modern banking application\n"
                       "developed with Qt and C++.\n\n"
                       "Administrator Edition\n\n"
                       "© 2026 Banking System");
}

void AdminDashboard::onHelp()
{
    QMessageBox::information(this, "Admin Help", 
                             "Banking Management System - Administrator Help\n\n"
                             "• Use menu bar or quick action buttons for management operations\n"
                             "• Double-click on activities for details\n"
                             "• Press F5 to refresh data\n"
                             "• Press Ctrl+L to logout\n\n"
                             "For technical support, contact system administrator.");
}

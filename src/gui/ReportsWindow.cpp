#include "gui/ReportsWindow.h"
#include "core/BankService.h"
#include "core/Account.h"
#include "core/Customer.h"
#include "core/Transaction.h"
#include <QKeyEvent>
#include <QHeaderView>
#include <QMainWindow>
#include <QMessageBox>
#include <QFileDialog>
#include <QStatusBar>
#include <QTextStream>
#include <QDate>
#include <QTime>
#include <QGraphicsDropShadowEffect>

ReportsWindow::ReportsWindow(QWidget *parent)
    : QDialog(parent)
    , mainFrame(nullptr)
    , titleLabel(nullptr)
    , tabWidget(nullptr)
    , summaryTab(nullptr)
    , summaryFrame(nullptr)
    , summaryTitle(nullptr)
    , summaryLayout(nullptr)
    , totalCustomersLabel(nullptr)
    , totalCustomersValue(nullptr)
    , activeAccountsLabel(nullptr)
    , activeAccountsValue(nullptr)
    , totalDepositsLabel(nullptr)
    , totalDepositsValue(nullptr)
    , todayTransactionsLabel(nullptr)
    , todayTransactionsValue(nullptr)
    , failedLoginsLabel(nullptr)
    , failedLoginsValue(nullptr)
    , refreshSummaryButton(nullptr)
    , transactionsTab(nullptr)
    , transactionsFrame(nullptr)
    , transactionsTitle(nullptr)
    , transDateFromEdit(nullptr)
    , transDateToEdit(nullptr)
    , transTypeComboBox(nullptr)
    , generateTransButton(nullptr)
    , transactionsTable(nullptr)
    , transTotalLabel(nullptr)
    , transTotalValue(nullptr)
    , exportTransButton(nullptr)
    , customersTab(nullptr)
    , customersFrame(nullptr)
    , customersTitle(nullptr)
    , customerStatusComboBox(nullptr)
    , generateCustomersButton(nullptr)
    , customersTable(nullptr)
    , exportCustomersButton(nullptr)
    , securityTab(nullptr)
    , securityFrame(nullptr)
    , securityTitle(nullptr)
    , secDateFromEdit(nullptr)
    , secDateToEdit(nullptr)
    , secEventTypeComboBox(nullptr)
    , generateSecurityButton(nullptr)
    , securityTable(nullptr)
    , exportSecurityButton(nullptr)
    , mainLayout(nullptr)
    , frameLayout(nullptr)
    , summaryButtonLayout(nullptr)
    , transFilterLayout(nullptr)
    , transButtonLayout(nullptr)
    , customersFilterLayout(nullptr)
    , customersButtonLayout(nullptr)
    , secFilterLayout(nullptr)
    , secButtonLayout(nullptr)
    , bankService(nullptr)
    , isLoading(false)
{
    // Initialize services
    bankService = BankService::getInstance();
    
    // Setup dialog properties
    setModal(true);
    setWindowTitle("System Reports");
    setMinimumSize(1000, 700);
    resize(1200, 800);
    setWindowFlags(Qt::Dialog | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
    
    // Setup UI
    setupUI();
    setupStyles();
    
    // Load initial data
    loadSummaryReport();
    
    // Center dialog on parent
    if (parent) {
        move(parent->window()->frameGeometry().center() - frameGeometry().center());
    }
}

ReportsWindow::~ReportsWindow()
{
}

void ReportsWindow::setupUI()
{
    // Create main frame
    mainFrame = new QFrame();
    mainFrame->setObjectName("mainFrame");
    
    // Title
    titleLabel = new QLabel("System Reports & Analytics");
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    
    // Create tab widget
    tabWidget = new QTabWidget();
    tabWidget->setObjectName("tabWidget");
    
    // Setup tabs
    setupSummaryTab();
    tabWidget->addTab(summaryTab, "📊 Summary");
    
    setupTransactionsTab();
    tabWidget->addTab(transactionsTab, "💳 Transactions");
    
    setupCustomersTab();
    tabWidget->addTab(customersTab, "👥 Customers");
    
    setupSecurityTab();
    tabWidget->addTab(securityTab, "🔒 Security");
    
    // Setup layouts
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    
    frameLayout = new QVBoxLayout(mainFrame);
    frameLayout->setSpacing(20);
    frameLayout->setContentsMargins(30, 30, 30, 30);
    
    frameLayout->addWidget(titleLabel);
    frameLayout->addWidget(tabWidget);
    
    // Add frame to main layout
    mainLayout->addWidget(mainFrame);
}

void ReportsWindow::setupSummaryTab()
{
    summaryTab = new QWidget();
    summaryFrame = new QFrame();
    summaryFrame->setObjectName("contentFrame");
    
    summaryTitle = new QLabel("System Overview");
    summaryTitle->setObjectName("sectionTitleLabel");
    
    // Summary statistics grid
    summaryLayout = new QGridLayout();
    summaryLayout->setVerticalSpacing(15);
    summaryLayout->setHorizontalSpacing(30);
    
    totalCustomersLabel = new QLabel("Total Customers:");
    totalCustomersLabel->setObjectName("statLabel");
    totalCustomersValue = new QLabel("0");
    totalCustomersValue->setObjectName("statValue");
    
    activeAccountsLabel = new QLabel("Active Accounts:");
    activeAccountsLabel->setObjectName("statLabel");
    activeAccountsValue = new QLabel("0");
    activeAccountsValue->setObjectName("statValue");
    
    totalDepositsLabel = new QLabel("Total Deposits:");
    totalDepositsLabel->setObjectName("statLabel");
    totalDepositsValue = new QLabel("$0.00");
    totalDepositsValue->setObjectName("statValue");
    
    todayTransactionsLabel = new QLabel("Today's Transactions:");
    todayTransactionsLabel->setObjectName("statLabel");
    todayTransactionsValue = new QLabel("0");
    todayTransactionsValue->setObjectName("statValue");
    
    failedLoginsLabel = new QLabel("Failed Logins (24h):");
    failedLoginsLabel->setObjectName("statLabel");
    failedLoginsValue = new QLabel("0");
    failedLoginsValue->setObjectName("statValue");
    
    summaryLayout->addWidget(totalCustomersLabel, 0, 0);
    summaryLayout->addWidget(totalCustomersValue, 0, 1);
    summaryLayout->addWidget(activeAccountsLabel, 0, 2);
    summaryLayout->addWidget(activeAccountsValue, 0, 3);
    summaryLayout->addWidget(totalDepositsLabel, 1, 0);
    summaryLayout->addWidget(totalDepositsValue, 1, 1);
    summaryLayout->addWidget(todayTransactionsLabel, 1, 2);
    summaryLayout->addWidget(todayTransactionsValue, 1, 3);
    summaryLayout->addWidget(failedLoginsLabel, 2, 0, 1, 2);
    summaryLayout->addWidget(failedLoginsValue, 2, 2, 1, 2);
    
    refreshSummaryButton = new QPushButton("🔄 Refresh Summary");
    refreshSummaryButton->setObjectName("primaryButton");
    connect(refreshSummaryButton, &QPushButton::clicked, this, &ReportsWindow::onRefreshSummaryClicked);
    
    summaryButtonLayout = new QHBoxLayout();
    summaryButtonLayout->addStretch();
    summaryButtonLayout->addWidget(refreshSummaryButton);
    
    // Summary tab layout
    QVBoxLayout* summaryLayout2 = new QVBoxLayout(summaryTab);
    summaryLayout2->addWidget(summaryTitle);
    summaryLayout2->addWidget(summaryFrame);
    summaryLayout2->addLayout(summaryButtonLayout);
    
    // Summary frame layout
    QVBoxLayout* summaryFrameLayout = new QVBoxLayout(summaryFrame);
    summaryFrameLayout->addLayout(summaryLayout);
    summaryFrameLayout->addStretch();
}

void ReportsWindow::setupTransactionsTab()
{
    transactionsTab = new QWidget();
    transactionsFrame = new QFrame();
    transactionsFrame->setObjectName("contentFrame");
    
    transactionsTitle = new QLabel("Transaction Report");
    transactionsTitle->setObjectName("sectionTitleLabel");
    
    // Filter controls
    transDateFromEdit = new QDateEdit();
    transDateFromEdit->setObjectName("dateEdit");
    transDateFromEdit->setCalendarPopup(true);
    transDateFromEdit->setDate(QDate::currentDate().addDays(-30));
    
    transDateToEdit = new QDateEdit();
    transDateToEdit->setObjectName("dateEdit");
    transDateToEdit->setCalendarPopup(true);
    transDateToEdit->setDate(QDate::currentDate());
    
    transTypeComboBox = new QComboBox();
    transTypeComboBox->setObjectName("filterComboBox");
    transTypeComboBox->addItem("All Types", "ALL");
    transTypeComboBox->addItem("Deposits", "DEPOSIT");
    transTypeComboBox->addItem("Withdrawals", "WITHDRAWAL");
    transTypeComboBox->addItem("Transfers", "TRANSFER");
    
    generateTransButton = new QPushButton("📊 Generate Report");
    generateTransButton->setObjectName("primaryButton");
    connect(generateTransButton, &QPushButton::clicked, this, &ReportsWindow::onGenerateTransClicked);
    
    transFilterLayout = new QHBoxLayout();
    transFilterLayout->addWidget(new QLabel("From:"));
    transFilterLayout->addWidget(transDateFromEdit);
    transFilterLayout->addWidget(new QLabel("To:"));
    transFilterLayout->addWidget(transDateToEdit);
    transFilterLayout->addWidget(new QLabel("Type:"));
    transFilterLayout->addWidget(transTypeComboBox);
    transFilterLayout->addWidget(generateTransButton);
    transFilterLayout->addStretch();
    
    // Transactions table
    setupTransactionsTable();
    
    // Bottom section
    transTotalLabel = new QLabel("Total Amount:");
    transTotalLabel->setObjectName("totalLabel");
    transTotalValue = new QLabel("$0.00");
    transTotalValue->setObjectName("totalValue");
    
    exportTransButton = new QPushButton("📄 Export CSV");
    exportTransButton->setObjectName("secondaryButton");
    connect(exportTransButton, &QPushButton::clicked, this, &ReportsWindow::onExportTransClicked);
    
    transButtonLayout = new QHBoxLayout();
    transButtonLayout->addWidget(transTotalLabel);
    transButtonLayout->addWidget(transTotalValue);
    transButtonLayout->addStretch();
    transButtonLayout->addWidget(exportTransButton);
    
    // Transactions tab layout
    QVBoxLayout* transLayout = new QVBoxLayout(transactionsTab);
    transLayout->addWidget(transactionsTitle);
    transLayout->addLayout(transFilterLayout);
    transLayout->addWidget(transactionsFrame);
    transLayout->addLayout(transButtonLayout);
    
    // Transactions frame layout
    QVBoxLayout* transFrameLayout = new QVBoxLayout(transactionsFrame);
    transFrameLayout->addWidget(transactionsTable);
}

void ReportsWindow::setupCustomersTab()
{
    customersTab = new QWidget();
    customersFrame = new QFrame();
    customersFrame->setObjectName("contentFrame");
    
    customersTitle = new QLabel("Customer Report");
    customersTitle->setObjectName("sectionTitleLabel");
    
    // Filter controls
    customerStatusComboBox = new QComboBox();
    customerStatusComboBox->setObjectName("filterComboBox");
    customerStatusComboBox->addItem("All Customers", "ALL");
    customerStatusComboBox->addItem("Active Customers", "ACTIVE");
    customerStatusComboBox->addItem("Suspended Customers", "SUSPENDED");
    
    generateCustomersButton = new QPushButton("📊 Generate Report");
    generateCustomersButton->setObjectName("primaryButton");
    connect(generateCustomersButton, &QPushButton::clicked, this, &ReportsWindow::onGenerateCustomersClicked);
    
    customersFilterLayout = new QHBoxLayout();
    customersFilterLayout->addWidget(new QLabel("Status:"));
    customersFilterLayout->addWidget(customerStatusComboBox);
    customersFilterLayout->addWidget(generateCustomersButton);
    customersFilterLayout->addStretch();
    
    // Customers table
    setupCustomersTable();
    
    exportCustomersButton = new QPushButton("📄 Export CSV");
    exportCustomersButton->setObjectName("secondaryButton");
    connect(exportCustomersButton, &QPushButton::clicked, this, &ReportsWindow::onExportCustomersClicked);
    
    customersButtonLayout = new QHBoxLayout();
    customersButtonLayout->addStretch();
    customersButtonLayout->addWidget(exportCustomersButton);
    
    // Customers tab layout
    QVBoxLayout* customersLayout = new QVBoxLayout(customersTab);
    customersLayout->addWidget(customersTitle);
    customersLayout->addLayout(customersFilterLayout);
    customersLayout->addWidget(customersFrame);
    customersLayout->addLayout(customersButtonLayout);
    
    // Customers frame layout
    QVBoxLayout* customersFrameLayout = new QVBoxLayout(customersFrame);
    customersFrameLayout->addWidget(customersTable);
}

void ReportsWindow::setupSecurityTab()
{
    securityTab = new QWidget();
    securityFrame = new QFrame();
    securityFrame->setObjectName("contentFrame");
    
    securityTitle = new QLabel("Security Report");
    securityTitle->setObjectName("sectionTitleLabel");
    
    // Filter controls
    secDateFromEdit = new QDateEdit();
    secDateFromEdit->setObjectName("dateEdit");
    secDateFromEdit->setCalendarPopup(true);
    secDateFromEdit->setDate(QDate::currentDate().addDays(-7));
    
    secDateToEdit = new QDateEdit();
    secDateToEdit->setObjectName("dateEdit");
    secDateToEdit->setCalendarPopup(true);
    secDateToEdit->setDate(QDate::currentDate());
    
    secEventTypeComboBox = new QComboBox();
    secEventTypeComboBox->setObjectName("filterComboBox");
    secEventTypeComboBox->addItem("All Events", "ALL");
    secEventTypeComboBox->addItem("Login Attempts", "LOGIN");
    secEventTypeComboBox->addItem("Failed Logins", "FAILED_LOGIN");
    secEventTypeComboBox->addItem("Suspicious Activity", "SUSPICIOUS");
    secEventTypeComboBox->addItem("Account Locks", "ACCOUNT_LOCK");
    
    generateSecurityButton = new QPushButton("📊 Generate Report");
    generateSecurityButton->setObjectName("primaryButton");
    connect(generateSecurityButton, &QPushButton::clicked, this, &ReportsWindow::onGenerateSecurityClicked);
    
    secFilterLayout = new QHBoxLayout();
    secFilterLayout->addWidget(new QLabel("From:"));
    secFilterLayout->addWidget(secDateFromEdit);
    secFilterLayout->addWidget(new QLabel("To:"));
    secFilterLayout->addWidget(secDateToEdit);
    secFilterLayout->addWidget(new QLabel("Event:"));
    secFilterLayout->addWidget(secEventTypeComboBox);
    secFilterLayout->addWidget(generateSecurityButton);
    secFilterLayout->addStretch();
    
    // Security table
    setupSecurityTable();
    
    exportSecurityButton = new QPushButton("📄 Export CSV");
    exportSecurityButton->setObjectName("secondaryButton");
    connect(exportSecurityButton, &QPushButton::clicked, this, &ReportsWindow::onExportSecurityClicked);
    
    secButtonLayout = new QHBoxLayout();
    secButtonLayout->addStretch();
    secButtonLayout->addWidget(exportSecurityButton);
    
    // Security tab layout
    QVBoxLayout* secLayout = new QVBoxLayout(securityTab);
    secLayout->addWidget(securityTitle);
    secLayout->addLayout(secFilterLayout);
    secLayout->addWidget(securityFrame);
    secLayout->addLayout(secButtonLayout);
    
    // Security frame layout
    QVBoxLayout* secFrameLayout = new QVBoxLayout(securityFrame);
    secFrameLayout->addWidget(securityTable);
}

void ReportsWindow::setupStyles()
{
    QString styleSheet = R"(
        /* Main frame */
        QFrame#mainFrame {
            background: white;
            border-radius: 12px;
            border: 1px solid #e0e0e0;
        }
        
        /* Title label */
        QLabel#titleLabel {
            font-size: 22px;
            font-weight: bold;
            color: #1a237e;
            margin-bottom: 15px;
        }
        
        /* Section titles */
        QLabel#sectionTitleLabel {
            font-size: 16px;
            font-weight: bold;
            color: #37474f;
            margin-bottom: 15px;
        }
        
        /* Content frames */
        QFrame#contentFrame {
            background: #fafafa;
            border-radius: 8px;
            border: 1px solid #e0e0e0;
            padding: 15px;
        }
        
        /* Tab widget */
        QTabWidget#tabWidget::pane {
            border: 1px solid #e0e0e0;
            border-radius: 8px;
            background: #fafafa;
        }
        
        QTabWidget#tabWidget::tab-bar {
            alignment: center;
        }
        
        QTabBar::tab {
            background: #e0e0e0;
            border: 1px solid #d0d0d0;
            border-bottom: none;
            border-radius: 8px 8px 0 0;
            padding: 10px 20px;
            margin-right: 2px;
            font-weight: 600;
        }
        
        QTabBar::tab:selected {
            background: #1a237e;
            color: white;
        }
        
        QTabBar::tab:hover {
            background: #5c6bc0;
            color: white;
        }
        
        /* Statistics labels */
        QLabel#statLabel {
            font-size: 14px;
            font-weight: 600;
            color: #546e7a;
        }
        
        QLabel#statValue {
            font-size: 18px;
            font-weight: bold;
            color: #1a237e;
        }
        
        QLabel#totalLabel {
            font-size: 14px;
            font-weight: 600;
            color: #546e7a;
        }
        
        QLabel#totalValue {
            font-size: 18px;
            font-weight: bold;
            color: #2e7d32;
        }
        
        /* Tables */
        QTableWidget {
            background: white;
            border: 1px solid #e0e0e0;
            border-radius: 6px;
            gridline-color: #f0f0f0;
            selection-background-color: #e3f2fd;
            font-size: 11px;
        }
        
        QTableWidget::item {
            padding: 8px;
            border-bottom: 1px solid #f0f0f0;
        }
        
        QTableWidget::item:selected {
            background: #e3f2fd;
            color: #1565c0;
        }
        
        QTableWidget::item:hover {
            background: #f5f5f5;
        }
        
        QHeaderView::section {
            background: #1a237e;
            color: white;
            padding: 10px;
            border: none;
            font-weight: bold;
            font-size: 12px;
        }
        
        /* Date edits */
        QDateEdit#dateEdit {
            background: white;
            border: 2px solid #e0e0e0;
            border-radius: 8px;
            padding: 6px;
            font-size: 12px;
            color: #263238;
        }
        
        QDateEdit#dateEdit:focus {
            border-color: #1a237e;
            outline: none;
        }
        
        /* Combo box */
        QComboBox#filterComboBox {
            background: white;
            border: 2px solid #e0e0e0;
            border-radius: 8px;
            padding: 6px;
            font-size: 12px;
            color: #263238;
            min-width: 120px;
        }
        
        QComboBox#filterComboBox:focus {
            border-color: #1a237e;
            outline: none;
        }
        
        /* Buttons */
        QPushButton#primaryButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #1a237e, stop:1 #283593);
            color: white;
            border: none;
            border-radius: 8px;
            padding: 8px 16px;
            font-size: 12px;
            font-weight: 600;
            min-width: 120px;
        }
        
        QPushButton#primaryButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #283593, stop:1 #3949ab);
        }
        
        QPushButton#secondaryButton {
            background: #78909c;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 8px 16px;
            font-size: 12px;
            font-weight: 600;
            min-width: 100px;
        }
        
        QPushButton#secondaryButton:hover {
            background: #607d8b;
        }
        
        QPushButton:disabled {
            background: #b0bec5;
            color: #78909c;
        }
    )";
    
    setStyleSheet(styleSheet);
    applyModernStyling();
}

void ReportsWindow::applyModernStyling()
{
    // Add drop shadow effect to main frame
    if (mainFrame) {
        QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
        shadow->setBlurRadius(15);
        shadow->setColor(QColor(0, 0, 0, 30));
        shadow->setOffset(0, 5);
        mainFrame->setGraphicsEffect(shadow);
    }
}

void ReportsWindow::setupTransactionsTable()
{
    transactionsTable = new QTableWidget();
    transactionsTable->setObjectName("transactionsTable");
    
    transactionsTable->setColumnCount(6);
    transactionsTable->setHorizontalHeaderLabels({
        "Date", "Reference", "Type", "Amount", "Account", "Status"
    });
    
    // Configure table properties
    transactionsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    transactionsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    transactionsTable->setAlternatingRowColors(true);
    transactionsTable->verticalHeader()->setVisible(false);
    transactionsTable->horizontalHeader()->setStretchLastSection(true);
    
    // Set column widths
    transactionsTable->setColumnWidth(0, 100); // Date
    transactionsTable->setColumnWidth(1, 120); // Reference
    transactionsTable->setColumnWidth(2, 80);  // Type
    transactionsTable->setColumnWidth(3, 100); // Amount
    transactionsTable->setColumnWidth(4, 120); // Account
}

void ReportsWindow::setupCustomersTable()
{
    customersTable = new QTableWidget();
    customersTable->setObjectName("customersTable");
    
    customersTable->setColumnCount(6);
    customersTable->setHorizontalHeaderLabels({
        "ID", "Username", "Full Name", "Email", "Phone", "Status"
    });
    
    // Configure table properties
    customersTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    customersTable->setSelectionMode(QAbstractItemView::SingleSelection);
    customersTable->setAlternatingRowColors(true);
    customersTable->verticalHeader()->setVisible(false);
    customersTable->horizontalHeader()->setStretchLastSection(true);
    
    // Set column widths
    customersTable->setColumnWidth(0, 50);  // ID
    customersTable->setColumnWidth(1, 100); // Username
    customersTable->setColumnWidth(2, 150); // Full Name
    customersTable->setColumnWidth(3, 200); // Email
    customersTable->setColumnWidth(4, 120); // Phone
}

void ReportsWindow::setupSecurityTable()
{
    securityTable = new QTableWidget();
    securityTable->setObjectName("securityTable");
    
    securityTable->setColumnCount(5);
    securityTable->setHorizontalHeaderLabels({
        "Timestamp", "User", "Event Type", "Details", "Status"
    });
    
    // Configure table properties
    securityTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    securityTable->setSelectionMode(QAbstractItemView::SingleSelection);
    securityTable->setAlternatingRowColors(true);
    securityTable->verticalHeader()->setVisible(false);
    securityTable->horizontalHeader()->setStretchLastSection(true);
    
    // Set column widths
    securityTable->setColumnWidth(0, 120); // Timestamp
    securityTable->setColumnWidth(1, 100); // User
    securityTable->setColumnWidth(2, 120); // Event Type
    securityTable->setColumnWidth(3, 200); // Details
}

void ReportsWindow::loadSummaryReport()
{
    setLoadingState(true);
    updateSummaryStatistics();
    setLoadingState(false);
}

void ReportsWindow::loadTransactionReport()
{
    setLoadingState(true);
    
    // Get transactions based on filters
    QDateTime fromDate(transDateFromEdit->date(), QTime(0, 0, 0));
    QDateTime toDate(transDateToEdit->date(), QTime(23, 59, 59));
    
    auto transactions = bankService->getTransactionsByDateRange(fromDate, toDate);
    
    QString selectedType = transTypeComboBox->currentData().toString();
    
    // Filter by type if needed
    if (selectedType != "ALL") {
        QList<std::shared_ptr<Transaction>> filteredTransactions;
        for (const auto& transaction : transactions) {
            if (transaction->getTransactionType() == selectedType) {
                filteredTransactions.append(transaction);
            }
        }
        transactions = filteredTransactions;
    }
    
    // Populate table
    transactionsTable->setRowCount(transactions.size());
    double totalAmount = 0.0;
    
    for (int i = 0; i < transactions.size(); ++i) {
        auto transaction = transactions[i];
        
        transactionsTable->setItem(i, 0, new QTableWidgetItem(transaction->getFormattedDate()));
        transactionsTable->setItem(i, 1, new QTableWidgetItem(transaction->getReferenceNumber()));
        transactionsTable->setItem(i, 2, new QTableWidgetItem(transaction->getTransactionTypeDisplay()));
        transactionsTable->setItem(i, 3, new QTableWidgetItem(QString("$%1").arg(QLocale().toString(transaction->getAmount(), 'f', 2))));
        
        // Get account number
        QString accountNumber = "Unknown";
        if (transaction->getFromAccountId() > 0) {
            auto account = bankService->getAccountById(transaction->getFromAccountId());
            if (account) {
                accountNumber = account->getAccountNumber();
            }
        }
        transactionsTable->setItem(i, 4, new QTableWidgetItem(accountNumber));
        transactionsTable->setItem(i, 5, new QTableWidgetItem(transaction->getStatusDisplay()));
        
        totalAmount += transaction->getAmount();
    }
    
    transTotalValue->setText(QString("$%1").arg(QLocale().toString(totalAmount, 'f', 2)));
    
    setLoadingState(false);
}

void ReportsWindow::loadCustomerReport()
{
    setLoadingState(true);
    
    auto customers = bankService->getAllCustomers();
    
    QString selectedStatus = customerStatusComboBox->currentData().toString();
    
    // Filter by status if needed
    if (selectedStatus != "ALL") {
        QList<std::shared_ptr<Customer>> filteredCustomers;
        for (const auto& customer : customers) {
            bool isActive = customer->isActiveStatus() && !customer->isAccountLocked();
            
            if ((selectedStatus == "ACTIVE" && isActive) || 
                (selectedStatus == "SUSPENDED" && !isActive)) {
                filteredCustomers.append(customer);
            }
        }
        customers = filteredCustomers;
    }
    
    // Populate table
    customersTable->setRowCount(customers.size());
    
    for (int i = 0; i < customers.size(); ++i) {
        auto customer = customers[i];
        
        customersTable->setItem(i, 0, new QTableWidgetItem(QString::number(customer->getUserId())));
        customersTable->setItem(i, 1, new QTableWidgetItem(customer->getUsername()));
        customersTable->setItem(i, 2, new QTableWidgetItem(customer->getFullName()));
        customersTable->setItem(i, 3, new QTableWidgetItem(customer->getEmail()));
        customersTable->setItem(i, 4, new QTableWidgetItem(customer->getPhoneNumber()));
        
        QString status = customer->isActiveStatus() && !customer->isAccountLocked() ? "Active" : "Suspended";
        QTableWidgetItem* statusItem = new QTableWidgetItem(status);
        
        if (status == "Active") {
            statusItem->setForeground(QBrush(QColor("#2e7d32")));
        } else {
            statusItem->setForeground(QBrush(QColor("#d32f2f")));
        }
        
        customersTable->setItem(i, 5, statusItem);
    }
    
    setLoadingState(false);
}

void ReportsWindow::loadSecurityReport()
{
    setLoadingState(true);
    
    // For now, add sample security events
    securityTable->setRowCount(5);
    
    QList<QStringList> securityEvents = {
        {"2026-05-08 10:30", "john_doe", "LOGIN", "Successful login", "SUCCESS"},
        {"2026-05-08 10:25", "jane_smith", "FAILED_LOGIN", "Invalid password", "FAILED"},
        {"2026-05-08 10:20", "unknown", "SUSPICIOUS", "Multiple failed attempts", "WARNING"},
        {"2026-05-08 10:15", "bob_wilson", "ACCOUNT_LOCK", "Account locked temporarily", "INFO"},
        {"2026-05-08 10:10", "admin", "LOGIN", "Admin login successful", "SUCCESS"}
    };
    
    for (int i = 0; i < securityEvents.size(); ++i) {
        const QStringList& eventData = securityEvents[i];
        for (int j = 0; j < eventData.size(); ++j) {
            QTableWidgetItem* item = new QTableWidgetItem(eventData[j]);
            
            // Color code based on status
            if (j == 4) { // Status column
                if (eventData[j] == "SUCCESS") {
                    item->setForeground(QBrush(QColor("#2e7d32")));
                } else if (eventData[j] == "FAILED") {
                    item->setForeground(QBrush(QColor("#d32f2f")));
                } else if (eventData[j] == "WARNING") {
                    item->setForeground(QBrush(QColor("#f57c00")));
                } else {
                    item->setForeground(QBrush(QColor("#1976d2")));
                }
            }
            
            securityTable->setItem(i, j, item);
        }
    }
    
    setLoadingState(false);
}

void ReportsWindow::updateSummaryStatistics()
{
    auto stats = bankService->getBankStatistics();
    
    totalCustomersValue->setText(QString::number(stats.totalCustomers));
    activeAccountsValue->setText(QString::number(stats.activeAccounts));
    totalDepositsValue->setText(QString("$%1").arg(QLocale().toString(stats.totalDeposits, 'f', 2)));
    todayTransactionsValue->setText(QString::number(stats.todayTransactions));
    failedLoginsValue->setText(QString::number(stats.failedLoginsToday));
}

void ReportsWindow::exportTransactionsReport()
{
    QString fileName = QFileDialog::getSaveFileName(
        this, "Export Transactions Report", 
        QString("transactions_report_%1.csv").arg(QDate::currentDate().toString("yyyyMMdd")),
        "CSV Files (*.csv)"
    );
    
    if (!fileName.isEmpty()) {
        exportToCSV(transactionsTable, fileName);
    }
}

void ReportsWindow::exportCustomersReport()
{
    QString fileName = QFileDialog::getSaveFileName(
        this, "Export Customers Report", 
        QString("customers_report_%1.csv").arg(QDate::currentDate().toString("yyyyMMdd")),
        "CSV Files (*.csv)"
    );
    
    if (!fileName.isEmpty()) {
        exportToCSV(customersTable, fileName);
    }
}

void ReportsWindow::exportSecurityReport()
{
    QString fileName = QFileDialog::getSaveFileName(
        this, "Export Security Report", 
        QString("security_report_%1.csv").arg(QDate::currentDate().toString("yyyyMMdd")),
        "CSV Files (*.csv)"
    );
    
    if (!fileName.isEmpty()) {
        exportToCSV(securityTable, fileName);
    }
}

void ReportsWindow::exportToCSV(QTableWidget* table, const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Export Error", "Failed to create export file.");
        return;
    }
    
    QTextStream out(&file);
    
    // Write header
    QStringList headers;
    for (int i = 0; i < table->columnCount(); ++i) {
        headers << table->horizontalHeaderItem(i)->text();
    }
    out << headers.join(",") << "\n";
    
    // Write data
    for (int i = 0; i < table->rowCount(); ++i) {
        QStringList row;
        for (int j = 0; j < table->columnCount(); ++j) {
            QTableWidgetItem* item = table->item(i, j);
            row << (item ? item->text() : "");
        }
        out << row.join(",") << "\n";
    }
    
    file.close();
    
    QMessageBox::information(this, "Export Complete", 
                             QString("Report exported successfully to:\n%1").arg(fileName));
}

void ReportsWindow::showStatusMessage(const QString& message, bool isError)
{
    // Update status bar or show a temporary message
    if (parent()) {
        if (auto mainWindow = qobject_cast<QMainWindow*>(parent())) {
            if (isError) {
                mainWindow->statusBar()->showMessage(message, 5000);
            } else {
                mainWindow->statusBar()->showMessage(message, 3000);
            }
        }
    }
}

void ReportsWindow::setLoadingState(bool loading)
{
    isLoading = loading;
    
    if (loading) {
        setCursor(Qt::WaitCursor);
        refreshSummaryButton->setEnabled(false);
        generateTransButton->setEnabled(false);
        generateCustomersButton->setEnabled(false);
        generateSecurityButton->setEnabled(false);
    } else {
        setCursor(Qt::ArrowCursor);
        refreshSummaryButton->setEnabled(true);
        generateTransButton->setEnabled(true);
        generateCustomersButton->setEnabled(true);
        generateSecurityButton->setEnabled(true);
    }
}

void ReportsWindow::closeEvent(QCloseEvent* event)
{
    if (isLoading) {
        event->ignore();
        return;
    }
    QDialog::closeEvent(event);
}

void ReportsWindow::keyPressEvent(QKeyEvent* event)
{
    // Handle keyboard shortcuts
    if (event->key() == Qt::Key_F5) {
        if (tabWidget->currentWidget() == summaryTab) {
            onRefreshSummaryClicked();
        }
    } else if (event->key() == Qt::Key_Escape && !isLoading) {
        close();
    }
    QDialog::keyPressEvent(event);
}

// Slot implementations
void ReportsWindow::onRefreshSummaryClicked()
{
    loadSummaryReport();
}

void ReportsWindow::onGenerateTransClicked()
{
    loadTransactionReport();
}

void ReportsWindow::onGenerateCustomersClicked()
{
    loadCustomerReport();
}

void ReportsWindow::onGenerateSecurityClicked()
{
    loadSecurityReport();
}

void ReportsWindow::onExportTransClicked()
{
    exportTransactionsReport();
}

void ReportsWindow::onExportCustomersClicked()
{
    exportCustomersReport();
}

void ReportsWindow::onExportSecurityClicked()
{
    exportSecurityReport();
}

void ReportsWindow::onTransTypeChanged()
{
    // Auto-generate report when type changes
    if (!isLoading) {
        loadTransactionReport();
    }
}

void ReportsWindow::onCustomerStatusChanged()
{
    // Auto-generate report when status changes
    if (!isLoading) {
        loadCustomerReport();
    }
}

void ReportsWindow::onSecEventTypeChanged()
{
    // Auto-generate report when event type changes
    if (!isLoading) {
        loadSecurityReport();
    }
}

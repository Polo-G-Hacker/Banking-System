#include "gui/AdminManagementWindow.h"
#include "core/BankService.h"
#include "core/Account.h"
#include "core/Customer.h"
#include <QKeyEvent>
#include <QMainWindow>
#include <QMessageBox>
#include <QInputDialog>
#include <QHeaderView>
#include <QStatusBar>
#include <QGraphicsDropShadowEffect>

AdminManagementWindow::AdminManagementWindow(QWidget *parent)
    : QDialog(parent)
    , mainFrame(nullptr)
    , titleLabel(nullptr)
    , tabWidget(nullptr)
    , customersTab(nullptr)
    , customersFrame(nullptr)
    , customersTable(nullptr)
    , addCustomerButton(nullptr)
    , editCustomerButton(nullptr)
    , suspendCustomerButton(nullptr)
    , activateCustomerButton(nullptr)
    , customerSearchEdit(nullptr)
    , refreshCustomersButton(nullptr)
    , accountsTab(nullptr)
    , accountsFrame(nullptr)
    , accountsTable(nullptr)
    , createAccountButton(nullptr)
    , closeAccountButton(nullptr)
    , viewAccountDetailsButton(nullptr)
    , accountFilterComboBox(nullptr)
    , accountSearchEdit(nullptr)
    , refreshAccountsButton(nullptr)
    , mainLayout(nullptr)
    , frameLayout(nullptr)
    , customersButtonLayout(nullptr)
    , accountsButtonLayout(nullptr)
    , customersSearchLayout(nullptr)
    , accountsSearchLayout(nullptr)
    , bankService(nullptr)
    , isLoading(false)
{
    // Initialize services
    bankService = BankService::getInstance();
    
    // Setup dialog properties
    setModal(true);
    setWindowTitle("User & Account Management");
    setMinimumSize(1000, 700);
    resize(1200, 800);
    setWindowFlags(Qt::Dialog | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
    
    // Setup UI
    setupUI();
    setupStyles();
    
    // Load initial data
    loadCustomers();
    loadAccounts();
    
    // Center dialog on parent
    if (parent) {
        move(parent->window()->frameGeometry().center() - frameGeometry().center());
    }
}

AdminManagementWindow::~AdminManagementWindow()
{
}

void AdminManagementWindow::setupUI()
{
    // Create main frame
    mainFrame = new QFrame();
    mainFrame->setObjectName("mainFrame");
    
    // Title
    titleLabel = new QLabel("User & Account Management");
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    
    // Create tab widget
    tabWidget = new QTabWidget();
    tabWidget->setObjectName("tabWidget");
    
    // Setup customers tab
    setupCustomersTab();
    tabWidget->addTab(customersTab, "👥 Customers");
    
    // Setup accounts tab
    setupAccountsTab();
    tabWidget->addTab(accountsTab, "💳 Accounts");
    
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

void AdminManagementWindow::setupCustomersTab()
{
    customersTab = new QWidget();
    customersFrame = new QFrame();
    customersFrame->setObjectName("contentFrame");
    
    // Search layout
    customerSearchEdit = new QLineEdit();
    customerSearchEdit->setObjectName("searchEdit");
    customerSearchEdit->setPlaceholderText("Search customers by name, username, or email...");
    
    refreshCustomersButton = new QPushButton("🔄 Refresh");
    refreshCustomersButton->setObjectName("refreshButton");
    
    customersSearchLayout = new QHBoxLayout();
    customersSearchLayout->addWidget(customerSearchEdit);
    customersSearchLayout->addWidget(refreshCustomersButton);
    
    // Customers table
    setupCustomersTable();
    
    // Button layout
    addCustomerButton = new QPushButton("➕ Add Customer");
    addCustomerButton->setObjectName("primaryButton");
    connect(addCustomerButton, &QPushButton::clicked, this, &AdminManagementWindow::onAddCustomerClicked);
    
    editCustomerButton = new QPushButton("✏️ Edit");
    editCustomerButton->setObjectName("secondaryButton");
    connect(editCustomerButton, &QPushButton::clicked, this, &AdminManagementWindow::onEditCustomerClicked);
    
    suspendCustomerButton = new QPushButton("⏸️ Suspend");
    suspendCustomerButton->setObjectName("warningButton");
    connect(suspendCustomerButton, &QPushButton::clicked, this, &AdminManagementWindow::onSuspendCustomerClicked);
    
    activateCustomerButton = new QPushButton("✅ Activate");
    activateCustomerButton->setObjectName("successButton");
    connect(activateCustomerButton, &QPushButton::clicked, this, &AdminManagementWindow::onActivateCustomerClicked);
    
    customersButtonLayout = new QHBoxLayout();
    customersButtonLayout->addWidget(addCustomerButton);
    customersButtonLayout->addWidget(editCustomerButton);
    customersButtonLayout->addWidget(suspendCustomerButton);
    customersButtonLayout->addWidget(activateCustomerButton);
    customersButtonLayout->addStretch();
    
    // Customers tab layout
    QVBoxLayout* customersLayout = new QVBoxLayout(customersTab);
    customersLayout->addLayout(customersSearchLayout);
    customersLayout->addWidget(customersFrame);
    customersLayout->addLayout(customersButtonLayout);
    
    // Customers frame layout
    QVBoxLayout* customersFrameLayout = new QVBoxLayout(customersFrame);
    customersFrameLayout->addWidget(customersTable);
    
    // Connect signals
    connect(refreshCustomersButton, &QPushButton::clicked, this, &AdminManagementWindow::onRefreshCustomersClicked);
    connect(customerSearchEdit, &QLineEdit::textChanged, this, &AdminManagementWindow::onCustomerSearchChanged);
    connect(customersTable, &QTableWidget::itemSelectionChanged, this, &AdminManagementWindow::onCustomerSelectionChanged);
    connect(customersTable, &QTableWidget::cellDoubleClicked, this, &AdminManagementWindow::onCustomerDoubleClicked);
}

void AdminManagementWindow::setupAccountsTab()
{
    accountsTab = new QWidget();
    accountsFrame = new QFrame();
    accountsFrame->setObjectName("contentFrame");
    
    // Search and filter layout
    accountFilterComboBox = new QComboBox();
    accountFilterComboBox->setObjectName("filterComboBox");
    accountFilterComboBox->addItem("All Accounts", "ALL");
    accountFilterComboBox->addItem("Active Accounts", "ACTIVE");
    accountFilterComboBox->addItem("Inactive Accounts", "INACTIVE");
    accountFilterComboBox->addItem("Savings Accounts", "SAVINGS");
    accountFilterComboBox->addItem("Checking Accounts", "CHECKING");
    accountFilterComboBox->addItem("Current Accounts", "CURRENT");
    
    accountSearchEdit = new QLineEdit();
    accountSearchEdit->setObjectName("searchEdit");
    accountSearchEdit->setPlaceholderText("Search accounts by number or customer...");
    
    refreshAccountsButton = new QPushButton("🔄 Refresh");
    refreshAccountsButton->setObjectName("refreshButton");
    
    accountsSearchLayout = new QHBoxLayout();
    accountsSearchLayout->addWidget(accountFilterComboBox);
    accountsSearchLayout->addWidget(accountSearchEdit);
    accountsSearchLayout->addWidget(refreshAccountsButton);
    
    // Accounts table
    setupAccountsTable();
    
    // Button layout
    createAccountButton = new QPushButton("➕ Create Account");
    createAccountButton->setObjectName("primaryButton");
    connect(createAccountButton, &QPushButton::clicked, this, &AdminManagementWindow::onCreateAccountClicked);
    
    closeAccountButton = new QPushButton("❌ Close Account");
    closeAccountButton->setObjectName("dangerButton");
    connect(closeAccountButton, &QPushButton::clicked, this, &AdminManagementWindow::onCloseAccountClicked);
    
    viewAccountDetailsButton = new QPushButton("👁️ View Details");
    viewAccountDetailsButton->setObjectName("secondaryButton");
    connect(viewAccountDetailsButton, &QPushButton::clicked, this, &AdminManagementWindow::onViewAccountDetailsClicked);
    
    accountsButtonLayout = new QHBoxLayout();
    accountsButtonLayout->addWidget(createAccountButton);
    accountsButtonLayout->addWidget(viewAccountDetailsButton);
    accountsButtonLayout->addWidget(closeAccountButton);
    accountsButtonLayout->addStretch();
    
    // Accounts tab layout
    QVBoxLayout* accountsLayout = new QVBoxLayout(accountsTab);
    accountsLayout->addLayout(accountsSearchLayout);
    accountsLayout->addWidget(accountsFrame);
    accountsLayout->addLayout(accountsButtonLayout);
    
    // Accounts frame layout
    QVBoxLayout* accountsFrameLayout = new QVBoxLayout(accountsFrame);
    accountsFrameLayout->addWidget(accountsTable);
    
    // Connect signals
    connect(refreshAccountsButton, &QPushButton::clicked, this, &AdminManagementWindow::onRefreshAccountsClicked);
    connect(accountFilterComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &AdminManagementWindow::onAccountFilterChanged);
    connect(accountSearchEdit, &QLineEdit::textChanged, this, &AdminManagementWindow::onAccountSearchChanged);
    connect(accountsTable, &QTableWidget::itemSelectionChanged, this, &AdminManagementWindow::onAccountSelectionChanged);
    connect(accountsTable, &QTableWidget::cellDoubleClicked, this, &AdminManagementWindow::onAccountDoubleClicked);
}

void AdminManagementWindow::setupStyles()
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
        
        /* Content frames */
        QFrame#contentFrame {
            background: #fafafa;
            border-radius: 8px;
            border: 1px solid #e0e0e0;
            padding: 10px;
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
        
        /* Line edits */
        QLineEdit#searchEdit {
            background: white;
            border: 2px solid #e0e0e0;
            border-radius: 8px;
            padding: 8px;
            font-size: 12px;
            color: #263238;
        }
        
        QLineEdit#searchEdit:focus {
            border-color: #1a237e;
            outline: none;
        }
        
        /* Combo box */
        QComboBox#filterComboBox {
            background: white;
            border: 2px solid #e0e0e0;
            border-radius: 8px;
            padding: 8px;
            font-size: 12px;
            color: #263238;
            min-width: 150px;
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
            padding: 10px 20px;
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
            padding: 10px 20px;
            font-size: 12px;
            font-weight: 600;
            min-width: 100px;
        }
        
        QPushButton#secondaryButton:hover {
            background: #607d8b;
        }
        
        QPushButton#successButton {
            background: #4caf50;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 10px 20px;
            font-size: 12px;
            font-weight: 600;
            min-width: 100px;
        }
        
        QPushButton#successButton:hover {
            background: #66bb6a;
        }
        
        QPushButton#warningButton {
            background: #ff9800;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 10px 20px;
            font-size: 12px;
            font-weight: 600;
            min-width: 100px;
        }
        
        QPushButton#warningButton:hover {
            background: #f57c00;
        }
        
        QPushButton#dangerButton {
            background: #d32f2f;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 10px 20px;
            font-size: 12px;
            font-weight: 600;
            min-width: 120px;
        }
        
        QPushButton#dangerButton:hover {
            background: #f44336;
        }
        
        QPushButton#refreshButton {
            background: #5c6bc0;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 8px 16px;
            font-size: 11px;
            font-weight: 600;
            min-width: 80px;
        }
        
        QPushButton#refreshButton:hover {
            background: #3f51b5;
        }
        
        QPushButton:disabled {
            background: #b0bec5;
            color: #78909c;
        }
    )";
    
    setStyleSheet(styleSheet);
    applyModernStyling();
}

void AdminManagementWindow::applyModernStyling()
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

void AdminManagementWindow::setupCustomersTable()
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

void AdminManagementWindow::setupAccountsTable()
{
    accountsTable = new QTableWidget();
    accountsTable->setObjectName("accountsTable");
    
    accountsTable->setColumnCount(6);
    accountsTable->setHorizontalHeaderLabels({
        "Account Number", "Customer", "Type", "Balance", "Status", "Created"
    });
    
    // Configure table properties
    accountsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    accountsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    accountsTable->setAlternatingRowColors(true);
    accountsTable->verticalHeader()->setVisible(false);
    accountsTable->horizontalHeader()->setStretchLastSection(true);
    
    // Set column widths
    accountsTable->setColumnWidth(0, 120); // Account Number
    accountsTable->setColumnWidth(1, 150); // Customer
    accountsTable->setColumnWidth(2, 100); // Type
    accountsTable->setColumnWidth(3, 100); // Balance
    accountsTable->setColumnWidth(4, 80);  // Status
}

void AdminManagementWindow::loadCustomers()
{
    setLoadingState(true);
    refreshCustomersData();
    setLoadingState(false);
}

void AdminManagementWindow::loadAccounts()
{
    setLoadingState(true);
    refreshAccountsData();
    setLoadingState(false);
}

void AdminManagementWindow::refreshCustomersData()
{
    auto customers = bankService->getAllCustomers();
    
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
    
    updateCustomerButtons();
}

void AdminManagementWindow::refreshAccountsData()
{
    auto accounts = bankService->getAllAccounts();
    
    accountsTable->setRowCount(accounts.size());
    
    for (int i = 0; i < accounts.size(); ++i) {
        auto account = accounts[i];
        
        accountsTable->setItem(i, 0, new QTableWidgetItem(account->getAccountNumber()));
        
        // Get customer name
        QString customerName = "Unknown";
        auto customers = bankService->getAllCustomers();
        for (const auto& customer : customers) {
            if (customer->getUserId() == account->getUserId()) {
                customerName = customer->getFullName();
                break;
            }
        }
        accountsTable->setItem(i, 1, new QTableWidgetItem(customerName));
        
        accountsTable->setItem(i, 2, new QTableWidgetItem(account->getAccountTypeDisplay()));
        accountsTable->setItem(i, 3, new QTableWidgetItem(QString("$%1").arg(QLocale().toString(account->getBalance(), 'f', 2))));
        
        QString status = account->isActiveStatus() ? "Active" : "Inactive";
        QTableWidgetItem* statusItem = new QTableWidgetItem(status);
        
        if (status == "Active") {
            statusItem->setForeground(QBrush(QColor("#2e7d32")));
        } else {
            statusItem->setForeground(QBrush(QColor("#d32f2f")));
        }
        
        accountsTable->setItem(i, 4, statusItem);
        accountsTable->setItem(i, 5, new QTableWidgetItem(account->getCreatedAt().toString("yyyy-MM-dd")));
    }
    
    updateAccountButtons();
}

void AdminManagementWindow::showAddCustomerDialog()
{
    bool ok;
    QString username = QInputDialog::getText(this, "Add Customer", 
                                               "Username:", QLineEdit::Normal, "", &ok);
    if (!ok || username.isEmpty()) return;
    
    QString email = QInputDialog::getText(this, "Add Customer", 
                                           "Email:", QLineEdit::Normal, "", &ok);
    if (!ok || email.isEmpty()) return;
    
    QString fullName = QInputDialog::getText(this, "Add Customer", 
                                              "Full Name:", QLineEdit::Normal, "", &ok);
    if (!ok || fullName.isEmpty()) return;
    
    QString phone = QInputDialog::getText(this, "Add Customer", 
                                             "Phone Number:", QLineEdit::Normal, "", &ok);
    if (!ok || phone.isEmpty()) return;
    
    QString password = QInputDialog::getText(this, "Add Customer", 
                                              "Password:", QLineEdit::Password, "", &ok);
    if (!ok || password.isEmpty()) return;
    
    // Create customer
    auto result = bankService->createCustomer(username, email, password, fullName, phone);
    
    if (result.success) {
        QMessageBox::information(this, "Success", 
                                 QString("Customer created successfully!\nCustomer ID: %1")
                                 .arg(result.customerId));
        refreshCustomersData();
    } else {
        QMessageBox::warning(this, "Error", 
                              QString("Failed to create customer: %1")
                              .arg(result.message));
    }
}

void AdminManagementWindow::showEditCustomerDialog()
{
    int customerId = getSelectedCustomerId();
    if (customerId <= 0) return;
    
    QMessageBox::information(this, "Edit Customer", 
                             QString("Edit customer functionality for ID %1 would be implemented here.")
                             .arg(customerId));
}

void AdminManagementWindow::suspendSelectedCustomer()
{
    int customerId = getSelectedCustomerId();
    if (customerId <= 0) return;
    
    bool ok = false;
    QString reason = QInputDialog::getText(this, "Suspend Customer", 
                                            "Reason for suspension:", QLineEdit::Normal, "", &ok);
    if (!ok || reason.isEmpty()) return;
    
    bool success = bankService->suspendCustomer(customerId, reason);
    
    if (success) {
        QMessageBox::information(this, "Success", "Customer suspended successfully.");
        refreshCustomersData();
    } else {
        QMessageBox::warning(this, "Error", "Failed to suspend customer.");
    }
}

void AdminManagementWindow::activateSelectedCustomer()
{
    int customerId = getSelectedCustomerId();
    if (customerId <= 0) return;
    
    bool success = bankService->activateCustomer(customerId);
    
    if (success) {
        QMessageBox::information(this, "Success", "Customer activated successfully.");
        refreshCustomersData();
    } else {
        QMessageBox::warning(this, "Error", "Failed to activate customer.");
    }
}

void AdminManagementWindow::showCreateAccountDialog()
{
    bool ok;
    int customerId = QInputDialog::getInt(this, "Create Account", 
                                          "Customer ID:", 1, 1, 999999, 1, &ok);
    if (!ok) return;
    
    QStringList accountTypes = {"SAVINGS", "CHECKING", "CURRENT"};
    QString accountType = QInputDialog::getItem(this, "Create Account", 
                                                "Account Type:", accountTypes, 0, false, &ok);
    if (!ok || accountType.isEmpty()) return;
    
    double initialBalance = QInputDialog::getDouble(this, "Create Account", 
                                                     "Initial Balance:", 0.0, 0.0, 10000.0, 2, &ok);
    if (!ok) return;
    
    bool success = bankService->createAccount(customerId, accountType, initialBalance);
    
    if (success) {
        QMessageBox::information(this, "Success", "Account created successfully.");
        refreshAccountsData();
    } else {
        QMessageBox::warning(this, "Error", "Failed to create account.");
    }
}

void AdminManagementWindow::closeSelectedAccount()
{
    int accountId = getSelectedAccountId();
    if (accountId <= 0) return;
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Close Account", 
        "Are you sure you want to close this account? This action cannot be undone.",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        bool success = bankService->closeAccount(accountId);
        
        if (success) {
            QMessageBox::information(this, "Success", "Account closed successfully.");
            refreshAccountsData();
        } else {
            QMessageBox::warning(this, "Error", "Failed to close account.");
        }
    }
}

void AdminManagementWindow::viewSelectedAccountDetails()
{
    int accountId = getSelectedAccountId();
    if (accountId <= 0) return;
    
    auto account = bankService->getAccountById(accountId);
    if (!account) return;
    
    QMessageBox::information(this, "Account Details", 
                             QString("Account Details:\n\n"
                                     "Account Number: %1\n"
                                     "Type: %2\n"
                                     "Balance: $%3\n"
                                     "Status: %4\n"
                                     "Created: %5\n\n"
                                     "Full account details would be shown here.")
                             .arg(account->getAccountNumber())
                             .arg(account->getAccountTypeDisplay())
                             .arg(QLocale().toString(account->getBalance(), 'f', 2))
                             .arg(account->isActiveStatus() ? "Active" : "Inactive")
                             .arg(account->getCreatedAt().toString("yyyy-MM-dd hh:mm:ss")));
}

void AdminManagementWindow::showStatusMessage(const QString& message, bool isError)
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

void AdminManagementWindow::setLoadingState(bool loading)
{
    isLoading = loading;
    
    if (loading) {
        setCursor(Qt::WaitCursor);
        refreshCustomersButton->setEnabled(false);
        refreshAccountsButton->setEnabled(false);
    } else {
        setCursor(Qt::ArrowCursor);
        refreshCustomersButton->setEnabled(true);
        refreshAccountsButton->setEnabled(true);
    }
}

int AdminManagementWindow::getSelectedCustomerId()
{
    int currentRow = customersTable->currentRow();
    if (currentRow >= 0) {
        return customersTable->item(currentRow, 0)->text().toInt();
    }
    return -1;
}

int AdminManagementWindow::getSelectedAccountId()
{
    int currentRow = accountsTable->currentRow();
    if (currentRow >= 0) {
        QString accountNumber = accountsTable->item(currentRow, 0)->text();
        auto account = bankService->getAccountByNumber(accountNumber);
        return account ? account->getAccountId() : -1;
    }
    return -1;
}

void AdminManagementWindow::updateCustomerButtons()
{
    bool hasSelection = customersTable->currentRow() >= 0;
    editCustomerButton->setEnabled(hasSelection && !isLoading);
    suspendCustomerButton->setEnabled(hasSelection && !isLoading);
    activateCustomerButton->setEnabled(hasSelection && !isLoading);
}

void AdminManagementWindow::updateAccountButtons()
{
    bool hasSelection = accountsTable->currentRow() >= 0;
    viewAccountDetailsButton->setEnabled(hasSelection && !isLoading);
    closeAccountButton->setEnabled(hasSelection && !isLoading);
}

void AdminManagementWindow::closeEvent(QCloseEvent* event)
{
    if (isLoading) {
        event->ignore();
        return;
    }
    QDialog::closeEvent(event);
}

void AdminManagementWindow::keyPressEvent(QKeyEvent* event)
{
    // Handle keyboard shortcuts
    if (event->key() == Qt::Key_F5) {
        if (tabWidget->currentWidget() == customersTab) {
            onRefreshCustomersClicked();
        } else {
            onRefreshAccountsClicked();
        }
    } else if (event->key() == Qt::Key_Escape && !isLoading) {
        close();
    }
    QDialog::keyPressEvent(event);
}

// Slot implementations
void AdminManagementWindow::onAddCustomerClicked()
{
    showAddCustomerDialog();
}

void AdminManagementWindow::onEditCustomerClicked()
{
    showEditCustomerDialog();
}

void AdminManagementWindow::onSuspendCustomerClicked()
{
    suspendSelectedCustomer();
}

void AdminManagementWindow::onActivateCustomerClicked()
{
    activateSelectedCustomer();
}

void AdminManagementWindow::onCustomerSelectionChanged()
{
    updateCustomerButtons();
}

void AdminManagementWindow::onCustomerSearchChanged()
{
    // Implement search functionality
    QString searchText = customerSearchEdit->text().toLower();
    
    for (int i = 0; i < customersTable->rowCount(); ++i) {
        bool match = false;
        
        for (int j = 0; j < customersTable->columnCount(); ++j) {
            QTableWidgetItem* item = customersTable->item(i, j);
            if (item && item->text().toLower().contains(searchText)) {
                match = true;
                break;
            }
        }
        
        customersTable->setRowHidden(i, !match && !searchText.isEmpty());
    }
}

void AdminManagementWindow::onRefreshCustomersClicked()
{
    refreshCustomersData();
}

void AdminManagementWindow::onCustomerDoubleClicked(int row, int column)
{
    Q_UNUSED(column);
    customersTable->selectRow(row);
    onEditCustomerClicked();
}

void AdminManagementWindow::onCreateAccountClicked()
{
    showCreateAccountDialog();
}

void AdminManagementWindow::onCloseAccountClicked()
{
    closeSelectedAccount();
}

void AdminManagementWindow::onViewAccountDetailsClicked()
{
    viewSelectedAccountDetails();
}

void AdminManagementWindow::onAccountFilterChanged()
{
    // Implement filter functionality
    QString filter = accountFilterComboBox->currentData().toString();
    
    for (int i = 0; i < accountsTable->rowCount(); ++i) {
        bool show = true;
        
        if (filter != "ALL") {
            QString status = accountsTable->item(i, 4)->text();
            QString type = accountsTable->item(i, 2)->text();
            
            if (filter == "ACTIVE" && status != "Active") {
                show = false;
            } else if (filter == "INACTIVE" && status != "Inactive") {
                show = false;
            } else if (filter == "SAVINGS" && type != "Savings Account") {
                show = false;
            } else if (filter == "CHECKING" && type != "Checking Account") {
                show = false;
            } else if (filter == "CURRENT" && type != "Current Account") {
                show = false;
            }
        }
        
        accountsTable->setRowHidden(i, !show);
    }
}

void AdminManagementWindow::onAccountSearchChanged()
{
    // Implement search functionality
    QString searchText = accountSearchEdit->text().toLower();
    
    for (int i = 0; i < accountsTable->rowCount(); ++i) {
        bool match = false;
        
        for (int j = 0; j < accountsTable->columnCount(); ++j) {
            QTableWidgetItem* item = accountsTable->item(i, j);
            if (item && item->text().toLower().contains(searchText)) {
                match = true;
                break;
            }
        }
        
        accountsTable->setRowHidden(i, !match && !searchText.isEmpty());
    }
}

void AdminManagementWindow::onRefreshAccountsClicked()
{
    refreshAccountsData();
}

void AdminManagementWindow::onAccountSelectionChanged()
{
    updateAccountButtons();
}

void AdminManagementWindow::onAccountDoubleClicked(int row, int column)
{
    Q_UNUSED(column);
    accountsTable->selectRow(row);
    onViewAccountDetailsClicked();
}

#include "gui/TransactionHistoryWindow.h"
#include "core/BankService.h"
#include "core/Account.h"
#include "core/Transaction.h"
#include <QKeyEvent>
#include <QHeaderView>
#include <QMainWindow>
#include <QMessageBox>
#include <QFileDialog>
#include <QStatusBar>
#include <QTextStream>
#include <QDate>
#include <QTimer>
#include <QGraphicsDropShadowEffect>

TransactionHistoryWindow::TransactionHistoryWindow(int userId, QWidget *parent)
    : QDialog(parent)
    , mainFrame(nullptr)
    , titleLabel(nullptr)
    , subtitleLabel(nullptr)
    , filterFrame(nullptr)
    , filterLabel(nullptr)
    , accountComboBox(nullptr)
    , dateFromLabel(nullptr)
    , dateFromEdit(nullptr)
    , dateToLabel(nullptr)
    , dateToEdit(nullptr)
    , typeLabel(nullptr)
    , typeComboBox(nullptr)
    , searchLabel(nullptr)
    , searchEdit(nullptr)
    , filterButton(nullptr)
    , resetButton(nullptr)
    , showAllCheckbox(nullptr)
    , transactionTable(nullptr)
    , countLabel(nullptr)
    , exportButton(nullptr)
    , refreshButton(nullptr)
    , mainLayout(nullptr)
    , frameLayout(nullptr)
    , filterLayout(nullptr)
    , buttonLayout(nullptr)
    , tableButtonLayout(nullptr)
    , bankService(nullptr)
    , currentUserId(userId)
    , isLoading(false)
{
    // Initialize services
    bankService = BankService::getInstance();
    
    // Setup dialog properties
    setModal(true);
    setWindowTitle("Transaction History");
    setMinimumSize(900, 700);
    resize(1000, 800);
    setWindowFlags(Qt::Dialog | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
    
    // Setup UI
    setupUI();
    setupStyles();
    
    // Load data
    loadAccounts();
    loadTransactions();
    
    // Center dialog on parent
    if (parent) {
        move(parent->window()->frameGeometry().center() - frameGeometry().center());
    }
}

TransactionHistoryWindow::~TransactionHistoryWindow()
{
}

void TransactionHistoryWindow::setupUI()
{
    // Create main frame
    mainFrame = new QFrame();
    mainFrame->setObjectName("mainFrame");
    
    // Title and subtitle
    titleLabel = new QLabel("Transaction History");
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    
    subtitleLabel = new QLabel("View and search your transaction history");
    subtitleLabel->setObjectName("subtitleLabel");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    
    // Filter section
    filterFrame = new QFrame();
    filterFrame->setObjectName("filterFrame");
    
    filterLabel = new QLabel("Filters:");
    filterLabel->setObjectName("sectionTitleLabel");
    
    accountComboBox = new QComboBox();
    accountComboBox->setObjectName("filterComboBox");
    accountComboBox->addItem("All Accounts", -1);
    
    dateFromLabel = new QLabel("From:");
    dateFromLabel->setObjectName("filterLabel");
    
    dateFromEdit = new QDateEdit();
    dateFromEdit->setObjectName("dateEdit");
    dateFromEdit->setCalendarPopup(true);
    dateFromEdit->setDate(QDate::currentDate().addDays(-30));
    
    dateToLabel = new QLabel("To:");
    dateToLabel->setObjectName("filterLabel");
    
    dateToEdit = new QDateEdit();
    dateToEdit->setObjectName("dateEdit");
    dateToEdit->setCalendarPopup(true);
    dateToEdit->setDate(QDate::currentDate());
    
    typeLabel = new QLabel("Type:");
    typeLabel->setObjectName("filterLabel");
    
    typeComboBox = new QComboBox();
    typeComboBox->setObjectName("filterComboBox");
    typeComboBox->addItem("All Types", "ALL");
    typeComboBox->addItem("Deposit", "DEPOSIT");
    typeComboBox->addItem("Withdrawal", "WITHDRAWAL");
    typeComboBox->addItem("Transfer", "TRANSFER");
    
    searchLabel = new QLabel("Search:");
    searchLabel->setObjectName("filterLabel");
    
    searchEdit = new QLineEdit();
    searchEdit->setObjectName("searchEdit");
    searchEdit->setPlaceholderText("Search by description or reference...");
    searchEdit->setMaxLength(100);
    
    filterButton = new QPushButton("🔍 Apply Filters");
    filterButton->setObjectName("filterButton");
    
    resetButton = new QPushButton("🔄 Reset");
    resetButton->setObjectName("resetButton");
    
    showAllCheckbox = new QCheckBox("Show all transactions");
    showAllCheckbox->setObjectName("showAllCheckbox");
    
    // Transaction table
    setupTransactionTable();
    
    // Bottom buttons
    countLabel = new QLabel("0 transactions found");
    countLabel->setObjectName("countLabel");
    
    exportButton = new QPushButton("📄 Export");
    exportButton->setObjectName("exportButton");
    
    refreshButton = new QPushButton("🔄 Refresh");
    refreshButton->setObjectName("refreshButton");
    
    // Setup layouts
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    
    frameLayout = new QVBoxLayout(mainFrame);
    frameLayout->setSpacing(20);
    frameLayout->setContentsMargins(30, 30, 30, 30);
    
    // Add title section
    frameLayout->addWidget(titleLabel);
    frameLayout->addWidget(subtitleLabel);
    
    // Setup filter layout
    filterLayout = new QGridLayout();
    filterLayout->setVerticalSpacing(10);
    filterLayout->setHorizontalSpacing(15);
    
    filterLayout->addWidget(filterLabel, 0, 0, 1, 6);
    filterLayout->addWidget(accountComboBox, 1, 0, 1, 2);
    filterLayout->addWidget(dateFromLabel, 1, 2);
    filterLayout->addWidget(dateFromEdit, 1, 3);
    filterLayout->addWidget(dateToLabel, 1, 4);
    filterLayout->addWidget(dateToEdit, 1, 5);
    filterLayout->addWidget(typeLabel, 2, 0);
    filterLayout->addWidget(typeComboBox, 2, 1);
    filterLayout->addWidget(searchLabel, 2, 2);
    filterLayout->addWidget(searchEdit, 2, 3, 1, 3);
    
    buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(showAllCheckbox);
    buttonLayout->addStretch();
    buttonLayout->addWidget(filterButton);
    buttonLayout->addWidget(resetButton);
    
    filterLayout->addLayout(buttonLayout, 3, 0, 1, 6);
    
    filterFrame->setLayout(filterLayout);
    frameLayout->addWidget(filterFrame);
    
    // Add transaction table
    frameLayout->addWidget(transactionTable);
    
    // Add bottom section
    tableButtonLayout = new QHBoxLayout();
    tableButtonLayout->addWidget(countLabel);
    tableButtonLayout->addStretch();
    tableButtonLayout->addWidget(refreshButton);
    tableButtonLayout->addWidget(exportButton);
    
    frameLayout->addLayout(tableButtonLayout);
    
    // Add frame to main layout
    mainLayout->addWidget(mainFrame);
    
    // Connect signals
    connect(filterButton, &QPushButton::clicked, this, &TransactionHistoryWindow::onFilterClicked);
    connect(resetButton, &QPushButton::clicked, this, &TransactionHistoryWindow::onResetClicked);
    connect(refreshButton, &QPushButton::clicked, this, &TransactionHistoryWindow::onRefreshClicked);
    connect(exportButton, &QPushButton::clicked, this, &TransactionHistoryWindow::onExportClicked);
    connect(accountComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &TransactionHistoryWindow::onAccountChanged);
    connect(searchEdit, &QLineEdit::textChanged, this, &TransactionHistoryWindow::onSearchTextChanged);
    connect(showAllCheckbox, &QCheckBox::toggled, this, &TransactionHistoryWindow::onShowAllChanged);
    connect(transactionTable, &QTableWidget::cellDoubleClicked, 
            this, &TransactionHistoryWindow::onTransactionDoubleClicked);
}

void TransactionHistoryWindow::setupStyles()
{
    QString styleSheet = R"(
        /* Main frame */
        QFrame#mainFrame {
            background: white;
            border-radius: 12px;
            border: 1px solid #e0e0e0;
        }
        
        /* Title labels */
        QLabel#titleLabel {
            font-size: 22px;
            font-weight: bold;
            color: #1a237e;
            margin-bottom: 5px;
        }
        
        QLabel#subtitleLabel {
            font-size: 14px;
            color: #5c6bc0;
            margin-bottom: 15px;
        }
        
        /* Filter frame */
        QFrame#filterFrame {
            background: #f8f9fa;
            border-radius: 8px;
            border: 1px solid #e9ecef;
            padding: 15px;
        }
        
        QLabel#sectionTitleLabel {
            font-size: 14px;
            font-weight: bold;
            color: #37474f;
            margin-bottom: 10px;
        }
        
        QLabel#filterLabel {
            font-size: 12px;
            font-weight: 600;
            color: #607d8b;
        }
        
        /* Combo boxes */
        QComboBox#filterComboBox {
            background: white;
            border: 1px solid #e0e0e0;
            border-radius: 6px;
            padding: 6px;
            font-size: 12px;
            color: #263238;
        }
        
        QComboBox#filterComboBox:focus {
            border-color: #1a237e;
            outline: none;
        }
        
        /* Date edits */
        QDateEdit#dateEdit {
            background: white;
            border: 1px solid #e0e0e0;
            border-radius: 6px;
            padding: 6px;
            font-size: 12px;
            color: #263238;
        }
        
        QDateEdit#dateEdit:focus {
            border-color: #1a237e;
            outline: none;
        }
        
        /* Line edit */
        QLineEdit#searchEdit {
            background: white;
            border: 1px solid #e0e0e0;
            border-radius: 6px;
            padding: 6px;
            font-size: 12px;
            color: #263238;
        }
        
        QLineEdit#searchEdit:focus {
            border-color: #1a237e;
            outline: none;
        }
        
        /* Checkbox */
        QCheckBox#showAllCheckbox {
            font-size: 12px;
            color: #607d8b;
        }
        
        QCheckBox#showAllCheckbox::indicator {
            width: 16px;
            height: 16px;
            border: 1px solid #e0e0e0;
            border-radius: 3px;
            background: white;
        }
        
        QCheckBox#showAllCheckbox::indicator:checked {
            background: #1a237e;
            border-color: #1a237e;
        }
        
        /* Transaction table */
        QTableWidget {
            background: white;
            border: 1px solid #e0e0e0;
            border-radius: 8px;
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
        
        /* Buttons */
        QPushButton#filterButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #1a237e, stop:1 #283593);
            color: white;
            border: none;
            border-radius: 6px;
            padding: 8px 16px;
            font-size: 12px;
            font-weight: 600;
        }
        
        QPushButton#filterButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #283593, stop:1 #3949ab);
        }
        
        QPushButton#resetButton {
            background: #78909c;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 8px 16px;
            font-size: 12px;
            font-weight: 600;
        }
        
        QPushButton#resetButton:hover {
            background: #607d8b;
        }
        
        QPushButton#exportButton {
            background: #4caf50;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 8px 16px;
            font-size: 12px;
            font-weight: 600;
        }
        
        QPushButton#exportButton:hover {
            background: #66bb6a;
        }
        
        QPushButton#refreshButton {
            background: #ff9800;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 8px 16px;
            font-size: 12px;
            font-weight: 600;
        }
        
        QPushButton#refreshButton:hover {
            background: #f57c00;
        }
        
        /* Count label */
        QLabel#countLabel {
            font-size: 12px;
            color: #607d8b;
            font-weight: 600;
        }
    )";
    
    setStyleSheet(styleSheet);
    applyModernStyling();
}

void TransactionHistoryWindow::applyModernStyling()
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

void TransactionHistoryWindow::setupTransactionTable()
{
    transactionTable = new QTableWidget();
    transactionTable->setObjectName("transactionTable");
    
    transactionTable->setColumnCount(7);
    transactionTable->setHorizontalHeaderLabels({
        "Date", "Reference", "Type", "Amount", 
        "From Account", "To Account", "Status"
    });
    
    // Configure table properties
    transactionTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    transactionTable->setSelectionMode(QAbstractItemView::SingleSelection);
    transactionTable->setAlternatingRowColors(true);
    transactionTable->verticalHeader()->setVisible(false);
    transactionTable->horizontalHeader()->setStretchLastSection(true);
    
    // Set column widths
    transactionTable->setColumnWidth(0, 120); // Date
    transactionTable->setColumnWidth(1, 120); // Reference
    transactionTable->setColumnWidth(2, 80);  // Type
    transactionTable->setColumnWidth(3, 100); // Amount
    transactionTable->setColumnWidth(4, 120); // From Account
    transactionTable->setColumnWidth(5, 120); // To Account
}

void TransactionHistoryWindow::loadAccounts()
{
    auto accounts = bankService->getUserAccounts(currentUserId);
    accountMap.clear();
    
    // Keep "All Accounts" option
    for (const auto& account : accounts) {
        if (account->isActiveStatus()) {
            QString displayText = QString("%1 - %2")
                                .arg(account->getAccountNumber())
                                .arg(account->getAccountTypeDisplay());
            
            accountComboBox->addItem(displayText, account->getAccountId());
            accountMap[account->getAccountId()] = account->getAccountNumber();
        }
    }
}

void TransactionHistoryWindow::loadTransactions()
{
    setLoadingState(true);
    
    // Get transactions based on current filters
    auto transactions = bankService->getUserTransactions(currentUserId);
    
    populateTransactionTable(transactions);
    updateTransactionCount(transactions.size());
    
    setLoadingState(false);
}

void TransactionHistoryWindow::applyFilters()
{
    setLoadingState(true);
    
    // Get all transactions first
    auto allTransactions = bankService->getUserTransactions(currentUserId);
    QList<std::shared_ptr<Transaction>> filteredTransactions;
    
    // Apply filters
    for (const auto& transaction : allTransactions) {
        bool matches = true;
        
        // Account filter
        int selectedAccountId = accountComboBox->currentData().toInt();
        if (selectedAccountId > 0) {
            if (transaction->getFromAccountId() != selectedAccountId && 
                transaction->getToAccountId() != selectedAccountId) {
                matches = false;
            }
        }
        
        // Date filter
        if (!showAllCheckbox->isChecked()) {
            QDateTime transactionDate = transaction->getTransactionDate();
            QDate transactionQDate = transactionDate.date();
            
            if (transactionQDate < dateFromEdit->date() || 
                transactionQDate > dateToEdit->date()) {
                matches = false;
            }
        }
        
        // Type filter
        QString selectedType = typeComboBox->currentData().toString();
        if (selectedType != "ALL") {
            if (transaction->getTransactionType() != selectedType) {
                matches = false;
            }
        }
        
        // Search filter
        QString searchText = searchEdit->text().trimmed();
        if (!searchText.isEmpty()) {
            QString description = transaction->getDescription().toLower();
            QString reference = transaction->getReferenceNumber().toLower();
            
            if (!description.contains(searchText.toLower()) && 
                !reference.contains(searchText.toLower())) {
                matches = false;
            }
        }
        
        if (matches) {
            filteredTransactions.append(transaction);
        }
    }
    
    populateTransactionTable(filteredTransactions);
    updateTransactionCount(filteredTransactions.size());
    
    setLoadingState(false);
}

void TransactionHistoryWindow::resetFilters()
{
    accountComboBox->setCurrentIndex(0); // All Accounts
    dateFromEdit->setDate(QDate::currentDate().addDays(-30));
    dateToEdit->setDate(QDate::currentDate());
    typeComboBox->setCurrentIndex(0); // All Types
    searchEdit->clear();
    showAllCheckbox->setChecked(false);
    
    loadTransactions();
}

void TransactionHistoryWindow::populateTransactionTable(const QList<std::shared_ptr<Transaction>>& transactions)
{
    transactionTable->setRowCount(transactions.size());
    
    for (int i = 0; i < transactions.size(); ++i) {
        auto transaction = transactions[i];
        formatTransactionRow(i, transaction);
    }
}

void TransactionHistoryWindow::formatTransactionRow(int row, std::shared_ptr<Transaction> transaction)
{
    // Date
    transactionTable->setItem(row, 0, new QTableWidgetItem(transaction->getFormattedDate()));
    
    // Reference
    transactionTable->setItem(row, 1, new QTableWidgetItem(transaction->getReferenceNumber()));
    
    // Type
    QTableWidgetItem* typeItem = new QTableWidgetItem(transaction->getTransactionTypeDisplay());
    if (transaction->isDeposit()) {
        typeItem->setForeground(QBrush(QColor("#2e7d32")));
    } else if (transaction->isWithdrawal()) {
        typeItem->setForeground(QBrush(QColor("#d32f2f")));
    } else if (transaction->isTransfer()) {
        typeItem->setForeground(QBrush(QColor("#1976d2")));
    }
    transactionTable->setItem(row, 2, typeItem);
    
    // Amount
    QString amountText = QString("$%1").arg(QLocale().toString(transaction->getAmount(), 'f', 2));
    QTableWidgetItem* amountItem = new QTableWidgetItem(amountText);
    
    if (transaction->isDeposit()) {
        amountItem->setForeground(QBrush(QColor("#2e7d32")));
    } else {
        amountItem->setForeground(QBrush(QColor("#d32f2f")));
    }
    transactionTable->setItem(row, 3, amountItem);
    
    // From Account
    QString fromAccount = "N/A";
    if (transaction->getFromAccountId() > 0) {
        auto account = bankService->getAccountById(transaction->getFromAccountId());
        if (account) {
            fromAccount = account->getAccountNumber();
        }
    }
    transactionTable->setItem(row, 4, new QTableWidgetItem(fromAccount));
    
    // To Account
    QString toAccount = "N/A";
    if (transaction->getToAccountId() > 0) {
        auto account = bankService->getAccountById(transaction->getToAccountId());
        if (account) {
            toAccount = account->getAccountNumber();
        }
    }
    transactionTable->setItem(row, 5, new QTableWidgetItem(toAccount));
    
    // Status
    QTableWidgetItem* statusItem = new QTableWidgetItem(transaction->getStatusDisplay());
    if (transaction->isCompleted()) {
        statusItem->setForeground(QBrush(QColor("#2e7d32")));
    } else if (transaction->isPending()) {
        statusItem->setForeground(QBrush(QColor("#f57c00")));
    } else {
        statusItem->setForeground(QBrush(QColor("#d32f2f")));
    }
    transactionTable->setItem(row, 6, statusItem);
}

void TransactionHistoryWindow::exportTransactions()
{
    QString fileName = QFileDialog::getSaveFileName(
        this, "Export Transactions", 
        QString("transactions_%1.csv").arg(QDate::currentDate().toString("yyyyMMdd")),
        "CSV Files (*.csv)"
    );
    
    if (fileName.isEmpty()) {
        return;
    }
    
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Export Error", "Failed to create export file.");
        return;
    }
    
    QTextStream out(&file);
    
    // Write header
    out << "Date,Reference,Type,Amount,From Account,To Account,Status\n";
    
    // Write data
    for (int i = 0; i < transactionTable->rowCount(); ++i) {
        QStringList row;
        for (int j = 0; j < transactionTable->columnCount(); ++j) {
            QTableWidgetItem* item = transactionTable->item(i, j);
            row << (item ? item->text() : "");
        }
        out << row.join(",") << "\n";
    }
    
    file.close();
    
    QMessageBox::information(this, "Export Complete", 
                             QString("Transactions exported successfully to:\n%1").arg(fileName));
}

void TransactionHistoryWindow::showStatusMessage(const QString& message, bool isError)
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

void TransactionHistoryWindow::setLoadingState(bool loading)
{
    isLoading = loading;
    
    if (loading) {
        setCursor(Qt::WaitCursor);
        filterButton->setEnabled(false);
        refreshButton->setEnabled(false);
        exportButton->setEnabled(false);
    } else {
        setCursor(Qt::ArrowCursor);
        filterButton->setEnabled(true);
        refreshButton->setEnabled(true);
        exportButton->setEnabled(true);
    }
}

void TransactionHistoryWindow::updateTransactionCount(int count)
{
    countLabel->setText(QString("%1 transaction%2 found")
                      .arg(count)
                      .arg(count == 1 ? "" : "s"));
}

void TransactionHistoryWindow::closeEvent(QCloseEvent* event)
{
    if (isLoading) {
        event->ignore();
        return;
    }
    QDialog::closeEvent(event);
}

void TransactionHistoryWindow::keyPressEvent(QKeyEvent* event)
{
    // Handle keyboard shortcuts
    if (event->key() == Qt::Key_F5) {
        onRefreshClicked();
    } else if (event->key() == Qt::Key_Escape && !isLoading) {
        close();
    }
    QDialog::keyPressEvent(event);
}

// Slot implementations
void TransactionHistoryWindow::onFilterClicked()
{
    applyFilters();
}

void TransactionHistoryWindow::onResetClicked()
{
    resetFilters();
}

void TransactionHistoryWindow::onRefreshClicked()
{
    loadTransactions();
}

void TransactionHistoryWindow::onExportClicked()
{
    exportTransactions();
}

void TransactionHistoryWindow::onAccountChanged()
{
    // Auto-apply filter when account changes
    if (!isLoading) {
        applyFilters();
    }
}

void TransactionHistoryWindow::onSearchTextChanged()
{
    // Auto-apply filter with slight delay to avoid too frequent updates
    static QTimer* searchTimer = nullptr;
    if (!searchTimer) {
        searchTimer = new QTimer(this);
        searchTimer->setSingleShot(true);
        connect(searchTimer, &QTimer::timeout, this, &TransactionHistoryWindow::applyFilters);
    }
    searchTimer->start(500); // 500ms delay
}

void TransactionHistoryWindow::onTransactionDoubleClicked(int row, int column)
{
    Q_UNUSED(column);
    
    QString reference = transactionTable->item(row, 1)->text();
    QString type = transactionTable->item(row, 2)->text();
    QString amount = transactionTable->item(row, 3)->text();
    QString status = transactionTable->item(row, 6)->text();
    
    QMessageBox::information(this, "Transaction Details", 
                             QString("Transaction Details:\n\n"
                                     "Reference: %1\n"
                                     "Type: %2\n"
                                     "Amount: %3\n"
                                     "Status: %4\n\n"
                                     "Full transaction details would be shown here.")
                             .arg(reference).arg(type).arg(amount).arg(status));
}

void TransactionHistoryWindow::onShowAllChanged()
{
    bool showAll = showAllCheckbox->isChecked();
    
    // Enable/disable date filters
    dateFromEdit->setEnabled(!showAll);
    dateToEdit->setEnabled(!showAll);
    dateFromLabel->setEnabled(!showAll);
    dateToLabel->setEnabled(!showAll);
    
    if (!isLoading) {
        applyFilters();
    }
}

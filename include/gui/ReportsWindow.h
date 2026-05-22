#ifndef REPORTSWINDOW_H
#define REPORTSWINDOW_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QComboBox>
#include <QLineEdit>
#include <QDateEdit>
#include <QProgressBar>
#include <QTabWidget>
#include <QTextEdit>

class BankService;

class ReportsWindow : public QDialog
{
    Q_OBJECT

private:
    // UI Components
    QFrame* mainFrame;
    QLabel* titleLabel;
    QTabWidget* tabWidget;
    
    // Summary tab
    QWidget* summaryTab;
    QFrame* summaryFrame;
    QLabel* summaryTitle;
    QGridLayout* summaryLayout;
    QLabel* totalCustomersLabel;
    QLabel* totalCustomersValue;
    QLabel* activeAccountsLabel;
    QLabel* activeAccountsValue;
    QLabel* totalDepositsLabel;
    QLabel* totalDepositsValue;
    QLabel* todayTransactionsLabel;
    QLabel* todayTransactionsValue;
    QLabel* failedLoginsLabel;
    QLabel* failedLoginsValue;
    QPushButton* refreshSummaryButton;
    
    // Transactions tab
    QWidget* transactionsTab;
    QFrame* transactionsFrame;
    QLabel* transactionsTitle;
    QDateEdit* transDateFromEdit;
    QDateEdit* transDateToEdit;
    QComboBox* transTypeComboBox;
    QPushButton* generateTransButton;
    QTableWidget* transactionsTable;
    QLabel* transTotalLabel;
    QLabel* transTotalValue;
    QPushButton* exportTransButton;
    
    // Customers tab
    QWidget* customersTab;
    QFrame* customersFrame;
    QLabel* customersTitle;
    QComboBox* customerStatusComboBox;
    QPushButton* generateCustomersButton;
    QTableWidget* customersTable;
    QPushButton* exportCustomersButton;
    
    // Security tab
    QWidget* securityTab;
    QFrame* securityFrame;
    QLabel* securityTitle;
    QDateEdit* secDateFromEdit;
    QDateEdit* secDateToEdit;
    QComboBox* secEventTypeComboBox;
    QPushButton* generateSecurityButton;
    QTableWidget* securityTable;
    QPushButton* exportSecurityButton;
    
    // Layouts
    QVBoxLayout* mainLayout;
    QVBoxLayout* frameLayout;
    QHBoxLayout* summaryButtonLayout;
    QHBoxLayout* transFilterLayout;
    QHBoxLayout* transButtonLayout;
    QHBoxLayout* customersFilterLayout;
    QHBoxLayout* customersButtonLayout;
    QHBoxLayout* secFilterLayout;
    QHBoxLayout* secButtonLayout;
    
    // Services
    BankService* bankService;
    
    // State variables
    bool isLoading;
    
    // UI setup
    void setupUI();
    void setupSummaryTab();
    void setupTransactionsTab();
    void setupCustomersTab();
    void setupSecurityTab();
    void setupStyles();
    void applyModernStyling();
    
    // Data loading
    void loadSummaryReport();
    void loadTransactionReport();
    void loadCustomerReport();
    void loadSecurityReport();
    
    // Table management
    void setupTransactionsTable();
    void setupCustomersTable();
    void setupSecurityTable();
    
    // Export functionality
    void exportTransactionsReport();
    void exportCustomersReport();
    void exportSecurityReport();
    void exportToCSV(QTableWidget* table, const QString& fileName);
    
    // UI helpers
    void showStatusMessage(const QString& message, bool isError = false);
    void setLoadingState(bool loading);
    void updateSummaryStatistics();

protected:
    void closeEvent(QCloseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

public:
    explicit ReportsWindow(QWidget *parent = nullptr);
    ~ReportsWindow();

private slots:
    void onRefreshSummaryClicked();
    void onGenerateTransClicked();
    void onGenerateCustomersClicked();
    void onGenerateSecurityClicked();
    void onExportTransClicked();
    void onExportCustomersClicked();
    void onExportSecurityClicked();
    void onTransTypeChanged();
    void onCustomerStatusChanged();
    void onSecEventTypeChanged();

signals:
    void reportGenerated(const QString& reportType);
};

#endif // REPORTSWINDOW_H

#ifndef ADMINMANAGEMENTWINDOW_H
#define ADMINMANAGEMENTWINDOW_H

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
#include <QProgressBar>
#include <QTabWidget>

class BankService;

class AdminManagementWindow : public QDialog
{
    Q_OBJECT

private:
    // UI Components
    QFrame* mainFrame;
    QLabel* titleLabel;
    QTabWidget* tabWidget;
    
    // Customers tab
    QWidget* customersTab;
    QFrame* customersFrame;
    QTableWidget* customersTable;
    QPushButton* addCustomerButton;
    QPushButton* editCustomerButton;
    QPushButton* suspendCustomerButton;
    QPushButton* activateCustomerButton;
    QLineEdit* customerSearchEdit;
    QPushButton* refreshCustomersButton;
    
    // Accounts tab
    QWidget* accountsTab;
    QFrame* accountsFrame;
    QTableWidget* accountsTable;
    QPushButton* createAccountButton;
    QPushButton* closeAccountButton;
    QPushButton* viewAccountDetailsButton;
    QComboBox* accountFilterComboBox;
    QLineEdit* accountSearchEdit;
    QPushButton* refreshAccountsButton;
    
    // Layouts
    QVBoxLayout* mainLayout;
    QVBoxLayout* frameLayout;
    QHBoxLayout* customersButtonLayout;
    QHBoxLayout* accountsButtonLayout;
    QHBoxLayout* customersSearchLayout;
    QHBoxLayout* accountsSearchLayout;
    
    // Services
    BankService* bankService;
    
    // State variables
    bool isLoading;
    
    // UI setup
    void setupUI();
    void setupCustomersTab();
    void setupAccountsTab();
    void setupStyles();
    void applyModernStyling();
    
    // Data loading
    void loadCustomers();
    void loadAccounts();
    void refreshCustomersData();
    void refreshAccountsData();
    
    // Table management
    void setupCustomersTable();
    void setupAccountsTable();
    void populateCustomersTable();
    void populateAccountsTable();
    
    // Customer operations
    void showAddCustomerDialog();
    void showEditCustomerDialog();
    void suspendSelectedCustomer();
    void activateSelectedCustomer();
    
    // Account operations
    void showCreateAccountDialog();
    void closeSelectedAccount();
    void viewSelectedAccountDetails();
    
    // UI helpers
    void showStatusMessage(const QString& message, bool isError = false);
    void setLoadingState(bool loading);
    void updateCustomerButtons();
    void updateAccountButtons();
    int getSelectedCustomerId();
    int getSelectedAccountId();

protected:
    void closeEvent(QCloseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

public:
    explicit AdminManagementWindow(QWidget *parent = nullptr);
    ~AdminManagementWindow();
    void showAccountsTab();
    void showCustomersTab();

private slots:
    void onAddCustomerClicked();
    void onEditCustomerClicked();
    void onSuspendCustomerClicked();
    void onActivateCustomerClicked();
    void onCustomerSelectionChanged();
    void onCustomerSearchChanged();
    void onRefreshCustomersClicked();
    void onCustomerDoubleClicked(int row, int column);
    
    void onCreateAccountClicked();
    void onCloseAccountClicked();
    void onViewAccountDetailsClicked();
    void onAccountFilterChanged();
    void onAccountSearchChanged();
    void onRefreshAccountsClicked();
    void onAccountSelectionChanged();
    void onAccountDoubleClicked(int row, int column);
};

#endif // ADMINMANAGEMENTWINDOW_H

#ifndef TRANSACTIONHISTORYWINDOW_H
#define TRANSACTIONHISTORYWINDOW_H

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
#include <QCheckBox>
#include <QProgressBar>

class BankService;

class TransactionHistoryWindow : public QDialog
{
    Q_OBJECT

private:
    // UI Components
    QFrame* mainFrame;
    QLabel* titleLabel;
    QLabel* subtitleLabel;
    
    // Filter section
    QFrame* filterFrame;
    QLabel* filterLabel;
    QComboBox* accountComboBox;
    QLabel* dateFromLabel;
    QDateEdit* dateFromEdit;
    QLabel* dateToLabel;
    QDateEdit* dateToEdit;
    QLabel* typeLabel;
    QComboBox* typeComboBox;
    QLabel* searchLabel;
    QLineEdit* searchEdit;
    QPushButton* filterButton;
    QPushButton* resetButton;
    QCheckBox* showAllCheckbox;
    
    // Transaction table
    QTableWidget* transactionTable;
    QLabel* countLabel;
    QPushButton* exportButton;
    QPushButton* refreshButton;
    
    // Layouts
    QVBoxLayout* mainLayout;
    QVBoxLayout* frameLayout;
    QGridLayout* filterLayout;
    QHBoxLayout* buttonLayout;
    QHBoxLayout* tableButtonLayout;
    
    // Services
    BankService* bankService;
    
    // State variables
    int currentUserId;
    QMap<int, QString> accountMap; // accountId -> accountNumber
    bool isLoading;
    
    // UI setup
    void setupUI();
    void setupStyles();
    void applyModernStyling();
    
    // Data loading
    void loadAccounts();
    void loadTransactions();
    void applyFilters();
    void resetFilters();
    
    // Table management
    void setupTransactionTable();
    void populateTransactionTable(const QList<std::shared_ptr<class Transaction>>& transactions);
    void formatTransactionRow(int row, std::shared_ptr<class Transaction> transaction);
    
    // Export functionality
    void exportTransactions();
    
    // UI helpers
    void showStatusMessage(const QString& message, bool isError = false);
    void setLoadingState(bool loading);
    void updateTransactionCount(int count);

protected:
    void closeEvent(QCloseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

public:
    explicit TransactionHistoryWindow(int userId, QWidget *parent = nullptr);
    ~TransactionHistoryWindow();

private slots:
    void onFilterClicked();
    void onResetClicked();
    void onRefreshClicked();
    void onExportClicked();
    void onAccountChanged();
    void onSearchTextChanged();
    void onTransactionDoubleClicked(int row, int column);
    void onShowAllChanged();

signals:
    void transactionSelected(int transactionId);
};

#endif // TRANSACTIONHISTORYWINDOW_H

#ifndef WITHDRAWALWINDOW_H
#define WITHDRAWALWINDOW_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QProgressBar>
#include <QDoubleSpinBox>

class BankService;
class AuthManager;

class WithdrawalWindow : public QDialog
{
    Q_OBJECT

private:
    // UI Components
    QFrame* mainFrame;
    QLabel* titleLabel;
    QLabel* subtitleLabel;
    QLabel* accountLabel;
    QComboBox* accountComboBox;
    QLabel* amountLabel;
    QDoubleSpinBox* amountSpinBox;
    QLabel* balanceLabel;
    QLabel* balanceValue;
    QLabel* descriptionLabel;
    QLineEdit* descriptionEdit;
    QLabel* pinLabel;
    QLineEdit* pinEdit;
    QPushButton* withdrawButton;
    QPushButton* cancelButton;
    QProgressBar* progressBar;
    QLabel* statusLabel;
    
    // Layouts
    QVBoxLayout* mainLayout;
    QVBoxLayout* frameLayout;
    QGridLayout* formLayout;
    QHBoxLayout* buttonLayout;
    
    // Services
    BankService* bankService;
    AuthManager* authManager;
    
    // State variables
    int currentUserId;
    QMap<int, QString> accountMap; // accountId -> accountNumber
    QMap<int, double> accountBalances; // accountId -> balance
    bool isProcessing;
    
    // UI setup
    void setupUI();
    void setupStyles();
    void applyModernStyling();
    
    // Data loading
    void loadAccounts();
    void updateBalanceDisplay();
    
    // Validation
    bool validateInput();
    
    // Processing
    void processWithdrawal();
    
    // UI helpers
    void showStatusMessage(const QString& message, bool isError = false);
    void setProcessingState(bool processing);
    void clearForm();

protected:
    void closeEvent(QCloseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

public:
    explicit WithdrawalWindow(int userId, QWidget *parent = nullptr);
    ~WithdrawalWindow();

private slots:
    void onWithdrawClicked();
    void onAccountChanged();
    void onAmountChanged();
    void onDescriptionChanged();
    void onPinChanged();

signals:
    void withdrawalCompleted(const QString& accountNumber, double amount);
};

#endif // WITHDRAWALWINDOW_H

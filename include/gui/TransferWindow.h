#ifndef TRANSFERWINDOW_H
#define TRANSFERWINDOW_H

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

class TransferWindow : public QDialog
{
    Q_OBJECT

private:
    // UI Components
    QFrame* mainFrame;
    QLabel* titleLabel;
    QLabel* subtitleLabel;
    QLabel* fromAccountLabel;
    QComboBox* fromAccountComboBox;
    QLabel* toAccountLabel;
    QLineEdit* toAccountEdit;
    QLabel* amountLabel;
    QDoubleSpinBox* amountSpinBox;
    QLabel* fromBalanceLabel;
    QLabel* fromBalanceValue;
    QLabel* descriptionLabel;
    QLineEdit* descriptionEdit;
    QLabel* pinLabel;
    QLineEdit* pinEdit;
    QPushButton* transferButton;
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
    bool validateTargetAccount(const QString& accountNumber);
    
    // Processing
    void processTransfer();
    
    // UI helpers
    void showStatusMessage(const QString& message, bool isError = false);
    void setProcessingState(bool processing);
    void updateTransferButtonState();
    void clearForm();

protected:
    void closeEvent(QCloseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

public:
    explicit TransferWindow(int userId, QWidget *parent = nullptr);
    ~TransferWindow();

private slots:
    void onTransferClicked();
    void onFromAccountChanged();
    void onToAccountChanged();
    void onAmountChanged();
    void onDescriptionChanged();
    void onPinChanged();

signals:
    void transferCompleted(const QString& fromAccount, const QString& toAccount, double amount);
};

#endif // TRANSFERWINDOW_H

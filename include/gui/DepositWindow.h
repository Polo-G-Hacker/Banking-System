#ifndef DEPOSITWINDOW_H
#define DEPOSITWINDOW_H

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

class DepositWindow : public QDialog
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
    QLabel* descriptionLabel;
    QLineEdit* descriptionEdit;
    QLabel* pinLabel;
    QLineEdit* pinEdit;
    QPushButton* depositButton;
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
    bool isProcessing;
    
    // UI setup
    void setupUI();
    void setupStyles();
    void applyModernStyling();
    
    // Data loading
    void loadAccounts();
    
    // Validation
    bool validateInput();
    
    // Processing
    void processDeposit();
    
    // UI helpers
    void showStatusMessage(const QString& message, bool isError = false);
    void setProcessingState(bool processing);
    void clearForm();

protected:
    void closeEvent(QCloseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

public:
    explicit DepositWindow(int userId, QWidget *parent = nullptr);
    ~DepositWindow();

private slots:
    void onDepositClicked();
    void onAccountChanged();
    void onAmountChanged();
    void onDescriptionChanged();
    void onPinChanged();

signals:
    void depositCompleted(const QString& accountNumber, double amount);
};

#endif // DEPOSITWINDOW_H

#ifndef CUSTOMERDASHBOARD_H
#define CUSTOMERDASHBOARD_H

#include <QMainWindow>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QScrollArea>
#include <QTableWidget>
#include <QProgressBar>
#include <QTimer>
#include <QMenuBar>
#include <QStatusBar>
#include <QSplitter>
#include <memory>

class BankService;
class AuthManager;
class SecurityManager;
class DepositWindow;
class WithdrawalWindow;
class TransferWindow;
class TransactionHistoryWindow;

class CustomerDashboard : public QMainWindow
{
    Q_OBJECT

private:
    // Menu and status bar
    QMenuBar* menuBar;
    QStatusBar* statusBar;
    QMenu* fileMenu;
    QMenu* accountMenu;
    QMenu* transactionMenu;
    QMenu* securityMenu;
    QMenu* helpMenu;
    
    // Main widgets
    QWidget* centralWidget;
    QSplitter* mainSplitter;
    
    // Left panel - Account summary
    QFrame* leftPanel;
    QScrollArea* leftScrollArea;
    QWidget* leftPanelContent;
    QVBoxLayout* leftPanelLayout;
    
    // User info section
    QFrame* userInfoFrame;
    QLabel* welcomeLabel;
    QLabel* usernameLabel;
    QLabel* userIdLabel;
    QLabel* lastLoginLabel;
    
    // Account summary section
    QFrame* accountSummaryFrame;
    QLabel* totalBalanceLabel;
    QLabel* totalBalanceValue;
    QProgressBar* balanceProgressBar;
    QTableWidget* accountsTable;
    
    // Quick actions section
    QFrame* quickActionsFrame;
    QPushButton* depositButton;
    QPushButton* withdrawButton;
    QPushButton* transferButton;
    QPushButton* historyButton;
    QPushButton* changePasswordButton;
    QPushButton* changePinButton;
    
    // Right panel - Recent transactions and notifications
    QFrame* rightPanel;
    QScrollArea* rightScrollArea;
    QWidget* rightPanelContent;
    QVBoxLayout* rightPanelLayout;
    
    // Recent transactions section
    QFrame* recentTransactionsFrame;
    QTableWidget* recentTransactionsTable;
    QPushButton* viewAllTransactionsButton;
    
    // Notifications section
    QFrame* notificationsFrame;
    QLabel* notificationsLabel;
    QLabel* notificationsContent;
    QPushButton* clearNotificationsButton;
    
    // Status indicators
    QLabel* sessionStatusLabel;
    QLabel* securityStatusLabel;
    QTimer* updateTimer;
    
    // Services
    BankService* bankService;
    AuthManager* authManager;
    SecurityManager* securityManager;
    
    // State variables
    int currentUserId;
    QString currentUsername;
    bool isDataLoaded;
    
    // UI setup
    void setupUI();
    void setupMenuBar();
    void setupStatusBar();
    void setupLeftPanel();
    void setupRightPanel();
    void setupStyles();
    void applyModernStyling();
    
    // Data loading and updates
    void loadUserData();
    void loadAccountSummary();
    void loadRecentTransactions();
    void updateSessionStatus();
    void updateSecurityStatus();
    void refreshAllData();
    
    // Account management
    void loadAccounts();
    void updateAccountDisplay();
    double getTotalBalance();
    
    // Transaction management
    void loadRecentTransactionsData();
    void formatTransactionTable();
    
    // Notifications
    void loadNotifications();
    void addNotification(const QString& message);
    void clearNotifications();
    
    // UI helpers
    void showDepositDialog();
    void showWithdrawalDialog();
    void showTransferDialog();
    void showTransactionHistory();
    void showChangePasswordDialog();
    void showChangePinDialog();
    void showStatusMessage(const QString& message, bool isError = false);
    void updateWelcomeMessage();

protected:
    void closeEvent(QCloseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

public:
    explicit CustomerDashboard(int userId, const QString& username, QWidget *parent = nullptr);
    ~CustomerDashboard();

private slots:
    void onDepositClicked();
    void onWithdrawClicked();
    void onTransferClicked();
    void onHistoryClicked();
    void onChangePasswordClicked();
    void onChangePinClicked();
    void onViewAllTransactionsClicked();
    void onClearNotificationsClicked();
    void onUpdateTimer();
    void onAccountDoubleClicked(int row, int column);
    void onTransactionDoubleClicked(int row, int column);
    
    // Menu actions
    void onLogout();
    void onExit();
    void onRefreshData();
    void onAbout();
    void onHelp();

signals:
    void logoutRequested();
    void sessionExpired();
};

#endif // CUSTOMERDASHBOARD_H

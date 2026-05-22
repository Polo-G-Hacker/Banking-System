#ifndef ADMINDASHBOARD_H
#define ADMINDASHBOARD_H

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
class AdminManagementWindow;
class ReportsWindow;

class AdminDashboard : public QMainWindow
{
    Q_OBJECT

private:
    // Menu and status bar
    QMenuBar* menuBar;
    QStatusBar* statusBar;
    QMenu* fileMenu;
    QMenu* userMenu;
    QMenu* accountMenu;
    QMenu* reportMenu;
    QMenu* securityMenu;
    QMenu* helpMenu;
    
    // Main widgets
    QWidget* centralWidget;
    QSplitter* mainSplitter;
    
    // Left panel - Statistics and quick actions
    QFrame* leftPanel;
    QScrollArea* leftScrollArea;
    QWidget* leftPanelContent;
    QVBoxLayout* leftPanelLayout;
    
    // Admin info section
    QFrame* adminInfoFrame;
    QLabel* welcomeLabel;
    QLabel* adminNameLabel;
    QLabel* adminRoleLabel;
    QLabel* lastLoginLabel;
    
    // Statistics section
    QFrame* statisticsFrame;
    QLabel* statisticsTitle;
    QGridLayout* statsLayout;
    QLabel* totalCustomersLabel;
    QLabel* totalCustomersValue;
    QLabel* activeCustomersLabel;
    QLabel* activeCustomersValue;
    QLabel* totalAccountsLabel;
    QLabel* totalAccountsValue;
    QLabel* totalDepositsLabel;
    QLabel* totalDepositsValue;
    QLabel* todayTransactionsLabel;
    QLabel* todayTransactionsValue;
    
    // Quick actions section
    QFrame* quickActionsFrame;
    QPushButton* manageCustomersButton;
    QPushButton* manageAccountsButton;
    QPushButton* viewReportsButton;
    QPushButton* createCustomerButton;
    QPushButton* backupDatabaseButton;
    QPushButton* viewLogsButton;
    
    // Right panel - Recent activities and system status
    QFrame* rightPanel;
    QScrollArea* rightScrollArea;
    QWidget* rightPanelContent;
    QVBoxLayout* rightPanelLayout;
    
    // Recent activities section
    QFrame* recentActivitiesFrame;
    QTableWidget* recentActivitiesTable;
    QPushButton* viewAllActivitiesButton;
    
    // System status section
    QFrame* systemStatusFrame;
    QLabel* systemStatusLabel;
    QLabel* databaseStatusLabel;
    QLabel* securityStatusLabel;
    QLabel* sessionStatusLabel;
    QProgressBar* systemHealthBar;
    
    // Status indicators
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
    void loadAdminData();
    void loadStatistics();
    void loadRecentActivities();
    void updateSystemStatus();
    void refreshAllData();
    
    // Statistics calculation
    void updateStatisticsDisplay();
    
    // Activities management
    void loadRecentActivitiesData();
    void formatActivitiesTable();
    
    // UI helpers
    void showManageCustomersDialog();
    void showManageAccountsDialog();
    void showReportsDialog();
    void showCreateCustomerDialog();
    void showBackupDialog();
    void showLogsDialog();
    void showStatusMessage(const QString& message, bool isError = false);
    void updateWelcomeMessage();

protected:
    void closeEvent(QCloseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

public:
    explicit AdminDashboard(int userId, const QString& username, QWidget *parent = nullptr);
    ~AdminDashboard();

private slots:
    void onManageCustomersClicked();
    void onManageAccountsClicked();
    void onViewReportsClicked();
    void onCreateCustomerClicked();
    void onBackupDatabaseClicked();
    void onViewLogsClicked();
    void onUpdateTimer();
    void onActivityDoubleClicked(int row, int column);
    
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

#endif // ADMINDASHBOARD_H

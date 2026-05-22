#ifndef ADMIN_H
#define ADMIN_H

#include "User.h"
#include <QList>
#include <memory>

class Customer;
class Account;

class Admin : public User {
private:
    QList<std::shared_ptr<Customer>> managedCustomers;
    QString adminLevel;
    QDateTime lastLogin;

public:
    Admin();
    Admin(int userId, const QString& username, const QString& email, 
          const QString& passwordHash, const QString& fullName,
          const QString& adminLevel = "SUPERVISOR");
    
    virtual ~Admin() = default;

    // Admin-specific getters/setters
    QString getAdminLevel() const { return adminLevel; }
    void setAdminLevel(const QString& level) { adminLevel = level; }
    QDateTime getLastLogin() const { return lastLogin; }
    void setLastLogin(const QDateTime& login) { lastLogin = login; }

    // Customer management
    void addManagedCustomer(std::shared_ptr<Customer> customer);
    void removeManagedCustomer(int customerId);
    QList<std::shared_ptr<Customer>> getManagedCustomers() const { return managedCustomers; }
    std::shared_ptr<Customer> getCustomerById(int customerId) const;
    std::shared_ptr<Customer> getCustomerByUsername(const QString& username) const;
    
    // Admin operations
    bool createCustomer(const QString& username, const QString& email, 
                       const QString& password, const QString& fullName, 
                       const QString& phoneNumber);
    bool suspendCustomer(int customerId, const QString& reason);
    bool activateCustomer(int customerId);
    bool createAccountForCustomer(int customerId, const QString& accountType, 
                                  double initialBalance = 0.0);
    bool closeAccount(int accountId);
    
    // System management
    int getTotalCustomers() const { return managedCustomers.size(); }
    int getActiveCustomers() const;
    int getSuspendedCustomers() const;
    double getTotalBankDeposits() const;
    
    // Override User methods
    bool isAdmin() const override { return true; }
    bool isCustomer() const override { return false; }
    
    // Admin permissions check
    bool canManageCustomers() const;
    bool canViewReports() const;
    bool canSuspendAccounts() const;
    bool canCreateAccounts() const;
};

#endif // ADMIN_H

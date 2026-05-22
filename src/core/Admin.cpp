#include "core/Admin.h"
#include "core/Customer.h"
#include "core/Account.h"
#include <QDebug>

Admin::Admin() : User() {
    role = "ADMIN";
    adminLevel = "SUPERVISOR";
}

Admin::Admin(int userId, const QString& username, const QString& email, 
             const QString& passwordHash, const QString& fullName,
             const QString& adminLevel)
    : User(userId, username, email, passwordHash, "", fullName, "ADMIN"),
      adminLevel(adminLevel) {
}

void Admin::addManagedCustomer(std::shared_ptr<Customer> customer) {
    if (customer) {
        managedCustomers.append(customer);
    }
}

void Admin::removeManagedCustomer(int customerId) {
    for (int i = 0; i < managedCustomers.size(); ++i) {
        if (managedCustomers[i]->getUserId() == customerId) {
            managedCustomers.removeAt(i);
            break;
        }
    }
}

std::shared_ptr<Customer> Admin::getCustomerById(int customerId) const {
    for (const auto& customer : managedCustomers) {
        if (customer->getUserId() == customerId) {
            return customer;
        }
    }
    return nullptr;
}

std::shared_ptr<Customer> Admin::getCustomerByUsername(const QString& username) const {
    for (const auto& customer : managedCustomers) {
        if (customer->getUsername() == username) {
            return customer;
        }
    }
    return nullptr;
}

bool Admin::createCustomer(const QString& username, const QString& email, 
                           const QString& password, const QString& fullName, 
                           const QString& phoneNumber) {
    if (!canManageCustomers()) {
        return false;
    }
    
    // Validate input
    if (!User::validateEmailFormat(email) || !User::validatePhoneNumber(phoneNumber)) {
        return false;
    }
    
    if (!User::validatePasswordStrength(password)) {
        return false;
    }
    
    // Check if username already exists
    for (const auto& customer : managedCustomers) {
        if (customer->getUsername() == username) {
            return false;
        }
    }
    
    // Create new customer (in a real implementation, this would save to database)
    auto newCustomer = std::make_shared<Customer>();
    newCustomer->setUsername(username);
    newCustomer->setEmail(email);
    newCustomer->setPasswordHash(User::hashPassword(password));
    newCustomer->setFullName(fullName);
    newCustomer->setPhoneNumber(phoneNumber);
    
    managedCustomers.append(newCustomer);
    return true;
}

bool Admin::suspendCustomer(int customerId, const QString& reason) {
    if (!canSuspendAccounts()) {
        return false;
    }
    
    auto customer = getCustomerById(customerId);
    if (customer) {
        customer->setAccountLocked(true);
        customer->lockAccount(1440); // Lock for 24 hours (1440 minutes)
        return true;
    }
    
    return false;
}

bool Admin::activateCustomer(int customerId) {
    auto customer = getCustomerById(customerId);
    if (customer) {
        customer->unlockAccount();
        customer->setIsActive(true);
        return true;
    }
    
    return false;
}

bool Admin::createAccountForCustomer(int customerId, const QString& accountType, 
                                     double initialBalance) {
    if (!canCreateAccounts()) {
        return false;
    }
    
    auto customer = getCustomerById(customerId);
    if (!customer) {
        return false;
    }
    
    // Create new account (in a real implementation, this would save to database)
    auto newAccount = std::make_shared<Account>();
    newAccount->setUserId(customerId);
    newAccount->setAccountType(accountType);
    newAccount->setBalance(initialBalance);
    newAccount->setAccountNumber(QString("ACC%1").arg(QDateTime::currentMSecsSinceEpoch()));
    
    customer->addAccount(newAccount);
    return true;
}

bool Admin::closeAccount(int accountId) {
    if (!canManageCustomers()) {
        return false;
    }
    
    for (auto& customer : managedCustomers) {
        auto account = customer->getAccountById(accountId);
        if (account) {
            account->setIsActive(false);
            return true;
        }
    }
    
    return false;
}

int Admin::getActiveCustomers() const {
    int count = 0;
    for (const auto& customer : managedCustomers) {
        if (customer->isActiveStatus() && !customer->isAccountLocked()) {
            count++;
        }
    }
    return count;
}

int Admin::getSuspendedCustomers() const {
    int count = 0;
    for (const auto& customer : managedCustomers) {
        if (customer->isAccountLocked()) {
            count++;
        }
    }
    return count;
}

double Admin::getTotalBankDeposits() const {
    double total = 0.0;
    for (const auto& customer : managedCustomers) {
        total += customer->getTotalBalance();
    }
    return total;
}

bool Admin::canManageCustomers() const {
    return adminLevel == "SUPERVISOR" || adminLevel == "MANAGER" || adminLevel == "ADMIN";
}

bool Admin::canViewReports() const {
    return adminLevel == "SUPERVISOR" || adminLevel == "MANAGER" || adminLevel == "ADMIN";
}

bool Admin::canSuspendAccounts() const {
    return adminLevel == "MANAGER" || adminLevel == "ADMIN";
}

bool Admin::canCreateAccounts() const {
    return adminLevel == "SUPERVISOR" || adminLevel == "MANAGER" || adminLevel == "ADMIN";
}

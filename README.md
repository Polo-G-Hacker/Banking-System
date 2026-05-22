# Banking Management System

A professional desktop banking application built with C++ and Qt Framework, featuring comprehensive security, modern UI, and robust database integration.

## 🏗️ Architecture

The system follows a **layered architecture** with clear separation of concerns:

- **Presentation Layer** (Qt GUI) - Modern, responsive user interface
- **Business Logic Layer** (C++ Classes) - Core banking operations and business rules
- **Data Access Layer** (MySQL + Qt SQL) - Database operations and persistence

## 🚀 Features

### 🔐 Security Features
- **Secure Authentication** with SHA-256 password hashing
- **Two-Factor Authentication (2FA)** with OTP verification
- **CAPTCHA Protection** (Mathematical for customers, Alphanumeric for admins)
- **Account Locking** after failed login attempts
- **Session Timeout** with automatic logout
- **Transaction PIN** for sensitive operations
- **Data Encryption** using Caesar cipher for sensitive information
- **Audit Logging** of all system activities
- **Suspicious Activity Detection**

### 👥 User Roles
- **Customers**: Access their accounts, transactions, and personal banking operations
- **Admins**: Full system management, user administration, and reporting

### 💳 Banking Operations
- **Deposit Funds** with validation and confirmation
- **Withdraw Funds** with balance verification
- **Transfer Funds** between accounts with atomic transactions
- **Transaction History** with search and filtering
- **Account Management** (create, suspend, activate accounts)

### 📊 Admin Features
- **Customer Management** (create, edit, suspend, activate)
- **Account Management** with full control
- **System Reports** (transactions, customers, security)
- **Database Backup** and restore functionality
- **Real-time Statistics** and monitoring

### 🎨 User Interface
- **Modern Professional Design** with dark blue theme
- **Responsive Layouts** that adapt to different screen sizes
- **Rounded Components** and smooth animations
- **Intuitive Navigation** with menu bars and quick actions
- **Status Indicators** and progress feedback

## 📋 System Requirements

### Prerequisites
- **Qt 6.x** (Core, Widgets, SQL, Network modules)
- **MySQL 8.0+** database server
- **CMake 3.16+** build system
- **C++17** compatible compiler

### Platform Support
- **Windows 10/11** (Primary development platform)
- **Linux** (Ubuntu 20.04+)
- **macOS** (10.15+)

## 🛠️ Installation

### 1. Database Setup
```sql
-- Create the database and import the schema
mysql -u root -p < database/schema.sql
```

### 2. Build Configuration
```bash
# Create build directory
mkdir build
cd build

# Configure with CMake
cmake .. -DCMAKE_PREFIX_PATH=/path/to/qt6

# Build the project
cmake --build . --config Release
```

### 3. Configuration
Edit `src/main.cpp` to configure your database connection:
```cpp
QString dbHostname = "localhost";
QString dbName = "banking_system";
QString dbUsername = "root";
QString dbPassword = "your_mysql_password";
```

## 🏃‍♂️ Usage

### Starting the Application
```bash
# Run the executable (Windows)
./bin/BankingSystem.exe

# Run the executable (Linux/macOS)
./bin/BankingSystem
```

### Default Credentials
- **Admin**: Username: `admin`, Password: `Admin123!@#`
- **Customer**: Username: `john_doe`, Password: `Customer123!@#`

### First Steps
1. Login with admin credentials
2. Create customer accounts through the admin dashboard
3. Test customer functionality with created accounts
4. Explore reports and system monitoring features

## 📁 Project Structure

```
Banking System/
├── CMakeLists.txt              # Main build configuration
├── README.md                   # This file
├── database/
│   └── schema.sql              # MySQL database schema
├── include/
│   ├── core/                   # Business logic headers
│   │   ├── User.h
│   │   ├── Customer.h
│   │   ├── Admin.h
│   │   ├── Account.h
│   │   ├── Transaction.h
│   │   └── BankService.h
│   ├── security/               # Security layer headers
│   │   ├── SecurityManager.h
│   │   ├── AuthManager.h
│   │   ├── DatabaseManager.h
│   │   ├── OTPManager.h
│   │   └── Logger.h
│   └── gui/                    # UI layer headers
│       ├── LoginWindow.h
│       ├── OTPDialog.h
│       ├── CustomerDashboard.h
│       ├── AdminDashboard.h
│       ├── DepositWindow.h
│       ├── WithdrawalWindow.h
│       ├── TransferWindow.h
│       ├── TransactionHistoryWindow.h
│       ├── AdminManagementWindow.h
│       └── ReportsWindow.h
├── src/
│   ├── main.cpp                # Application entry point
│   ├── core/                   # Business logic implementation
│   ├── security/               # Security layer implementation
│   └── gui/                    # UI layer implementation
└── resources/                  # Resources and assets
```

## 🔧 Configuration

### Database Connection
Configure MySQL connection in `src/main.cpp`:
- Hostname: Database server address
- Database name: `banking_system`
- Username: MySQL user with appropriate privileges
- Password: MySQL user password

### Security Settings
- Session timeout: 5 minutes (configurable)
- Account lock duration: 5 minutes after 3 failed attempts
- OTP validity: 10 minutes
- Maximum transaction amount: $100,000

## 🧪 Testing

### Unit Testing
The project structure supports unit testing. Create test files in a `tests/` directory and integrate with CMake.

### Integration Testing
Test database connectivity:
```bash
mysql -u root -p -e "USE banking_system; SHOW TABLES;"
```

### Security Testing
- Test account locking with failed login attempts
- Verify OTP generation and validation
- Test transaction PIN functionality
- Verify data encryption and decryption

## 📊 Database Schema

### Core Tables
- **users** - User accounts and authentication data
- **accounts** - Bank accounts and balances
- **transactions** - Transaction records and history
- **audit_logs** - System activity logs
- **otp_codes** - One-time password records
- **failed_logins** - Failed login attempt tracking
- **admin_actions** - Administrator activity logging
- **suspicious_activities** - Security event monitoring

## 🔒 Security Implementation

### Password Security
- SHA-256 hashing for all passwords
- Salted hashing implementation
- Password strength validation
- Secure password change workflow

### Data Protection
- Caesar cipher encryption for sensitive data
- Encrypted storage of account numbers and phone numbers
- Secure data transmission protocols
- Input validation and sanitization

### Access Control
- Role-based access control (RBAC)
- Session management with timeout
- Two-factor authentication for admin access
- Transaction PIN verification for sensitive operations

## 🚨 Security Considerations

### Production Deployment
1. **Change default passwords** immediately
2. **Configure firewall** to restrict database access
3. **Enable SSL/TLS** for database connections
4. **Regular security audits** and log monitoring
5. **Backup encryption** for sensitive data
6. **Network segmentation** for database servers

### Monitoring
- Monitor failed login attempts
- Track unusual transaction patterns
- Alert on suspicious activities
- Regular security log reviews

## 📈 Performance Optimization

### Database Optimization
- Indexed columns for frequently queried fields
- Optimized transaction queries
- Connection pooling for high-load scenarios
- Regular database maintenance

### Application Performance
- Lazy loading of large datasets
- Efficient memory management
- Asynchronous operations where appropriate
- UI responsiveness during long operations

## 🐛 Troubleshooting

### Common Issues

#### Database Connection Failed
```bash
# Check MySQL service status
sudo systemctl status mysql

# Verify database exists
mysql -u root -p -e "SHOW DATABASES;"

# Check user permissions
mysql -u root -p -e "SHOW GRANTS FOR 'user'@'host';"
```

#### Build Errors
```bash
# Verify Qt installation
qmake --version

# Check CMake configuration
cmake --version

# Verify compiler support
g++ --version
```

#### Runtime Issues
- Check database connection parameters
- Verify table permissions
- Review application logs for detailed errors

## 🤝 Contributing

### Code Style
- Follow Qt coding conventions
- Use meaningful variable names
- Add appropriate comments
- Maintain consistent formatting

### Development Workflow
1. Create feature branch
2. Implement changes with tests
3. Update documentation
4. Submit pull request for review

## 📄 License

This project is developed for educational purposes. Please ensure compliance with your institution's academic policies.

## 📞 Support

For technical support or questions:
- Review the troubleshooting section
- Check the database schema documentation
- Consult the code comments for implementation details

## 🔄 Version History

### v1.0.0 (Current)
- Complete banking system implementation
- Full security features
- Modern Qt GUI
- MySQL database integration
- Admin and customer dashboards
- Comprehensive reporting system

---

**Note**: This is an academic project designed for educational purposes. While it implements many professional banking features, it should not be used in production environments without additional security hardening and professional review.

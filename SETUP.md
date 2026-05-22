# 🚀 Banking Management System - Setup Guide

This guide provides step-by-step instructions to download, install, and configure all necessary components to run the Banking Management System.

## 📋 System Requirements

### Operating Systems
- **Windows 10/11** (Recommended)
- **Linux Ubuntu 20.04+**
- **macOS 10.15+**

### Hardware Requirements
- **RAM**: Minimum 4GB, Recommended 8GB
- **Storage**: Minimum 2GB free space
- **Processor**: Modern 64-bit CPU

## 🔧 Required Downloads & Installations

### 1. Qt Framework (Required)
**Download**: https://www.qt.io/download-qt-installer

**Version**: Qt 6.4+ (Open Source Edition)

**Components to Install**:
- ✅ Qt 6.4+ (Core, Widgets, SQL, Network modules)
- ✅ Qt Creator (IDE)
- ✅ MinGW 64-bit (Windows) or GCC (Linux/macOS)
- ✅ CMake integration

**Installation Steps**:
1. Download the Qt Online Installer
2. Run installer and select open source edition
3. Choose installation directory (recommended: `C:\Qt\6.4.0`)
4. Select required components listed above
5. Complete installation

### 2. MySQL Database Server (Recommended)
**Download**: https://dev.mysql.com/downloads/mysql/

**Version**: MySQL 8.0+

The application prefers MySQL when Qt's `QMYSQL` driver is available. If `QMYSQL` is not installed, it automatically falls back to SQLite through Qt's `QSQLITE` driver and creates a local `banking_system.sqlite` database file.

**Installation Steps** (Windows):
1. Download MySQL Community Server
2. Run installer and choose "Developer Default"
3. Set root password (remember this password!)
4. Configure MySQL as Windows Service
5. Enable "Start MySQL Server automatically"
6. Complete installation

**Installation Steps** (Linux):
```bash
sudo apt update
sudo apt install mysql-server
sudo mysql_secure_installation
```

### 3. CMake Build System (Required)
**Download**: https://cmake.org/download/

**Version**: CMake 3.16+

**Installation Steps**:
1. Download appropriate binary distribution
2. Extract to desired location
3. Add to system PATH environment variable

**Alternative (Windows)**: Install via Visual Studio Installer or Chocolatey
```powershell
choco install cmake
```

**Alternative (Linux)**:
```bash
sudo apt install cmake
```

### 4. Git Version Control (Optional but Recommended)
**Download**: https://git-scm.com/download/win

**Installation**: Follow standard installation for your OS

## 📁 Project Files Structure

You need the following files from the project. Download/create them in the exact structure shown:

```
Banking System/
├── CMakeLists.txt                    # ✅ Main build configuration
├── SETUP.md                          # ✅ This setup guide
├── README.md                          # ✅ Project documentation
├── database/
│   └── schema.sql                    # ✅ Database schema
├── include/
│   ├── core/
│   │   ├── User.h                    # ✅ User base class
│   │   ├── Customer.h                # ✅ Customer class
│   │   ├── Admin.h                   # ✅ Admin class
│   │   ├── Account.h                 # ✅ Account class
│   │   ├── Transaction.h             # ✅ Transaction class
│   │   └── BankService.h             # ✅ Main service facade
│   ├── security/
│   │   ├── SecurityManager.h         # ✅ Security utilities
│   │   ├── AuthManager.h             # ✅ Authentication manager
│   │   ├── DatabaseManager.h         # ✅ Database operations
│   │   ├── OTPManager.h              # ✅ OTP management
│   │   └── Logger.h                  # ✅ Logging system
│   └── gui/
│       ├── LoginWindow.h             # ✅ Login interface
│       ├── OTPDialog.h               # ✅ OTP verification
│       ├── CustomerDashboard.h       # ✅ Customer main interface
│       ├── AdminDashboard.h          # ✅ Admin main interface
│       ├── DepositWindow.h           # ✅ Deposit dialog
│       ├── WithdrawalWindow.h        # ✅ Withdrawal dialog
│       ├── TransferWindow.h          # ✅ Transfer dialog
│       ├── TransactionHistoryWindow.h # ✅ Transaction history
│       ├── AdminManagementWindow.h   # ✅ User management
│       └── ReportsWindow.h            # ✅ Reports interface
├── src/
│   ├── main.cpp                      # ✅ Application entry point
│   ├── core/
│   │   ├── User.cpp                  # ✅ User implementation
│   │   ├── Customer.cpp              # ✅ Customer implementation
│   │   ├── Admin.cpp                 # ✅ Admin implementation
│   │   ├── Account.cpp               # ✅ Account implementation
│   │   ├── Transaction.cpp           # ✅ Transaction implementation
│   │   └── BankService.cpp           # ✅ Service implementation
│   ├── security/
│   │   ├── SecurityManager.cpp       # ✅ Security implementation
│   │   ├── AuthManager.cpp           # ✅ Auth implementation
│   │   ├── DatabaseManager.cpp       # ✅ Database implementation
│   │   ├── OTPManager.cpp            # ✅ OTP implementation
│   │   └── Logger.cpp                # ✅ Logger implementation
│   └── gui/
│       ├── LoginWindow.cpp           # ✅ Login implementation
│       ├── OTPDialog.cpp             # ✅ OTP implementation
│       ├── CustomerDashboard.cpp     # ✅ Customer dashboard
│       ├── AdminDashboard.cpp        # ✅ Admin dashboard
│       ├── DepositWindow.cpp         # ✅ Deposit implementation
│       ├── WithdrawalWindow.cpp      # ✅ Withdrawal implementation
│       ├── TransferWindow.cpp        # ✅ Transfer implementation
│       ├── TransactionHistoryWindow.cpp # ✅ History implementation
│       ├── AdminManagementWindow.cpp  # ✅ Management implementation
│       └── ReportsWindow.cpp         # ✅ Reports implementation
└── resources/
    ├── icons/                        # 📁 (optional) Application icons
    ├── styles/                       # 📁 (optional) Additional stylesheets
    └── images/                       # 📁 (optional) UI images
```

## 🛠️ Step-by-Step Setup

### Step 1: Install Prerequisites
1. Install Qt Framework (with required modules)
2. Install MySQL Server if you want to use MySQL instead of the SQLite fallback
3. Install CMake
4. (Optional) Install Git

### Step 2: Setup Database

#### Option A: MySQL
1. Start MySQL service
2. Create database and import schema:
```bash
# Windows (Command Prompt)
mysql -u root -p < "c:\Users\USER\Desktop\School\C++\Banking System\database\schema.sql"

# Linux/macOS
mysql -u root -p < "/path/to/Banking System/database/schema.sql"
```
3. Verify database creation:
```sql
mysql -u root -p
USE banking_system;
SHOW TABLES;
```

#### Option B: SQLite fallback
No manual database setup is required. If Qt cannot load the `QMYSQL` driver, the app uses `QSQLITE` and initializes the local SQLite database automatically.

### Step 3: Configure Database Connection
Edit `src/main.cpp` and update these lines with your MySQL credentials if you are using MySQL. These values are ignored by the SQLite fallback:

```cpp
// Line 15-18 in main.cpp
QString dbHostname = "localhost";        // ✅ Keep as localhost
QString dbName = "banking_system";       // ✅ Keep as is
QString dbUsername = "root";             // ✅ Update if different
QString dbPassword = "adrien@12 "; // ✅ UPDATE THIS!
```

### Step 4: Build the Application

#### Method A: Using Qt Creator (Recommended)
1. Open Qt Creator
2. File → Open Project → Select `CMakeLists.txt`
3. Configure Kit (Qt 6.x + your compiler)
4. Build → Build Project (Ctrl+B)
5. Run → Run (Ctrl+R)

#### Method B: Using Command Line
```bash
# Navigate to project directory
cd "c:\Users\USER\Desktop\School\C++\Banking System"

# Create build directory
mkdir build
cd build

# Configure with CMake
cmake .. -DCMAKE_PREFIX_PATH="C:\Qt\6.4.0\msvc2019_64"

# Build the project
cmake --build . --config Release

# Run the application
.\Release\BankingSystem.exe
```

### Step 5: Test the Application
1. Launch the application
2. Login with admin credentials:
   - Username: `admin`
   - Password: `Admin123!@#`
3. Verify dashboard loads correctly
4. Test creating a customer account
5. Test customer login and basic operations

## 🔧 Configuration Files

### CMakeLists.txt (Already Provided)
- ✅ Qt6 modules: Core, Widgets, Sql, Network
- ✅ All source and header files included
- ✅ Proper linking and compilation flags

### Database Schema (Already Provided)
- ✅ All required tables created
- ✅ Sample data for admin and test customer
- ✅ Proper indexes and constraints

## 🚨 Common Setup Issues & Solutions

### Issue 1: "Qt not found" Error
**Solution**: 
- Verify Qt installation path
- Update `CMAKE_PREFIX_PATH` to correct Qt location
- Ensure Qt6 is installed (not Qt5)

### Issue 2: Database Connection Failed
**Solution**:
- If using MySQL, verify the MySQL service is running
- If using MySQL, check database credentials in main.cpp
- If using MySQL, ensure `banking_system` database exists
- Test with MySQL command line first when using MySQL
- Confirm that Qt has either the `QMYSQL` or `QSQLITE` SQL driver available

### Issue 3: Build Errors
**Solution**:
- Verify all files are present in correct directories
- Check CMakeLists.txt includes all source files
- Ensure compiler is properly installed

### Issue 4: Runtime Crashes
**Solution**:
- Check database connection before operations
- Verify all required tables exist
- Review application logs for specific errors

## 📦 Quick Setup Checklist

- [ ] Qt 6.x installed with required modules
- [ ] MySQL 8.0+ installed and running, or SQLite fallback available through Qt
- [ ] CMake 3.16+ installed
- [ ] Database schema imported if using MySQL
- [ ] Database credentials configured if using MySQL
- [ ] All project files present
- [ ] Application builds successfully
- [ ] Admin login works
- [ ] Customer operations functional

## 🎯 Next Steps After Setup

1. **Explore Features**: Test all banking operations
2. **Create Test Data**: Add sample customers and accounts
3. **Security Testing**: Test login attempts and OTP system
4. **Generate Reports**: Try admin reporting features
5. **Database Backup**: Test backup functionality

## 📞 Support Resources

### Documentation
- `README.md`: Complete feature documentation
- `SETUP.md`: This setup guide
- Code comments: Inline documentation in source files

### Troubleshooting
- Check MySQL service status if using MySQL
- Verify Qt installation paths
- Review build logs for specific errors
- Test database connection separately

---

**🎉 Congratulations!** Once you complete all these steps, your Banking Management System will be fully operational with all features working correctly.

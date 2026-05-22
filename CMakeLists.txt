cmake_minimum_required(VERSION 3.16)

project(BankingSystem VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql Network)

# Enable Qt MOC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/core
    ${CMAKE_CURRENT_SOURCE_DIR}/include/security
    ${CMAKE_CURRENT_SOURCE_DIR}/include/gui
)

# Source files for login screen and database connection
set(SOURCES
    src/main.cpp
    src/core/User.cpp
    src/core/Account.cpp
    src/core/Transaction.cpp
    src/core/Customer.cpp
    src/core/Admin.cpp
    src/core/BankService.cpp
    src/security/DatabaseManager.cpp
    src/security/AuthManager.cpp
    src/security/SecurityManager.cpp
    src/security/OTPManager.cpp
    src/security/Logger.cpp
    src/gui/LoginWindow.cpp
    src/gui/CustomerDashboard.cpp
    src/gui/AdminDashboard.cpp
    src/gui/DepositWindow.cpp
    src/gui/WithdrawalWindow.cpp
    src/gui/TransferWindow.cpp
    src/gui/TransactionHistoryWindow.cpp
    src/gui/AdminManagementWindow.cpp
    src/gui/ReportsWindow.cpp
    src/gui/OTPDialog.cpp
)

# Header files
set(HEADERS
    include/core/User.h
    include/core/Account.h
    include/core/Transaction.h
    include/core/Customer.h
    include/core/Admin.h
    include/core/BankService.h
    include/security/DatabaseManager.h
    include/security/AuthManager.h
    include/security/SecurityManager.h
    include/security/OTPManager.h
    include/security/Logger.h
    include/gui/LoginWindow.h
    include/gui/CustomerDashboard.h
    include/gui/AdminDashboard.h
    include/gui/DepositWindow.h
    include/gui/WithdrawalWindow.h
    include/gui/TransferWindow.h
    include/gui/TransactionHistoryWindow.h
    include/gui/AdminManagementWindow.h
    include/gui/ReportsWindow.h
    include/gui/OTPDialog.h
)

# Create executable
add_executable(BankingSystem ${SOURCES} ${HEADERS})

# Link Qt libraries
target_link_libraries(BankingSystem 
    Qt6::Core 
    Qt6::Widgets
    Qt6::Sql
    Qt6::Network
)

# Set Windows subsystem
if(WIN32)
    set_target_properties(BankingSystem PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

-- Banking System Database Schema
-- MySQL Database

CREATE DATABASE IF NOT EXISTS banking_system;
USE banking_system;

-- Users table
CREATE TABLE IF NOT EXISTS users (
    user_id INT PRIMARY KEY AUTO_INCREMENT,
    username VARCHAR(50) UNIQUE NOT NULL,
    email VARCHAR(100) UNIQUE NOT NULL,
    password_hash VARCHAR(256) NOT NULL,
    phone_number VARCHAR(255),
    full_name VARCHAR(100) NOT NULL,
    role ENUM('CUSTOMER', 'ADMIN') NOT NULL,
    transaction_pin VARCHAR(256),
    account_locked BOOLEAN DEFAULT FALSE,
    lock_until DATETIME NULL,
    failed_login_attempts INT DEFAULT 0,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    is_active BOOLEAN DEFAULT TRUE
);

-- Accounts table
CREATE TABLE IF NOT EXISTS accounts (
    account_id INT PRIMARY KEY AUTO_INCREMENT,
    account_number VARCHAR(255) UNIQUE NOT NULL,
    user_id INT NOT NULL,
    account_type ENUM('SAVINGS', 'CHECKING', 'CURRENT') NOT NULL,
    balance DECIMAL(15, 2) DEFAULT 0.00,
    is_active BOOLEAN DEFAULT TRUE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE
);

-- Transactions table
CREATE TABLE IF NOT EXISTS transactions (
    transaction_id INT PRIMARY KEY AUTO_INCREMENT,
    from_account_id INT,
    to_account_id INT,
    amount DECIMAL(15, 2) NOT NULL,
    transaction_type ENUM('DEPOSIT', 'WITHDRAWAL', 'TRANSFER') NOT NULL,
    description TEXT,
    transaction_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    status ENUM('PENDING', 'COMPLETED', 'FAILED', 'CANCELLED') DEFAULT 'COMPLETED',
    reference_number VARCHAR(50) UNIQUE NOT NULL,
    FOREIGN KEY (from_account_id) REFERENCES accounts(account_id) ON DELETE SET NULL,
    FOREIGN KEY (to_account_id) REFERENCES accounts(account_id) ON DELETE SET NULL
);

-- Audit Logs table
CREATE TABLE IF NOT EXISTS audit_logs (
    log_id INT PRIMARY KEY AUTO_INCREMENT,
    user_id INT,
    action VARCHAR(100) NOT NULL,
    description TEXT,
    ip_address VARCHAR(45),
    user_agent TEXT,
    status ENUM('SUCCESS', 'FAILED', 'WARNING') NOT NULL,
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE SET NULL
);

-- OTP Codes table
CREATE TABLE IF NOT EXISTS otp_codes (
    otp_id INT PRIMARY KEY AUTO_INCREMENT,
    user_id INT NOT NULL,
    otp_code VARCHAR(10) NOT NULL,
    purpose ENUM('LOGIN', 'TRANSACTION', 'PASSWORD_RESET') NOT NULL,
    is_used BOOLEAN DEFAULT FALSE,
    expires_at TIMESTAMP NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE
);

-- Failed Login Attempts table
CREATE TABLE IF NOT EXISTS failed_login_attempts (
    failed_id INT PRIMARY KEY AUTO_INCREMENT,
    username VARCHAR(50) NOT NULL,
    ip_address VARCHAR(45),
    attempt_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    reason VARCHAR(100)
);

-- Admin Actions table
CREATE TABLE IF NOT EXISTS admin_actions (
    action_id INT PRIMARY KEY AUTO_INCREMENT,
    admin_id INT NOT NULL,
    target_user_id INT,
    action_type ENUM('CREATE_USER', 'SUSPEND_ACCOUNT', 'ACTIVATE_ACCOUNT', 'CREATE_ACCOUNT', 'MODIFY_ACCOUNT') NOT NULL,
    description TEXT,
    action_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (admin_id) REFERENCES users(user_id) ON DELETE CASCADE,
    FOREIGN KEY (target_user_id) REFERENCES users(user_id) ON DELETE SET NULL
);

-- Suspicious Activities table
CREATE TABLE IF NOT EXISTS suspicious_activities (
    activity_id INT PRIMARY KEY AUTO_INCREMENT,
    user_id INT,
    activity_type VARCHAR(100) NOT NULL,
    description TEXT,
    severity ENUM('LOW', 'MEDIUM', 'HIGH') NOT NULL,
    detected_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    is_resolved BOOLEAN DEFAULT FALSE,
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE SET NULL
);

-- Email Notifications table
CREATE TABLE IF NOT EXISTS email_notifications (
    notification_id INT PRIMARY KEY AUTO_INCREMENT,
    user_id INT NOT NULL,
    email_type ENUM('TRANSACTION', 'LOGIN_ALERT', 'ACCOUNT_CREATED', 'PASSWORD_RESET', 'SUSPICIOUS_ACTIVITY') NOT NULL,
    subject VARCHAR(200) NOT NULL,
    body TEXT NOT NULL,
    sent_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    is_sent BOOLEAN DEFAULT FALSE,
    FOREIGN KEY (user_id) REFERENCES users(user_id) ON DELETE CASCADE
);

-- Insert default admin user (password: Admin123!@#)
INSERT IGNORE INTO users (username, email, password_hash, full_name, role, transaction_pin) VALUES 
('admin', 'admin@bank.com', SHA2('Admin123!@#', 256), 'System Administrator', 'ADMIN', SHA2('1234', 256));

-- Insert sample customer for testing
INSERT IGNORE INTO users (username, email, password_hash, phone_number, full_name, role, transaction_pin) VALUES 
('john_doe', 'john@example.com', SHA2('Customer123!@#', 256), '+1234567890', 'John Doe', 'CUSTOMER', SHA2('5678', 256));

-- Create sample account for the customer
INSERT IGNORE INTO accounts (account_number, user_id, account_type, balance) VALUES 
('ACC100001', 2, 'SAVINGS', 5000.00);

-- Create indexes for better performance
CREATE INDEX idx_users_username ON users(username);
CREATE INDEX idx_users_email ON users(email);
CREATE INDEX idx_accounts_user_id ON accounts(user_id);
CREATE INDEX idx_accounts_account_number ON accounts(account_number);
CREATE INDEX idx_transactions_from_account ON transactions(from_account_id);
CREATE INDEX idx_transactions_to_account ON transactions(to_account_id);
CREATE INDEX idx_transactions_date ON transactions(transaction_date);
CREATE INDEX idx_audit_logs_user_id ON audit_logs(user_id);
CREATE INDEX idx_audit_logs_timestamp ON audit_logs(timestamp);
CREATE INDEX idx_otp_codes_user_id ON otp_codes(user_id);
CREATE INDEX idx_failed_login_attempts_username ON failed_login_attempts(username);
CREATE INDEX idx_suspicious_activities_user_id ON suspicious_activities(user_id);

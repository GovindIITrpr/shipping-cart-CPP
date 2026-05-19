# Dragon Shopping Cart

A RESTful shopping cart API built in C++ with PostgreSQL database persistence. Features a custom HTTP server framework (Dragon) and implements the MVC architectural pattern.

## 📋 Table of Contents

- [⚡ Quick Start (5 minutes)](#-quick-start-5-minutes)
- [Features](#features)
- [System Requirements](#system-requirements)
- [Prerequisites](#prerequisites)
- [Installation & Setup](#installation--setup)
- [Building the Project](#building-the-project)
- [Database Setup](#database-setup)
- [Running the Server](#running-the-server)
- [API Endpoints](#api-endpoints)
- [Testing](#testing)
- [Troubleshooting](#troubleshooting)
- [Project Structure](#project-structure)

---

## ⚡ Quick Start (5 minutes)

**Do this in PowerShell (Run as Administrator):**

```powershell
# 1. Navigate to project
cd C:\Users\YourUsername\Desktop\C-Project\dragon-shopping-cart

# 2. Start PostgreSQL service
Start-Service -Name postgresql-x64-18

# 3. Setup database (replace 5433 if your port is different)
$port = 5432
& "C:\Program Files\PostgreSQL\18\bin\psql" -h localhost -p $port -U postgres -tc "SELECT 1 FROM pg_database WHERE datname = 'dragon_shop'" | Select-Object -First 1 | ForEach-Object { if ($_ -notlike "1") { & "C:\Program Files\PostgreSQL\18\bin\psql" -h localhost -p $port -U postgres -c "CREATE DATABASE dragon_shop;" } }
& "C:\Program Files\PostgreSQL\18\bin\psql" -h localhost -p $port -U postgres -d dragon_shop -f "sql\migrations\001_create_tables.sql"

# 4. Clean and build (first time only, or after changing dependencies)
rmdir /s /q build -ErrorAction SilentlyContinue
mkdir build
cd build

# 5. Configure CMake with vcpkg
# Replace C:\path\to\vcpkg with your actual vcpkg path
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\path\to\vcpkg\scripts\buildsystems\vcpkg.cmake -G "Visual Studio 17 2022" -A x64

# 6. Build
cmake --build . --config Release

# 7. Run the server
cd Release
.\dragon_shopping_cart.exe
```

**In a new PowerShell window, test the API:**

```powershell
# Get all cart items
Invoke-WebRequest -Uri "http://127.0.0.1:8081/cart" -Method GET
```

**Expected response:** You should see Laptop, Mouse, and Keyboard items.

> **Stuck?** See the [Troubleshooting](#troubleshooting) section. Most issues are:
> - PostgreSQL port is wrong (check with: `Get-Service postgresql-x64-18`)
> - vcpkg path is incorrect
> - Database not created (run Step 3 again)

---

## ✨ Features

- **RESTful API**: Full CRUD operations for shopping cart items
- **PostgreSQL Integration**: Persistent data storage with libpqxx
- **Custom HTTP Framework**: Dragon framework with request/response parsing
- **MVC Architecture**: Clean separation of concerns
- **JSON Support**: Built-in JSON parsing with nlohmann/json
- **Cross-Platform**: Windows-compatible C++17 implementation
- **Graceful Degradation**: Falls back to in-memory storage if database unavailable

## 🖥️ System Requirements

- **OS**: Windows 10 or later
- **CPU**: Any modern processor (x64 architecture)
- **RAM**: Minimum 2GB (recommended 4GB)
- **Disk**: At least 1GB free space for dependencies and build artifacts

## 📦 Prerequisites

Before setting up the project, install the following:

### 1. **PostgreSQL 18**
   - Download from https://www.postgresql.org/download/windows/
   - During installation, note the port (default 5432, but may be 5433 if port 5432 is in use)
   - Set a password for the `postgres` user (default: `postgres`)
   - **Important**: Remember the port number - you'll need it for configuration

### 2. **Visual Studio Build Tools 2019 or Later**
   - Download from https://visualstudio.microsoft.com/downloads/
   - Install the "Desktop development with C++" workload
   - Includes CMake, MSVC compiler, and necessary libraries

### 3. **CMake 3.10 or Later**
   - Download from https://cmake.org/download/
   - Add to PATH during installation

### 4. **vcpkg** (C++ Package Manager)
   - Clone the repository: `git clone https://github.com/Microsoft/vcpkg.git`
   - Navigate to the directory and run: `.\vcpkg\bootstrap-vcpkg.bat`
   - Note the installation path for later configuration

### 5. **Git** (optional but recommended)
   - Download from https://git-scm.com/download/win

## 🚀 Installation & Setup

### ✅ Verify Prerequisites First

Open PowerShell **as Administrator** and verify each is installed:

```powershell
# Check CMake
cmake --version  # Should be 3.10+

# Check Visual Studio Build Tools
# If this fails, reinstall Visual Studio Build Tools with C++ workload
where cl.exe  # Should show C:\Program Files\...\cl.exe

# Check PostgreSQL
Get-Service -Name postgresql-x64-18  # Should show "Running"

# If PostgreSQL is not running, start it:
Start-Service -Name postgresql-x64-18

# Check vcpkg exists
Test-Path C:\path\to\vcpkg\bootstrap-vcpkg.bat  # Replace with YOUR vcpkg path
```

### Step 1: Locate Your vcpkg Installation Path
```powershell
# Find where vcpkg is installed
Get-ChildItem -Path "C:\" -Recurse -Filter "bootstrap-vcpkg.bat" -ErrorAction SilentlyContinue | Select-Object -First 1
```
**Note this path** - you'll need it for CMake configuration (e.g., `C:\Users\YourName\vcpkg` or `C:\vcpkg`)

### Step 2: Find Your PostgreSQL Port
```powershell
# PostgreSQL uses either 5432 or 5433
# Try to connect to determine the port
& "C:\Program Files\PostgreSQL\18\bin\psql" -h localhost -p 5432 -U postgres -c "SELECT version();" 2>$null
if ($LASTEXITCODE -ne 0) {
    Write-Host "Port 5432 failed, trying 5433..."
    & "C:\Program Files\PostgreSQL\18\bin\psql" -h localhost -p 5433 -U postgres -c "SELECT version();"
    if ($LASTEXITCODE -eq 0) { Write-Host "PostgreSQL is on port 5433" }
}
```
**Note this port** - you'll need it for the build.

### Step 3: Create Build Directory
```powershell
cd C:\Users\YourUsername\Desktop\C-Project\dragon-shopping-cart
mkdir build
cd build
```

### Step 4: Run CMake Configuration
```powershell
# Replace:
# - C:\path\to\vcpkg with your actual vcpkg path from Step 1
# - 5433 with your PostgreSQL port from Step 2 (if different)

cmake .. `
  -DCMAKE_TOOLCHAIN_FILE="C:\path\to\vcpkg\scripts\buildsystems\vcpkg.cmake" `
  -G "Visual Studio 17 2022" `
  -A x64
```

**Expected output:** Should end with "Build files have been written to..."
If this fails, see the [Troubleshooting](#troubleshooting) section.

## 📦 Building the Project

### Build Release Executable:
```powershell
# From your build directory
cmake --build . --config Release
```

**This will take 2-5 minutes on first build.**

### Verify Build Success:
```powershell
# Check if executable exists
Test-Path .\Release\dragon_shopping_cart.exe  # Should return True

# List the executable
Get-Item .\Release\dragon_shopping_cart.exe | Select-Object FullName, Length
```

**Expected:** File should exist and be 2-5 MB in size.

### If Build Fails:
See [Common Build Issues](#troubleshooting) in the Troubleshooting section.

## 🗄️ Database Setup

### Step 1: Ensure PostgreSQL is Running
```powershell
Get-Service -Name postgresql-x64-18  # Should show "Running"

# If stopped, start it
Start-Service -Name postgresql-x64-18
```

### Step 2: Create Database and Load Schema
```powershell
# Set your PostgreSQL port (find it from Installation Step 2)
$port = 5433  # Change to 5432 if that's your port

# Create database
& "C:\Program Files\PostgreSQL\18\bin\psql" -h localhost -p $port -U postgres -c "CREATE DATABASE dragon_shop;"

# Load schema with sample data
& "C:\Program Files\PostgreSQL\18\bin\psql" -h localhost -p $port -U postgres -d dragon_shop -f "sql\migrations\001_create_tables.sql"
```

**Expected output:** Should show `DROP TABLE`, `CREATE TABLE`, `CREATE INDEX`, and `INSERT 0 3`.

### Step 3: Verify Database Setup
```powershell
$port = 5433  # Use your port
& "C:\Program Files\PostgreSQL\18\bin\psql" -h localhost -p $port -U postgres -d dragon_shop -c "SELECT COUNT(*) as item_count FROM cart_items;"
```

**Expected:** Should show `item_count: 3` (Laptop, Mouse, Keyboard)

## ▶️ Running the Server

### Start the Server:
```powershell
cd C:\Users\YourUsername\Desktop\C-Project\dragon-shopping-cart\build\Release

.\dragon_shopping_cart.exe
```

### Expected Output:
```
Dragon framework initialized
Connected to PostgreSQL database successfully
Database initialized successfully
Server created on port 8081
Server started on port 8081
Press Ctrl+C to stop the server...
Listening on http://127.0.0.1:8081
```

**The server is now ready for requests!**

### If You See Connection Errors:
1. **"Connection refused"** → PostgreSQL is not running. Run: `Start-Service -Name postgresql-x64-18`
2. **"Password authentication failed"** → Check your PostgreSQL port (5432 or 5433) and try the correct one
3. **"dragon_shop database doesn't exist"** → Re-run Database Setup Step 2

### Stop the Server:
Press **Ctrl+C** in the terminal running the server.

## 📡 API Endpoints

All endpoints return JSON responses and expect JSON request bodies (except GET).

### Base URL
```
http://127.0.0.1:8081
```

### 1. **GET /cart** - Retrieve All Items
```bash
# Using PowerShell
Invoke-WebRequest -Uri "http://127.0.0.1:8081/cart" -Method GET

# Using curl
curl http://127.0.0.1:8081/cart
```

**Response (200 OK):**
```json
{
  "items": [
    {
      "id": 1,
      "name": "Laptop",
      "price": 999.99,
      "quantity": 1
    }
  ]
}
```

### 2. **POST /cart/add** - Add Item to Cart
```bash
# Using PowerShell
$body = @{
    id = 4
    name = "Monitor"
    price = 299.99
    quantity = 1
} | ConvertTo-Json

Invoke-WebRequest -Uri "http://127.0.0.1:8081/cart/add" `
  -Method POST `
  -Headers @{"Content-Type"="application/json"} `
  -Body $body

# Using curl
curl -X POST http://127.0.0.1:8081/cart/add `
  -H "Content-Type: application/json" `
  -d '{"id":4,"name":"Monitor","price":299.99,"quantity":1}'
```

**Request Body:**
```json
{
  "id": 4,
  "name": "Monitor",
  "price": 299.99,
  "quantity": 1
}
```

**Response (201 Created):**
```json
{
  "status": "ok",
  "item_id": 4
}
```

### 3. **DELETE /cart/remove/:id** - Remove Item from Cart
```bash
# Using PowerShell
Invoke-WebRequest -Uri "http://127.0.0.1:8081/cart/remove/4" -Method DELETE

# Using curl
curl -X DELETE http://127.0.0.1:8081/cart/remove/4
```

**Response (200 OK):**
```json
{
  "status": "ok",
  "message": "Item removed"
}
```

### 4. **PUT /cart/update/:id** - Update Item
```bash
# Using PowerShell
$body = @{
    name = "Updated Monitor"
    price = 349.99
    quantity = 2
} | ConvertTo-Json

Invoke-WebRequest -Uri "http://127.0.0.1:8081/cart/update/4" `
  -Method PUT `
  -Headers @{"Content-Type"="application/json"} `
  -Body $body

# Using curl
curl -X PUT http://127.0.0.1:8081/cart/update/4 `
  -H "Content-Type: application/json" `
  -d '{"name":"Updated Monitor","price":349.99,"quantity":2}'
```

**Request Body:**
```json
{
  "name": "Updated Monitor",
  "price": 349.99,
  "quantity": 2
}
```

**Response (200 OK):**
```json
{
  "status": "ok",
  "message": "Item updated"
}
```

## 🧪 Testing

### Test All Endpoints (Quick Test)
```bash
# 1. View initial items
Invoke-WebRequest -Uri "http://127.0.0.1:8081/cart" -Method GET | ConvertTo-Json

# 2. Add a new item
$newItem = @{id=4; name="Monitor"; price=299.99; quantity=1} | ConvertTo-Json
Invoke-WebRequest -Uri "http://127.0.0.1:8081/cart/add" -Method POST `
  -Headers @{"Content-Type"="application/json"} -Body $newItem

# 3. View items again to verify insertion
Invoke-WebRequest -Uri "http://127.0.0.1:8081/cart" -Method GET | ConvertTo-Json

# 4. Update the new item
$updateItem = @{name="Gaming Monitor"; price=449.99; quantity=2} | ConvertTo-Json
Invoke-WebRequest -Uri "http://127.0.0.1:8081/cart/update/4" -Method PUT `
  -Headers @{"Content-Type"="application/json"} -Body $updateItem

# 5. Remove the item
Invoke-WebRequest -Uri "http://127.0.0.1:8081/cart/remove/4" -Method DELETE
```

## 🔧 Troubleshooting

### Issue: CMake configuration fails - "vcpkg toolchain not found"
**Cause:** Incorrect vcpkg path  
**Solution:**
```powershell
# Find your vcpkg installation
Get-ChildItem -Path "C:\" -Recurse -Filter "bootstrap-vcpkg.bat" -ErrorAction SilentlyContinue | ForEach-Object { $_.Directory }

# Once found, update the CMake command with the correct path:
cmake .. -DCMAKE_TOOLCHAIN_FILE="C:\YOUR_ACTUAL_VCPKG_PATH\scripts\buildsystems\vcpkg.cmake" -G "Visual Studio 17 2022" -A x64
```

### Issue: CMake configuration fails - "libpqxx not found"
**Cause:** Dependency not installed via vcpkg  
**Solution:**
```powershell
# Install dependencies
cd C:\YOUR_VCPKG_PATH
.\vcpkg install libpqxx:x64-windows
.\vcpkg install nlohmann-json:x64-windows

# Then retry CMake configuration
```

### Issue: Build fails with compiler errors
**Cause:** Missing Visual Studio Build Tools or old version  
**Solution:**
```powershell
# Verify compiler is installed
where cl.exe  # Should show path to cl.exe

# If not found, download and install Visual Studio Build Tools 2022:
# https://visualstudio.microsoft.com/downloads/
# Select "Desktop development with C++" workload
```

### Issue: Server fails to start - "Connection refused: 127.0.0.1:5432"
**Cause:** PostgreSQL is not running or wrong port  
**Solution:**
```powershell
# Check PostgreSQL service
Get-Service postgresql-x64-18

# Start it if stopped
Start-Service -Name postgresql-x64-18

# Verify connection with correct port (5432 or 5433)
& "C:\Program Files\PostgreSQL\18\bin\psql" -h localhost -p 5433 -U postgres -c "SELECT 1"

# If connection works, rebuild the project to use correct port
```

### Issue: Server fails to start - "Database connection error: password authentication failed"
**Cause:** PostgreSQL password mismatch  
**Solution:**
```powershell
# Default PostgreSQL password is "postgres"
# If different, verify with:
& "C:\Program Files\PostgreSQL\18\bin\psql" -h localhost -p 5433 -U postgres

# If this fails with password error, reset PostgreSQL password or check server logs
```

### Issue: Server fails to start - "dragon_shop database doesn't exist"
**Cause:** Database not created  
**Solution:**
```powershell
# Re-run database setup
$port = 5433
& "C:\Program Files\PostgreSQL\18\bin\psql" -h localhost -p $port -U postgres -c "CREATE DATABASE dragon_shop;"
& "C:\Program Files\PostgreSQL\18\bin\psql" -h localhost -p $port -U postgres -d dragon_shop -f "sql\migrations\001_create_tables.sql"
```

### Issue: Build fails or stale artifacts
**Cause:** Old build files interfering  
**Solution:**
```powershell
# Clean rebuild
cd C:\Users\YourUsername\Desktop\C-Project\dragon-shopping-cart
rmdir /s /q build
mkdir build
cd build

# Reconfigure CMake
cmake .. -DCMAKE_TOOLCHAIN_FILE="C:\path\to\vcpkg\scripts\buildsystems\vcpkg.cmake" -G "Visual Studio 17 2022" -A x64

# Rebuild
cmake --build . --config Release
```

### Issue: API request fails - "Connection refused" on port 8081
**Cause:** Server not running  
**Solution:**
```powershell
# Check if server is running
Get-NetTCPConnection -LocalPort 8081 -ErrorAction SilentlyContinue

# If nothing shows, start the server:
cd dragon-shopping-cart\build\Release
.\dragon_shopping_cart.exe

# Test from another PowerShell window
Invoke-WebRequest -Uri "http://127.0.0.1:8081/cart" -Method GET
```

### Issue: API request returns "Invalid JSON"
**Cause:** Malformed JSON in request  
**Solution:**
```powershell
# Ensure proper JSON formatting
$body = @{
    id = 4
    name = "Monitor"
    price = 299.99
    quantity = 1
} | ConvertTo-Json

# Verify header is set
Invoke-WebRequest -Uri "http://127.0.0.1:8081/cart/add" `
  -Method POST `
  -Headers @{"Content-Type"="application/json"} `
  -Body $body
```

### Still stuck?
1. Check that all prerequisites are installed (see Installation & Setup)
2. Verify PostgreSQL is running and accessible
3. Verify vcpkg path is correct
4. Try a clean rebuild (see "Build fails or stale artifacts" solution above)
5. Check server logs for detailed error messages

## 📁 Project Structure

```
dragon-shopping-cart/
├── src/
│   ├── main.cpp                 # Application entry point
│   ├── app.cpp/app.h            # Main application class and routing
│   ├── dragon/
│   │   └── dragon.h             # Custom HTTP framework (WinSock2-based)
│   ├── db/
│   │   ├── db.cpp/db.h          # PostgreSQL wrapper using libpqxx
│   ├── controllers/
│   │   ├── cart_controller.cpp  # CRUD operations for cart
│   │   └── cart_controller.h
│   ├── models/
│   │   ├── item.cpp/item.h      # Item model
│   │   └── cart.cpp/cart.h      # Cart model
│   ├── views/
│   │   ├── cart_view.cpp        # View rendering
│   │   └── cart_view.h
│   └── utils/
│       ├── json_utils.cpp       # JSON utilities
│       └── json_utils.h
├── sql/
│   └── migrations/
│       └── 001_create_tables.sql # Database schema
├── build/                       # Build artifacts (generated)
├── CMakeLists.txt              # CMake configuration
├── .env                        # Environment configuration (local, git-ignored)
├── .env.example                # Environment configuration template
└── README.md                   # This file
```

## 🔌 Technology Stack

- **Language**: C++17
- **HTTP Server**: Custom Dragon Framework (WinSock2-based)
- **Database**: PostgreSQL 18
- **Database Client**: libpqxx (C++ interface for libpq)
- **JSON Library**: nlohmann/json (v3.11.2)
- **Build System**: CMake 3.10+
- **Package Manager**: vcpkg
- **IDE/Compiler**: Visual Studio Build Tools 2019+

## 📝 License

This project is licensed under the MIT License.

## 💡 Notes

- The application listens on `http://127.0.0.1:8081` by default
- PostgreSQL connection string can be overridden via `DATABASE_URL` environment variable
- Default connection: `postgresql://postgres:postgres@127.0.0.1:5433/dragon_shop`
- If database is unavailable, the application will use in-memory storage as fallback
- Thread safety: Single database connection is shared across request threads (not protected by mutex in current version)

---

## ✅ Checklist for First-Time Setup

Use this checklist to ensure nothing is missed:

- [ ] **Prerequisites**
  - [ ] CMake 3.10+ installed and in PATH (`cmake --version`)
  - [ ] Visual Studio Build Tools 2022 with C++ workload installed
  - [ ] PostgreSQL 18 installed and running
  - [ ] vcpkg downloaded and bootstrapped
  
- [ ] **Configuration**
  - [ ] Found vcpkg path (e.g., C:\vcpkg)
  - [ ] Found PostgreSQL port (5432 or 5433)
  - [ ] Tested PostgreSQL connection

- [ ] **Database Setup**
  - [ ] PostgreSQL service is running
  - [ ] Database `dragon_shop` created
  - [ ] Schema loaded from `sql/migrations/001_create_tables.sql`
  - [ ] Verified 3 sample items exist

- [ ] **Build**
  - [ ] Build directory cleaned (or first-time)
  - [ ] CMake configured with correct vcpkg path
  - [ ] Build completed successfully
  - [ ] Executable exists at `build\Release\dragon_shopping_cart.exe`

- [ ] **Run**
  - [ ] Server started successfully
  - [ ] Server shows "Connected to PostgreSQL" message
  - [ ] Server listening on port 8081
  - [ ] API test returns cart items

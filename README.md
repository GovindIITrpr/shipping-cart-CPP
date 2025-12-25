# Dragon Shopping Cart

A RESTful shopping cart API built in C++ with PostgreSQL database persistence. Features a custom HTTP server framework (Dragon) and implements the MVC architectural pattern.

## 📋 Table of Contents

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

### Step 1: Clone/Open the Project
```bash
cd C:\Users\YourUsername\Desktop\C-Project\dragon-shopping-cart
```

### Step 2: Configure Environment Variables
Copy the example environment file and update with your PostgreSQL settings:
```bash
# Copy .env.example to .env
copy .env.example .env

# Edit .env with your PostgreSQL credentials and settings
# Key variables:
# - DB_HOST: PostgreSQL server address (default: 127.0.0.1)
# - DB_PORT: PostgreSQL port (default: 5433, check your installation)
# - DB_USER: PostgreSQL username (default: postgres)
# - DB_PASSWORD: PostgreSQL password
# - DB_NAME: Database name (default: dragon_shop)
# - SERVER_PORT: HTTP server port (default: 8081)
```

See [.env.example](.env.example) for all available configuration options.

### Step 3: Configure vcpkg Toolchain
### Step 3: Create Build Directory
```bash
mkdir build
cd build
```

### Step 4: Run CMake Configuration
```bash
# Option A: Using vcpkg integration (recommended)
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\path\to\vcpkg\scripts\buildsystems\vcpkg.cmake

# Option B: Standard CMake (if vcpkg is already integrated)
cmake ..
```

Replace `C:\path\to\vcpkg` with your actual vcpkg installation path.

### For Windows (Visual Studio):
```bash
# From the build directory
cmake --build . --config Release

# Or use Visual Studio IDE
start dragon_shopping_cart.sln
```

### Verify Build Success
Look for: `dragon_shopping_cart.exe` in `build\Release\`

## 🗄️ Database Setup

### Step 1: Verify PostgreSQL is Running
```bash
# Check if PostgreSQL service is running
Get-Service -Name postgresql-x64-18

# If not running, start it
Start-Service -Name postgresql-x64-18
```

### Step 2: Create the Database
```bash
# Find your PostgreSQL port (usually 5432 or 5433)
# Open PowerShell and run:
& "C:\Program Files\PostgreSQL\18\bin\psql" -h localhost -p 5433 -U postgres -c "CREATE DATABASE dragon_shop;"
```

### Step 3: Load the Schema
```bash
# Load migrations and sample data
& "C:\Program Files\PostgreSQL\18\bin\psql" -h localhost -p 5433 -U postgres -d dragon_shop -f "C:\path\to\project\sql\migrations\001_create_tables.sql"
```

**Expected Output:**
```
DROP TABLE
CREATE TABLE
CREATE INDEX
INSERT 0 3
```

### Step 4: Verify Database Setup
```bash
& "C:\Program Files\PostgreSQL\18\bin\psql" -h localhost -p 5433 -U postgres -d dragon_shop -c "SELECT * FROM cart_items;"
```

You should see 3 sample items (Laptop, Mouse, Keyboard).

## ▶️ Running the Server

### Step 1: Navigate to Build Directory
```bash
cd dragon-shopping-cart\build
```

### Step 2: Start the Server
```bash
# Run the executable
.\Release\dragon_shopping_cart.exe
```

### Expected Output
```
Dragon framework initialized
Connected to PostgreSQL database successfully
Database initialized successfully
Server created on port 8081
Server started on port 8081
Press Ctrl+C to stop the server...
Listening on http://127.0.0.1:8081
```

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

### Issue: "Connection refused" on port 8081
**Solution:**
- Ensure the server is running
- Check that no other application is using port 8081
- Verify firewall settings allow connections to localhost

### Issue: "Database connection error: password authentication failed"
**Solution:**
1. Verify PostgreSQL is running: `Get-Service postgresql-x64-18`
2. Check the correct port (5432 or 5433):
   ```bash
   & "C:\Program Files\PostgreSQL\18\bin\psql" -h localhost -U postgres -l
   ```
3. Update the port in `src/app.cpp` line 10 if needed
4. Rebuild: `cmake --build . --config Release`

### Issue: "dragon_shop database doesn't exist"
**Solution:**
```bash
# Create the database
& "C:\Program Files\PostgreSQL\18\bin\psql" -h localhost -p 5433 -U postgres -c "CREATE DATABASE dragon_shop;"

# Load schema
& "C:\Program Files\PostgreSQL\18\bin\psql" -h localhost -p 5433 -U postgres -d dragon_shop -f "sql/migrations/001_create_tables.sql"
```

### Issue: CMake configure fails with "libpqxx not found"
**Solution:**
1. Ensure vcpkg is properly integrated
2. Install libpqxx via vcpkg:
   ```bash
   .\vcpkg install libpqxx:x64-windows
   ```
3. Re-run CMake with toolchain flag:
   ```bash
   cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\path\to\vcpkg\scripts\buildsystems\vcpkg.cmake
   ```

### Issue: "Invalid JSON: parse error"
**Solution:**
- Ensure request `Content-Type: application/json` header is set
- Validate JSON syntax (no trailing commas, proper quotes)
- Use single quotes in PowerShell: `@{...} | ConvertTo-Json`

### Issue: Build fails with C++ compilation errors
**Solution:**
1. Ensure Visual Studio Build Tools 2019+ is installed with C++ workload
2. Clean build directory:
   ```bash
   rmdir /s /q build
   mkdir build && cd build
   cmake .. -DCMAKE_TOOLCHAIN_FILE=C:\path\to\vcpkg\scripts\buildsystems\vcpkg.cmake
   cmake --build . --config Release
   ```

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

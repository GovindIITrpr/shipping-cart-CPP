# Dragon Shopping Cart - Project Guide for Interviews

A comprehensive guide to understand the project architecture, code structure, and workflow.

---

## 📚 Table of Contents

1. [Project Overview](#project-overview)
2. [Architecture Pattern (MVC)](#architecture-pattern-mvc)
3. [Tech Stack](#tech-stack)
4. [Project Structure](#project-structure)
5. [Component Deep Dive](#component-deep-dive)
6. [Complete Workflow](#complete-workflow)
7. [Key Concepts](#key-concepts)
8. [Interview Talking Points](#interview-talking-points)

---

## 1. Project Overview

### What Does This Project Do?

This is a **RESTful Shopping Cart API** - essentially a backend service that manages shopping cart operations over HTTP.

**Real-world analogy:**
- Like Amazon's cart feature, but just the backend API
- Users/clients send HTTP requests (GET, POST, DELETE, PUT)
- Server processes requests and returns JSON responses
- Data is stored in PostgreSQL database

### Core Capabilities

```
User Makes HTTP Request (from browser/app)
          ↓
    Server Receives It
          ↓
    Controller Processes It
          ↓
    Retrieves/Updates Data (Database or In-Memory)
          ↓
    Sends Back JSON Response
```

---

## 2. Architecture Pattern (MVC)

The project uses **MVC Architecture** - a common pattern to organize code.

### What is MVC?

```
MVC = Model-View-Controller
├── Model    = Data & Business Logic (Cart, Item classes)
├── View     = Presentation (JSON responses in our case)
└── Controller = Intermediary (CartController - connects Model and View)
```

### Why Use MVC?

✅ **Separation of Concerns** - Each part has a single responsibility  
✅ **Easy to Test** - Can test each component independently  
✅ **Scalable** - Easy to add features without breaking existing code  
✅ **Professional Standard** - Used in most industry projects  

### How It Works in Our Project

```
HTTP Request
    ↓
Router (Dragon Framework)  ← "Which handler should handle this?"
    ↓
Controller (CartController) ← "What business logic applies?"
    ↓
Model (Cart, Item) ← "Update the data"
    ↓
Database (PostgreSQL) ← "Persist to storage"
    ↓
View (JSON Response) ← "Format response"
    ↓
Send Back to Client
```

---

## 3. Tech Stack

| Component | Technology | Purpose |
|-----------|-----------|---------|
| **Language** | C++17 | Core language |
| **HTTP Server** | Custom "Dragon" Framework | Handles HTTP requests |
| **Database** | PostgreSQL 18 | Persistent data storage |
| **DB Access** | libpqxx | C++ library to query PostgreSQL |
| **JSON** | nlohmann/json | Parse/create JSON |
| **Socket API** | WinSock2 (Windows) | Low-level network communication |

### Why These Choices?

- **C++**: Fast, efficient, suitable for backend servers
- **PostgreSQL**: Reliable, ACID-compliant, industry standard
- **Custom Dragon Framework**: Learning project - understand HTTP from basics
- **JSON**: Standard format for REST APIs

---

## 4. Project Structure

```
dragon-shopping-cart/
│
├── src/
│   ├── main.cpp              ← Entry point, starts app
│   ├── app.cpp/app.h         ← Main application class, sets up routes
│   │
│   ├── dragon/
│   │   └── dragon.h          ← Custom HTTP framework (handles HTTP)
│   │
│   ├── controllers/
│   │   └── cart_controller.* ← Business logic for cart operations
│   │
│   ├── models/
│   │   ├── cart.*            ← Cart model (manages items)
│   │   └── item.*            ← Item model (individual product)
│   │
│   ├── db/
│   │   ├── db_pool.h/cpp     ← Connection pooling (manages DB connections)
│   │   └── db.h/cpp          ← Database query handler
│   │
│   ├── views/
│   │   └── cart_view.*       ← View formatting (JSON responses)
│   │
│   └── utils/
│       └── json_utils.*      ← Helper functions for JSON
│
├── sql/
│   └── migrations/
│       └── 001_create_tables.sql ← Database schema
│
└── build/                    ← Compiled executable goes here
```

---

## 5. Component Deep Dive

### A. Dragon Framework (Custom HTTP Server)

**What it does:** Listens for HTTP requests and routes them to appropriate handlers

**Key parts:**

```cpp
// 1. Request - What client sends
class Request {
    std::string method;    // GET, POST, DELETE, PUT
    std::string path;      // /cart, /cart/add
    std::string body;      // JSON data
    std::map<...> headers; // Content-Type, etc.
}

// 2. Response - What server sends back
class Response {
    int statusCode;        // 200, 404, 500, etc.
    std::string body;      // JSON response
    std::map<...> headers; // Response headers
}

// 3. Router - Maps routes to handlers
class Router {
    void get(path, handler);     // Handles GET requests
    void post(path, handler);    // Handles POST requests
    void put(path, handler);     // Handles PUT requests
    void delete_(path, handler); // Handles DELETE requests
}

// 4. Server - Listens on port 8081
class Server {
    void start();  // Start listening for requests
    void stop();   // Stop listening
}
```

**Example:** When browser sends `GET /cart`, Router finds the handler and executes it.

### B. Models (Business Logic)

#### Item Model
```cpp
class Item {
    int id;           // Unique identifier
    string name;      // "Laptop", "Mouse"
    float price;      // Unit price
    int quantity;     // How many of this item
    
    // Methods to access/modify data
    getId(), getName(), getPrice(), getQuantity()
    setQuantity(), updatePrice()
}
```

#### Cart Model
```cpp
class Cart {
    vector<Item> items;  // List of items in cart
    
    // Methods
    addItem(item)      // Add item to cart
    removeItem(id)     // Remove item from cart
    getItems()         // Get all items
    getTotalPrice()    // Calculate total
}
```

**Real analogy:** Models are like "data containers" with business logic.

### C. Controller (Intermediary)

```cpp
class CartController {
    Cart& cart;                    // Reference to cart data
    shared_ptr<DBPool> db;         // Database connection
    
    // Business logic methods
    addItem(item)      // ← Validate, then add to cart AND database
    removeItem(id)     // ← Remove from cart AND database
    viewCart()         // ← Return all items
    updateItem(item)   // ← Update existing item
}
```

**What it does:**
1. Receives request from HTTP handler
2. Validates input data
3. Updates Model (Cart) in memory
4. Saves to Database
5. Returns response

**Example flow for adding item:**
```
POST /cart/add with JSON {id: 4, name: "Monitor", price: 299.99, quantity: 1}
    ↓
App routes to controller.addItem()
    ↓
Controller validates JSON
    ↓
Controller creates Item object
    ↓
Controller calls cart.addItem()          ← Updates in-memory cart
    ↓
Controller saves to database via db pool ← Persistent storage
    ↓
Returns JSON response {status: "ok"}
```

### D. Database Layer

#### Connection Pool (DBPool)
**Why needed?** 
- Creating new DB connection for every request is slow
- Solution: Keep 5 pre-opened connections ready
- When request needs DB, borrow a connection
- When done, return it to pool

```cpp
class DBPool {
    queue<Connection> availableConnections;  // Ready connections
    
    PooledConnection acquire() {
        // Get a connection from pool
        // If empty, wait for one to be returned
        return connection;
    }
    
    void release(connection) {
        // Return connection to pool when done
    }
}
```

#### Database Queries
```cpp
class DB {
    // Execute SQL queries using libpqxx
    
    // Example: Add item to database
    INSERT INTO cart_items (id, name, price, quantity) 
    VALUES (4, 'Monitor', 299.99, 1)
    
    // Example: Get all items
    SELECT id, name, price, quantity FROM cart_items
}
```

**Real analogy:** Like a library - instead of creating new checkout desk for each person, you have 5 desks ready to serve.

### E. Views (Response Formatting)

Views format the data as JSON for HTTP responses.

```cpp
// In App::setupRoutes(), GET /cart handler:
vector<Item> items = cartController->viewCart();

// Convert to JSON
json j;
j["items"] = json::array();
for (const auto& item : items) {
    j["items"].push_back({
        {"id", item.getId()},
        {"name", item.getName()},
        {"price", item.getPrice()},
        {"quantity", item.getQuantity()}
    });
}

// Send as response
res.send(j.dump());
```

**Output (JSON):**
```json
{
  "items": [
    {"id": 1, "name": "Laptop", "price": 999.99, "quantity": 1},
    {"id": 2, "name": "Mouse", "price": 29.99, "quantity": 2}
  ]
}
```

---

## 6. Complete Workflow

### Scenario: Adding an Item to Cart

```
STEP 1: Browser/App sends HTTP request
POST http://127.0.0.1:8081/cart/add
Body: {"id": 4, "name": "Monitor", "price": 299.99, "quantity": 1}

STEP 2: Dragon Framework receives request
├─ Parses HTTP headers
├─ Extracts method (POST)
├─ Extracts path (/cart/add)
├─ Extracts body (JSON)

STEP 3: Router matches route
Routes.find("POST /cart/add")
└─ Finds handler in app.cpp setupRoutes()

STEP 4: Handler executes
[this](dragon::http::Request &req, dragon::http::Response &res) {
    // Parse JSON from request body
    json j = json::parse(req.body);
    
    // Create Item object
    Item item{
        j.at("id").get<int>(),
        j.at("name").get<string>(),
        j.at("price").get<float>(),
        j.at("quantity").get<int>()
    };
    
    // Call controller
    bool added = cartController->addItem(item);
    
    // Send response
    if (added) {
        res.statusCode = 201;
        res.send(json{{"status","ok"},{"item_id", 4}}.dump());
    }
}

STEP 5: Controller processes
CartController::addItem(const Item& item) {
    // 1. Add to in-memory cart
    cart.addItem(item);
    
    // 2. Save to database
    if (db) {  // If database available
        db->executeQuery(
            "INSERT INTO cart_items (id, name, price, quantity) 
             VALUES (4, 'Monitor', 299.99, 1)"
        );
    }
    return true;
}

STEP 6: Model updates
Cart::addItem(const Item& item) {
    items.push_back(item);  // Add to vector
}

STEP 7: Database persists
PostgreSQL receives INSERT query
└─ Saves data to disk persistently

STEP 8: Response sent back
HTTP/1.1 201 Created
Content-Type: application/json
{"status":"ok","item_id":4}

STEP 9: Browser/App receives response
Display success message to user
```

### Scenario: Retrieving All Items

```
STEP 1: Browser sends GET request
GET http://127.0.0.1:8081/cart

STEP 2: Router matches GET /cart handler

STEP 3: Handler executes
auto items = cartController->viewCart();

STEP 4: Controller retrieves data
CartController::viewCart() {
    if (db) {
        // Get from database
        results = db->executeQuery("SELECT * FROM cart_items");
    } else {
        // Get from in-memory cart
        results = cart.getItems();
    }
    return results;
}

STEP 5: Format as JSON response
{
    "items": [
        {"id": 1, "name": "Laptop", "price": 999.99, "quantity": 1},
        {"id": 2, "name": "Mouse", "price": 29.99, "quantity": 2},
        {"id": 4, "name": "Monitor", "price": 299.99, "quantity": 1}
    ]
}

STEP 6: Send response (200 OK)
```

### Error Handling Flow

```
STEP 1: Client sends invalid JSON
POST /cart/add
Body: {"id": "invalid", "name": "Monitor"}  ← Invalid type

STEP 2: JSON parsing fails
try {
    json j = json::parse(req.body);
    int id = j.at("id").get<int>();  ← Throws exception!
} catch (const json::exception& e) {
    res.statusCode = 400;
    res.send("Invalid JSON: " + e.message);
}

STEP 3: Send error response
HTTP/1.1 400 Bad Request
{"error": "Invalid JSON: ..."}
```

---

## 7. Key Concepts

### A. HTTP Status Codes (You should know these!)

| Code | Meaning | Example |
|------|---------|---------|
| **200** | OK | Successfully retrieved items |
| **201** | Created | Item successfully added |
| **400** | Bad Request | Invalid JSON sent |
| **404** | Not Found | Route doesn't exist |
| **500** | Server Error | Database connection failed |

### B. REST Principles

REST = REpresentational State Transfer

Our API follows REST:
- **GET /cart** → Retrieve (Read)
- **POST /cart/add** → Create (Write)
- **PUT /cart/update/:id** → Update (Modify)
- **DELETE /cart/remove/:id** → Delete (Remove)

### C. Smart Design Choices

#### 1. **Graceful Degradation**
```cpp
// If database fails, still work in-memory
try {
    db = make_shared<DBPool>(connStr, 5);
} catch (const exception& e) {
    cerr << "Database failed, using in-memory storage" << endl;
    // Still works! Just can't persist data
}
```

#### 2. **Connection Pooling**
Instead of: Creating new connection for each request (SLOW)
✅ Use: Keep 5 connections ready in a pool (FAST)

#### 3. **Thread-Safe**
```cpp
std::mutex lock;  // Ensures only one thread accesses at a time
```

#### 4. **RAII Pattern** (Resource Acquisition Is Initialization)
```cpp
// Constructor acquires resource
PooledConnection(DBPool* pool, connection) { }

// Destructor automatically releases resource
~PooledConnection() { pool_->release(conn_); }

// No chance of forgetting to close connection!
```

---

## 8. Interview Talking Points

### How to Explain This Project

#### Start with High Level:
*"This is a RESTful Shopping Cart API built in C++ using the MVC architectural pattern. It allows users to manage shopping cart items through HTTP endpoints, with data persisted in PostgreSQL."*

#### Then Go Deeper:

**Q: What's the architecture?**
> "We use MVC - Models handle data (Cart, Item), Controllers handle business logic, and Views format responses as JSON. The Dragon framework is our custom HTTP server that routes requests to appropriate handlers."

**Q: How does adding an item work?**
> "When a POST request comes to /cart/add with JSON, the Dragon router finds the handler. The handler parses JSON, creates an Item object, passes it to CartController which updates the in-memory Cart and saves to PostgreSQL via the connection pool, then returns a success response."

**Q: What's the database layer?**
> "We have a DBPool that maintains 5 PostgreSQL connections. Instead of creating a new connection for each request (slow), we reuse from the pool. It's thread-safe using mutexes. If database fails, the app gracefully falls back to in-memory storage."

**Q: Why use custom Dragon framework?**
> "It's a learning project to understand HTTP from first principles - how requests are parsed, how responses are formatted, how routing works. In production, we'd use Drogon or similar."

**Q: What design patterns are used?**
> - **MVC**: Separation of concerns
> - **Object Pool**: Connection pooling for performance
> - **RAII**: Automatic resource cleanup
> - **Factory/Builder**: Creating connections
> - **Singleton**: Single app instance

**Q: How do you handle errors?**
> "We use try-catch blocks. If JSON parsing fails, we return 400. If database fails, we log error and return 500. We also validate all inputs before processing."

**Q: What are the advantages of this architecture?**
> - **Scalable**: Easy to add new endpoints
> - **Maintainable**: Clear separation of concerns
> - **Testable**: Can test each component independently
> - **Flexible**: Can swap database or HTTP framework
> - **Reliable**: Connection pooling and error handling

---

## 9. Code Examples to Remember

### Simple GET Request Handler
```cpp
router.get("/cart", [this](dragon::http::Request &req, dragon::http::Response &res) {
    auto items = cartController->viewCart();
    
    // Convert to JSON
    json response;
    response["items"] = json::array();
    for (const auto& item : items) {
        response["items"].push_back({
            {"id", item.getId()},
            {"name", item.getName()},
            {"price", item.getPrice()},
            {"quantity", item.getQuantity()}
        });
    }
    
    res.statusCode = 200;
    res.headers["Content-Type"] = "application/json";
    res.send(response.dump());
});
```

### Controller Business Logic
```cpp
bool CartController::addItem(const Item& item) {
    // 1. Add to memory
    cart.addItem(item);
    
    // 2. Persist to database
    if (db_) {
        try {
            auto conn = db_->acquire();  // Get connection from pool
            // Execute INSERT query
            pqxx::work txn(*conn);
            txn.exec_params(
                "INSERT INTO cart_items (id, name, price, quantity) VALUES ($1, $2, $3, $4)",
                item.getId(), item.getName(), item.getPrice(), item.getQuantity()
            );
            txn.commit();
        } catch (const std::exception& e) {
            // Database error, but in-memory still works
            std::cerr << "Database error: " << e.what() << std::endl;
            return true;  // Success in memory
        }
    }
    return true;
}
```

### Connection Pool Pattern
```cpp
class DBPool {
private:
    std::queue<std::unique_ptr<pqxx::connection>> availableConnections;
    std::mutex poolMutex;
    std::condition_variable cv;
    
public:
    PooledConnection acquire(int timeout_ms = 5000) {
        std::unique_lock<std::mutex> lock(poolMutex);
        
        // Wait if pool is empty
        while (availableConnections.empty()) {
            cv.wait_for(lock, std::chrono::milliseconds(timeout_ms));
        }
        
        // Get a connection
        auto conn = std::move(availableConnections.front());
        availableConnections.pop();
        
        return PooledConnection(this, std::move(conn));
    }
    
    void release(std::unique_ptr<pqxx::connection> conn) {
        std::lock_guard<std::mutex> lock(poolMutex);
        availableConnections.push(std::move(conn));
        cv.notify_one();  // Notify waiting threads
    }
};
```

---

## 10. Summary Table

| Aspect | Implementation |
|--------|-----------------|
| **Architecture** | MVC |
| **HTTP Framework** | Custom Dragon (HTTP 1.1) |
| **Database** | PostgreSQL with libpqxx |
| **Port** | 8081 |
| **Data Format** | JSON |
| **Thread Safety** | Mutexes in connection pool |
| **Error Handling** | Try-catch with graceful fallback |
| **Design Patterns** | RAII, Object Pool, Factory |

---

## Final Tips for Interview

1. **Understand the flow**: Know how a request travels from client → server → database → response
2. **Know your MVC**: Be able to explain what Model, View, Controller do in THIS project
3. **Explain trade-offs**: Why custom framework vs existing framework? Why PostgreSQL?
4. **Mention scalability**: Connection pooling, thread-safety, graceful degradation
5. **Have examples ready**: Be ready to walk through one API call from start to finish
6. **Know the technologies**: What's libpqxx? What's WinSock2? Why nlohmann/json?

---

Good luck with your interview! 🚀

# Concurrency and Transaction Handling Guide

## Issues Identified

### 1. **Thread Safety Issue** ⚠️
- **Problem**: Single `pqxx::connection` shared across multiple request threads without synchronization
- **Risk**: Race conditions, corrupted queries, connection state issues
- **Impact**: Data corruption or crashes under high load

### 2. **Transaction Management** ⚠️
- **Current**: Manual transaction handling in DB wrapper
- **Risk**: Incomplete transactions, deadlocks, dirty reads
- **Need**: Proper isolation levels, retry logic, automatic rollback

### 3. **Connection Pool** ⚠️
- **Problem**: Single connection limits throughput and scalability
- **Risk**: Request queuing, timeouts, connection exhaustion
- **Solution**: Connection pool with configurable size

---

## Solutions Implemented

### Solution 1: Thread-Safe DB Wrapper with Mutex (Simple)

**File**: `src/db/db.h`

```cpp
#include <mutex>
#include <queue>

class DB
{
public:
    DB(const std::string &connStr);
    ~DB();

    pqxx::result exec(const std::string &sql);
    void exec_no_result(const std::string &sql);
    bool is_connected() const;

private:
    std::unique_ptr<pqxx::connection> conn_;
    mutable std::mutex conn_mutex_;  // Protects connection access
};
```

**Implementation**: Lock mutex before each database operation.

**Pros**: Simple, minimal overhead for single connection
**Cons**: Serializes all requests (bottleneck under load)

---

### Solution 2: Connection Pool (Recommended) 🌟

**File**: `src/db/db_pool.h`

Maintains multiple connections (configurable pool size), distributes load across requests.

**Features**:
- ✅ Thread-safe queue of connections
- ✅ Automatic connection recycling
- ✅ Configurable pool size (default: 5)
- ✅ Timeout on exhausted pool
- ✅ Health checks on connection acquire

**Pros**: High throughput, scalable, fault-tolerant
**Cons**: Slightly higher memory overhead

---

## Transaction Patterns

### Pattern 1: Automatic Rollback on Exception
```cpp
try {
    pqxx::work txn(*conn);
    txn.exec(sql);
    txn.commit();  // Commits only if no exception
} catch (const std::exception &e) {
    // Transaction automatically rolls back here
    throw;
}
```

### Pattern 2: Retry Logic with Exponential Backoff
```cpp
int max_retries = 3;
for (int attempt = 1; attempt <= max_retries; ++attempt) {
    try {
        pqxx::work txn(*conn);
        // ... perform transaction
        txn.commit();
        return;
    } catch (const pqxx::integrity_constraint_violation &e) {
        if (attempt == max_retries) throw;
        std::this_thread::sleep_for(
            std::chrono::milliseconds(100 * pow(2, attempt - 1))
        );
    }
}
```

### Pattern 3: Isolation Levels
```cpp
pqxx::work txn(*conn_);
txn.set_variable("transaction_isolation", "'SERIALIZABLE'");
// Your queries here
txn.commit();

// Isolation Levels:
// READ UNCOMMITTED  - Fastest, dirty reads possible
// READ COMMITTED    - No dirty reads (default)
// REPEATABLE READ   - Snapshot isolation
// SERIALIZABLE      - Strictest (lowest performance)
```

---

## Deadlock Prevention

### 1. **Lock Ordering**
Always acquire locks in same order:
```cpp
// GOOD: consistent order
lock(user_table);
lock(cart_table);

// BAD: inconsistent order = deadlock risk
Thread1: lock(cart_table); lock(user_table);
Thread2: lock(user_table); lock(cart_table);  // DEADLOCK!
```

### 2. **Connection Timeout**
```cpp
pqxx::work txn(*conn);
txn.set_variable("statement_timeout", "'5000'");  // 5 seconds
txn.exec(sql);
txn.commit();
```

### 3. **Deadlock Detection & Retry**
```cpp
for (int attempt = 1; attempt <= 3; ++attempt) {
    try {
        pqxx::work txn(*conn);
        txn.exec(sql);
        txn.commit();
        return;
    } catch (const pqxx::in_doubt_error &e) {
        // Deadlock detected, retry
        if (attempt == 3) throw;
        std::this_thread::sleep_for(
            std::chrono::milliseconds(10 * attempt)
        );
    }
}
```

---

## Error Handling Best Practices

### 1. **Categorize Exceptions**
```cpp
catch (const pqxx::integrity_constraint_violation &e) {
    // Duplicate key, FK violation, etc.
    res.statusCode = 409;  // Conflict
    res.send(json{{"error", "Item already exists"}}.dump());
}
catch (const pqxx::in_doubt_error &e) {
    // Transaction state unclear (retry)
    res.statusCode = 503;  // Service Unavailable
    res.send(json{{"error", "Database temporarily unavailable"}}.dump());
}
catch (const pqxx::sql_error &e) {
    // SQL syntax or logic error
    res.statusCode = 400;  // Bad Request
    res.send(json{{"error", "Invalid operation"}}.dump());
}
catch (const std::exception &e) {
    // Generic error
    res.statusCode = 500;
    res.send(json{{"error", "Server error"}}.dump());
}
```

### 2. **Logging for Debugging**
```cpp
#include <iostream>
#include <fstream>

class Logger {
public:
    static void log(const std::string &level, const std::string &msg) {
        std::ofstream log_file("app.log", std::ios::app);
        auto now = std::chrono::system_clock::now();
        log_file << "[" << now.time_since_epoch().count() << "] "
                 << level << ": " << msg << std::endl;
    }
};

// Usage
Logger::log("ERROR", "Failed to add item: " + std::string(e.what()));
```

---

## Configuration Best Practices

### 1. **Pool Size**
- **Small API** (< 100 req/sec): 3-5 connections
- **Medium API** (100-1000 req/sec): 5-10 connections
- **Large API** (> 1000 req/sec): 10-20 connections

### 2. **Timeout Values**
```
Connection timeout: 10 seconds (initial connection)
Query timeout: 5 seconds (per query)
Statement timeout: 3 seconds (very aggressive queries)
```

### 3. **Environment Variables**
```bash
DB_POOL_SIZE=5
DB_CONN_TIMEOUT=10
DB_QUERY_TIMEOUT=5
DB_MAX_RETRIES=3
DB_RETRY_BACKOFF_MS=100
```

---

## Testing for Concurrency Issues

### 1. **Load Test with Apache Bench**
```bash
ab -n 1000 -c 50 http://localhost:8081/cart
```
- `-n 1000`: Total requests
- `-c 50`: Concurrent requests

### 2. **Stress Test Script (PowerShell)**
```powershell
$tasks = @()
1..100 | ForEach-Object {
    $tasks += Start-Job -ScriptBlock {
        $body = @{name="Item$_"; price=10.0; quantity=1} | ConvertTo-Json
        Invoke-WebRequest -Uri "http://127.0.0.1:8081/cart/add" `
          -Method POST `
          -Headers @{"Content-Type"="application/json"} `
          -Body $body
    }
}
$tasks | Receive-Job -Wait
```

### 3. **Monitor PostgreSQL Connections**
```sql
SELECT count(*) as connection_count FROM pg_stat_activity;
SELECT * FROM pg_stat_activity WHERE state = 'active';
```

---

## Migration Path

### Phase 1: Add Mutex Lock (Immediate)
- Minimal code change
- Fixes thread safety
- Performance hit acceptable for small load

### Phase 2: Implement Connection Pool (Next)
- ~200 lines of code
- No breaking API changes
- ~2x performance improvement

### Phase 3: Add Retry Logic & Advanced Patterns (Optional)
- Better fault tolerance
- Handles transient failures gracefully

---

## Deployment Checklist

- [ ] Enable PostgreSQL connection logging: `log_connections = on`
- [ ] Set reasonable pool size for expected load
- [ ] Configure timeouts appropriate for query complexity
- [ ] Implement health check endpoint
- [ ] Set up monitoring for DB connections
- [ ] Test under load before production
- [ ] Document pool size and timeout decisions
- [ ] Set up alerts for connection pool exhaustion

---

## Resources

- **libpqxx Documentation**: https://pqxx.org/development/libpqxx/wiki/Tutorial
- **PostgreSQL Transactions**: https://www.postgresql.org/docs/current/tutorial-transactions.html
- **Isolation Levels**: https://www.postgresql.org/docs/current/transaction-iso.html
- **Connection Pooling**: https://wiki.postgresql.org/wiki/Number_Of_Database_Connections

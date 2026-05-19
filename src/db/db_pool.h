#ifndef DB_POOL_H
#define DB_POOL_H

#include <pqxx/pqxx>
#include <string>
#include <queue>
#include <mutex>
#include <memory>
#include <chrono>
#include <condition_variable>
#include <iostream>

/**
 * Thread-safe connection pool for PostgreSQL
 *
 * Features:
 * - Multiple connections to handle concurrent requests
 * - Automatic connection recycling and health checks
 * - Configurable pool size and timeout
 * - RAII pattern for safe resource management
 */

class DBConnection
{
public:
    DBConnection(std::unique_ptr<pqxx::connection> conn) : conn_(std::move(conn)) {}

    pqxx::connection &get() { return *conn_; }
    bool is_valid() const { return conn_ && conn_->is_open(); }

private:
    std::unique_ptr<pqxx::connection> conn_;
};

class DBPool
{
public:
    /**
     * Initialize connection pool
     * @param connStr PostgreSQL connection string
     * @param pool_size Number of connections to maintain (default: 5)
     */
    DBPool(const std::string &connStr, int pool_size = 5);
    ~DBPool();

    /**
     * Acquire a connection from pool (blocks if pool exhausted)
     * @param timeout_ms Max wait time in milliseconds (default: 5000)
     * @return RAII wrapper for connection
     */
    class PooledConnection
    {
    public:
        PooledConnection(DBPool *pool, std::unique_ptr<pqxx::connection> conn)
            : pool_(pool), conn_(std::move(conn)) {}

        ~PooledConnection()
        {
            if (pool_ && conn_)
            {
                pool_->release(std::move(conn_));
            }
        }

        pqxx::connection &operator*() const { return *conn_; }
        pqxx::connection *operator->() const { return conn_.get(); }
        pqxx::connection &get() { return *conn_; }

    private:
        DBPool *pool_;
        std::unique_ptr<pqxx::connection> conn_;

        // Prevent copying
        PooledConnection(const PooledConnection &) = delete;
        PooledConnection &operator=(const PooledConnection &) = delete;
    };

    PooledConnection acquire(int timeout_ms = 5000);

    /**
     * Execute query with automatic connection acquisition and release
     * @param sql SQL query string
     * @return Query result
     */
    pqxx::result exec(const std::string &sql);
    void exec_no_result(const std::string &sql);
    bool is_connected() const { return available() > 0; }

    int available() const { return static_cast<int>(pool_.size()); }
    int total_size() const { return pool_size_; }

private:
    void release(std::unique_ptr<pqxx::connection> conn);
    std::unique_ptr<pqxx::connection> create_connection();

    std::string conn_str_;
    int pool_size_;
    std::queue<std::unique_ptr<pqxx::connection>> pool_;
    mutable std::mutex pool_mutex_;
    std::condition_variable pool_cv_;
};

#endif // DB_POOL_H

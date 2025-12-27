#include "db_pool.h"

DBPool::DBPool(const std::string &connStr, int pool_size)
    : conn_str_(connStr), pool_size_(pool_size)
{
    // Initialize pool with connections
    for (int i = 0; i < pool_size; ++i)
    {
        try
        {
            auto conn = create_connection();
            pool_.push(std::move(conn));
            std::cout << "Pool connection " << (i + 1) << "/" << pool_size << " created" << std::endl;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Failed to create pool connection: " << e.what() << std::endl;
        }
    }

    if (pool_.empty())
    {
        throw std::runtime_error("Failed to create any database connections in pool");
    }
    std::cout << "Database pool initialized with " << pool_.size() << " connections" << std::endl;
}

DBPool::~DBPool()
{
    std::lock_guard<std::mutex> lock(pool_mutex_);
    while (!pool_.empty())
    {
        pool_.pop();
    }
    std::cout << "Database pool closed" << std::endl;
}

std::unique_ptr<pqxx::connection> DBPool::create_connection()
{
    try
    {
        auto conn = std::make_unique<pqxx::connection>(conn_str_);
        if (!conn->is_open())
        {
            throw std::runtime_error("Connection failed to open");
        }
        return conn;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Connection creation error: " << e.what() << std::endl;
        throw;
    }
}

DBPool::PooledConnection DBPool::acquire(int timeout_ms)
{
    std::unique_lock<std::mutex> lock(pool_mutex_);

    // Wait for available connection with timeout
    if (!pool_cv_.wait_for(lock, std::chrono::milliseconds(timeout_ms),
                           [this]()
                           { return !pool_.empty(); }))
    {
        throw std::runtime_error("Connection pool exhausted (timeout)");
    }

    // Get connection from pool
    auto conn = std::move(pool_.front());
    pool_.pop();

    // Check connection health; recreate if needed
    if (!conn || !conn->is_open())
    {
        std::cerr << "Stale connection detected, creating new one" << std::endl;
        try
        {
            conn = create_connection();
        }
        catch (const std::exception &e)
        {
            std::cerr << "Failed to create replacement connection: " << e.what() << std::endl;
            throw;
        }
    }

    return PooledConnection(this, std::move(conn));
}

void DBPool::release(std::unique_ptr<pqxx::connection> conn)
{
    if (!conn)
        return;

    {
        std::lock_guard<std::mutex> lock(pool_mutex_);
        if (conn->is_open())
        {
            pool_.push(std::move(conn));
        }
    }
    pool_cv_.notify_one();
}

pqxx::result DBPool::exec(const std::string &sql)
{
    auto conn = acquire();
    try
    {
        pqxx::work txn(*conn);
        pqxx::result r = txn.exec(sql);
        txn.commit();
        return r;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Query execution error: " << e.what() << std::endl;
        throw;
    }
}

void DBPool::exec_no_result(const std::string &sql)
{
    auto conn = acquire();
    try
    {
        pqxx::work txn(*conn);
        txn.exec(sql);
        txn.commit();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Query execution error: " << e.what() << std::endl;
        throw;
    }
}

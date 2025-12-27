#include "db.h"

DB::DB(const std::string &connStr)
{
    try
    {
        conn_ = std::make_unique<pqxx::connection>(connStr);
        if (!conn_->is_open())
        {
            throw std::runtime_error("Failed to open database connection");
        }
        std::cout << "Connected to PostgreSQL database successfully" << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Database connection error: " << e.what() << std::endl;
        conn_.reset();
    }
}

DB::~DB()
{
    // pqxx::connection closes automatically when destroyed
    conn_.reset();
}

pqxx::result DB::exec(const std::string &sql)
{
    std::lock_guard<std::mutex> lock(conn_mutex_); // Thread-safe lock
    if (!conn_ || !conn_->is_open())
    {
        throw std::runtime_error("Database connection is not open");
    }
    try
    {
        pqxx::work txn(*conn_);
        pqxx::result r = txn.exec(sql);
        txn.commit();
        return r;
    }
    catch (const std::exception &e)
    {
        std::cerr << "SQL execution error: " << e.what() << std::endl;
        throw;
    }
}

void DB::exec_no_result(const std::string &sql)
{
    std::lock_guard<std::mutex> lock(conn_mutex_); // Thread-safe lock
    if (!conn_ || !conn_->is_open())
    {
        throw std::runtime_error("Database connection is not open");
    }
    try
    {
        pqxx::work txn(*conn_);
        txn.exec(sql);
        txn.commit();
    }
    catch (const std::exception &e)
    {
        std::cerr << "SQL execution error: " << e.what() << std::endl;
        throw;
    }
}

bool DB::is_connected() const
{
    return conn_ && conn_->is_open();
}

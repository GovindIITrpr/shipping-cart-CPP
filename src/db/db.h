#ifndef DB_H
#define DB_H

#include <pqxx/pqxx>
#include <string>
#include <vector>
#include <iostream>
#include <mutex>

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
    mutable std::mutex conn_mutex_; // Thread-safe access to connection
};

#endif // DB_H

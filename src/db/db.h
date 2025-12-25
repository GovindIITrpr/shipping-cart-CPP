#ifndef DB_H
#define DB_H

#include <pqxx/pqxx>
#include <string>
#include <vector>
#include <iostream>

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
};

#endif // DB_H

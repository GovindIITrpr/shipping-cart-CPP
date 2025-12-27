#ifndef DB_HANDLER_EXAMPLES_H
#define DB_HANDLER_EXAMPLES_H

#include "db_exceptions.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

/**
 * Example: How to use the new concurrency and exception handling
 * in HTTP request handlers
 */

// Example 1: Using mutex-protected single connection (Simple)
void handle_add_item_simple(const std::string &req_body,
                            std::shared_ptr<DB> db,
                            dragon::http::Response &res)
{
    try
    {
        auto j = json::parse(req_body);
        int id = j.at("id").get<int>();
        std::string name = j.at("name").get<std::string>();
        float price = j.at("price").get<float>();
        int qty = j.at("quantity").get<int>();

        // DB class now has mutex protection
        std::string sql = "INSERT INTO cart_items (item_id, name, price, quantity) VALUES (" +
                          std::to_string(id) + ", '" + name + "', " +
                          std::to_string(price) + ", " + std::to_string(qty) + ")";

        db->exec_no_result(sql); // Thread-safe!

        res.statusCode = 201;
        res.headers["Content-Type"] = "application/json";
        res.send(json{{"status", "ok"}, {"id", id}}.dump());
    }
    catch (const json::exception &e)
    {
        res.statusCode = 400;
        res.send(json{{"error", "Invalid JSON"}}.dump());
    }
    catch (const std::exception &e)
    {
        res.statusCode = 500;
        res.send(json{{"error", "Server error"}}.dump());
    }
}

// Example 2: Using connection pool with retry logic (Advanced)
void handle_add_item_with_pool(const std::string &req_body,
                               std::shared_ptr<DBPool> pool,
                               dragon::http::Response &res)
{
    try
    {
        auto j = json::parse(req_body);
        std::string name = j.at("name").get<std::string>();
        float price = j.at("price").get<float>();
        int qty = j.at("quantity").get<int>();

        // Use retry logic for transient failures
        pqxx::result result = dbexcept::retry_on_transient(
            [&pool, &name, price, qty]() -> pqxx::result
            {
                auto conn = pool->acquire(5000);
                std::string sql = "INSERT INTO cart_items (name, price, quantity) VALUES ('" +
                                  name + "', " + std::to_string(price) + ", " +
                                  std::to_string(qty) + ") RETURNING id";
                pqxx::work txn(*conn);
                pqxx::result r = txn.exec(sql);
                txn.commit();
                return r;
            },
            3,  // max 3 attempts
            100 // 100ms initial backoff
        );

        if (!result.empty())
        {
            int new_id = result[0][0].as<int>();
            res.statusCode = 201;
            res.headers["Content-Type"] = "application/json";
            res.send(json{{"status", "ok"}, {"id", new_id}}.dump());
        }
    }
    catch (const dbexcept::DBException &e)
    {
        res.statusCode = e.http_status;
        res.headers["Content-Type"] = "application/json";
        res.send(json{{"error", e.message}}.dump());
    }
    catch (const std::exception &e)
    {
        auto exc = dbexcept::categorize_exception(e);
        res.statusCode = exc.http_status;
        res.headers["Content-Type"] = "application/json";
        res.send(json{{"error", exc.message}}.dump());
    }
}

// Example 3: Handling constraint violations gracefully
void handle_update_item_with_conflict_handling(
    int item_id,
    const std::string &req_body,
    std::shared_ptr<DB> db,
    dragon::http::Response &res)
{
    try
    {
        auto j = json::parse(req_body);
        std::string name = j.value("name", "");
        float price = j.value("price", 0.0f);
        int qty = j.value("quantity", 0);

        std::string sql = "UPDATE cart_items SET name = '" + name +
                          "', price = " + std::to_string(price) +
                          ", quantity = " + std::to_string(qty) +
                          " WHERE id = " + std::to_string(item_id);

        db->exec_no_result(sql);

        res.statusCode = 200;
        res.headers["Content-Type"] = "application/json";
        res.send(json{{"status", "ok"}, {"message", "Item updated"}}.dump());
    }
    catch (const pqxx::integrity_constraint_violation &e)
    {
        // Handle constraint violation (e.g., duplicate unique column)
        res.statusCode = 409; // Conflict
        res.headers["Content-Type"] = "application/json";
        res.send(json{{"error", "Constraint violation: " + std::string(e.what())}}.dump());
    }
    catch (const pqxx::in_doubt_error &e)
    {
        // Handle deadlock - can retry
        res.statusCode = 503; // Service Unavailable
        res.headers["Content-Type"] = "application/json";
        res.send(json{{"error", "Database deadlock, please retry"}}.dump());
    }
    catch (const std::exception &e)
    {
        res.statusCode = 500;
        res.headers["Content-Type"] = "application/json";
        res.send(json{{"error", "Server error: " + std::string(e.what())}}.dump());
    }
}

#endif // DB_HANDLER_EXAMPLES_H

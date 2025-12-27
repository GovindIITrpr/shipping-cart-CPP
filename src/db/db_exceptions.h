#ifndef DB_EXCEPTIONS_H
#define DB_EXCEPTIONS_H

#include <pqxx/pqxx>
#include <string>
#include <iostream>

/**
 * Database exception categorization and handling utilities
 * Provides structured error handling and retry logic for common DB faults
 */

namespace dbexcept
{

    enum class ErrorType
    {
        CONSTRAINT_VIOLATION,  // Unique/FK constraint violation
        DEADLOCK,              // Transaction deadlock detected
        TIMEOUT,               // Query or connection timeout
        CONNECTION_FAILED,     // Connection pool exhausted or unavailable
        SQL_SYNTAX_ERROR,      // Invalid SQL syntax
        SERIALIZATION_FAILURE, // SERIALIZABLE isolation conflict
        UNKNOWN                // Generic database error
    };

    struct DBException
    {
        ErrorType type;
        std::string message;
        int http_status;

        DBException(ErrorType t, const std::string &msg, int status = 500)
            : type(t), message(msg), http_status(status) {}
    };

    /**
     * Categorize pqxx exception and convert to our exception type
     */
    inline DBException categorize_exception(const std::exception &e)
    {
        const auto &what = std::string(e.what());

        if (dynamic_cast<const pqxx::integrity_constraint_violation *>(&e))
        {
            return DBException(ErrorType::CONSTRAINT_VIOLATION,
                               "Item already exists or constraint violation", 409);
        }
        if (dynamic_cast<const pqxx::in_doubt_error *>(&e) ||
            what.find("deadlock") != std::string::npos)
        {
            return DBException(ErrorType::DEADLOCK,
                               "Database deadlock, please retry", 503);
        }
        if (dynamic_cast<const pqxx::broken_connection *>(&e))
        {
            return DBException(ErrorType::CONNECTION_FAILED,
                               "Database connection lost", 503);
        }
        if (what.find("timeout") != std::string::npos)
        {
            return DBException(ErrorType::TIMEOUT,
                               "Query timeout, please retry", 504);
        }
        if (dynamic_cast<const pqxx::sql_error *>(&e))
        {
            return DBException(ErrorType::SQL_SYNTAX_ERROR,
                               "Invalid database operation", 400);
        }
        if (what.find("serialization") != std::string::npos ||
            what.find("40001") != std::string::npos)
        {
            return DBException(ErrorType::SERIALIZATION_FAILURE,
                               "Serialization conflict, please retry", 503);
        }

        return DBException(ErrorType::UNKNOWN, what, 500);
    }

    /**
     * Retry helper for transient failures
     * Implements exponential backoff
     */
    template <typename Func>
    inline typename std::invoke_result<Func>::type retry_on_transient(
        Func &&func, int max_attempts = 3, int backoff_ms = 100)
    {
        for (int attempt = 1; attempt <= max_attempts; ++attempt)
        {
            try
            {
                return std::invoke(std::forward<Func>(func));
            }
            catch (const std::exception &e)
            {
                auto exc = categorize_exception(e);

                // Retry only for transient failures
                if (exc.type != ErrorType::DEADLOCK &&
                    exc.type != ErrorType::TIMEOUT &&
                    exc.type != ErrorType::SERIALIZATION_FAILURE)
                {
                    throw exc; // Non-transient error, fail immediately
                }

                if (attempt == max_attempts)
                {
                    throw exc; // Last attempt failed
                }

                std::cerr << "Transient error (attempt " << attempt << "/" << max_attempts
                          << "): " << exc.message << ", retrying..." << std::endl;

                // Exponential backoff: 100ms, 200ms, 400ms, ...
                int delay_ms = backoff_ms * (1 << (attempt - 1));
                std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
            }
        }
    }

} // namespace dbexcept

#endif // DB_EXCEPTIONS_H

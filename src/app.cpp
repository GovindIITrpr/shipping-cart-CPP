#include "app.h"
#include "utils/path.h"
#include <cstdlib>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

App::App()
    : cart(std::make_unique<Cart>())
{
    // Get DB connection string from environment or use default
    const char *dbUrl = std::getenv("DATABASE_URL");
    std::string connStr = dbUrl ? dbUrl : "postgresql://postgres:postgres@127.0.0.1:5433/dragon_shop";

    try
    {
        db = std::make_shared<DB>(connStr);
        cartController = std::make_unique<CartController>(*cart, db);
        std::cout << "Database initialized successfully" << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Failed to initialize database: " << e.what() << std::endl;
        std::cerr << "Falling back to in-memory storage" << std::endl;
        cartController = std::make_unique<CartController>(*cart, nullptr);
    }
}

void App::start()
{
    // Initialize the application and set up routing
    setupRoutes();
    // Start the server
    startServer();
}

void App::stop()
{
    if (server)
    {
        server->stop();
    }
}

void App::setupRoutes()
{
    // Example: route for viewing all cart items
    router.get("/cart", [this](dragon::http::Request &req, dragon::http::Response &res)
               {
        auto items = cartController->viewCart();
        res.statusCode = 200;
        res.headers["Content-Type"] = "application/json";
        json j;
        j["items"] = json::array();
        for (const auto &it : items) {
            j["items"].push_back(json{
                {"id", it.getId()},
                {"name", it.getName()},
                {"price", it.getPrice()},
                {"quantity", it.getQuantity()}
            });
        }
        res.send(j.dump()); });

    // Route for adding an item to the cart
    router.post("/cart/add", [this](dragon::http::Request &req, dragon::http::Response &res)
                {
    try {
        auto j = nlohmann::json::parse(req.body);
        Item item{
            j.at("id").get<int>(),
            j.at("name").get<std::string>(),
            j.at("price").get<float>(),
            j.at("quantity").get<int>()
        };
        bool added = cartController->addItem(item);                   // inserts into DB via controller
        if(added){
            res.statusCode = 201;
            res.headers["Content-Type"] = "application/json";
            res.send(nlohmann::json{{"status","ok"},{"item_id", item.getId()}}.dump());
        }else{
            res.statusCode = 500;
            res.send("Failed to add item to cart");
        }
    } catch (const nlohmann::json::exception &e) {
        res.statusCode = 400;
        res.send(std::string("Invalid JSON: ") + e.what());
    } catch (const std::exception &e) {
        res.statusCode = 500;
        res.send(std::string("Server error: ") + e.what());
    } });

    // Route for deleting an item from the cart
    router.delete_("/cart/remove", [this](dragon::http::Request &req, dragon::http::Response &res)
                   {
    try {
        auto j = nlohmann::json::parse(req.body);
        int id = j.value("id", 0);

        bool removed = cartController->removeItem(id);
        res.headers["Content-Type"] = "application/json";
        if (removed) {
            res.statusCode = 200;
            res.send(nlohmann::json{{"status","ok"},{"removed_id", id}}.dump());
        } else {
            res.statusCode = 404;
            res.send(nlohmann::json{{"status","error"},{"message","Item not found"}}.dump());
        }
    } catch (const std::exception &e) {
        res.statusCode = 400;
        res.send(std::string("Invalid request: ") + e.what());
    } });

    // Route for updating an item in the cart
    router.put("/cart/update", [this](dragon::http::Request &req, dragon::http::Response &res)
               {
    try { 

        auto j = nlohmann::json::parse(req.body);

        Item updatedItem{ 
            j.value("id", 0),
            j.value("name", ""),
            j.value("price", 0.0f),
            j.value("quantity", 0)
        };

        bool updated = cartController->updateItem(updatedItem);
        res.headers["Content-Type"] = "application/json";
        if (updated) {
            res.statusCode = 200;
            res.send(nlohmann::json{{"status","ok"},{"message","Item updated"}}.dump());
        } else {
            res.statusCode = 404;
            res.send(nlohmann::json{{"status","error"},{"message","Item not found"}}.dump());
        }
    } catch (const nlohmann::json::exception &e) {
        res.statusCode = 400;
        res.send(std::string("Invalid JSON: ") + e.what());
    } catch (const std::exception &e) {
        res.statusCode = 500;
        res.send(std::string("Server error: ") + e.what());
    } });
}

void App::startServer()
{
    server = std::make_unique<dragon::http::Server>(8081, router);
    server->start();
}
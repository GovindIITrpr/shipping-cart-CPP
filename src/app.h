#ifndef APP_H
#define APP_H

#include <dragon/dragon.h>
#include <memory>
#include "controllers/cart_controller.h"
#include "models/cart.h"
#include "db/db_pool.h"

class App
{
public:
    App();
    void start();
    void stop();

private:
    void setupRoutes();
    void startServer();
    dragon::http::Router router;
    std::unique_ptr<dragon::http::Server> server;
    std::unique_ptr<Cart> cart;
    std::shared_ptr<DBPool> db;
    std::unique_ptr<CartController> cartController;
};

#endif // APP_H
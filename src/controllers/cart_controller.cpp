#include "cart_controller.h"
#include "../models/cart.h"
#include "../models/item.h"
#include <sstream>

CartController::CartController(Cart &cart, std::shared_ptr<DB> db)
    : cart(cart), db_(db) {}

bool CartController::addItem(const Item &item)
{
    if (!db_ || !db_->is_connected())
    {
        std::cout << "DB is not connected" << std::endl;
        return false;
    }

    try
    {
        std::stringstream ss;
        ss << "INSERT INTO cart_items (item_id, name, price, quantity) VALUES ("
           << item.getId() << ", '"
           << item.getName() << "', "
           << item.getPrice() << ", "
           << item.getQuantity() << ")";
        db_->exec_no_result(ss.str());
        cart.addItem(item);
        std::cout << "Item added to DB and cart" << std::endl;
        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error adding item to DB: " << e.what() << std::endl;
        return false; 
    }
}

bool CartController::removeItem(int itemId)
{
    if (!db_ || !db_->is_connected())
    {
        std::cout << "DB is not connected" << std::endl;
        return false;
    }

    try
    {
        std::stringstream ss;
        ss << "DELETE FROM cart_items WHERE item_id = " << itemId;
        db_->exec_no_result(ss.str());
        cart.removeItem(itemId);
        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error removing item from DB: " << e.what() << std::endl;
        return false;
    }
}

std::vector<Item> CartController::viewCart() const
{
    std::vector<Item> items;

    if (!db_ || !db_->is_connected())
    {
        std::cout << "DB is not connected" << std::endl;
        return cart.getItems();
    }

    try
    {
        pqxx::result r = db_->exec("SELECT id, name, price, quantity FROM cart_items ORDER BY id");
        for (auto row : r)
        {
            items.emplace_back(
                row["id"].as<int>(),
                row["name"].as<std::string>(),
                row["price"].as<float>(),
                row["quantity"].as<int>());
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error fetching cart items from DB: " << e.what() << std::endl;
        return cart.getItems();
    }

    return items;
}

bool CartController::updateItem(const Item &item)
{
    if (!db_ || !db_->is_connected())
    {
        std::cout << "DB is not connected" << std::endl;
        return false;
    }

    try
    {
        std::stringstream ss;
        ss << "UPDATE cart_items SET name = '" << item.getName()
           << "', price = " << item.getPrice()
           << ", quantity = " << item.getQuantity()
           << ", item_id = " << item.getId()
           << " WHERE item_id = " << item.getId();
        db_->exec_no_result(ss.str());
        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error updating item in DB: " << e.what() << std::endl;
        return false;
    }
}
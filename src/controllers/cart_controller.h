#ifndef CART_CONTROLLER_H
#define CART_CONTROLLER_H

#include <string>
#include <vector>
#include <memory>
#include "models/cart.h"
#include "models/item.h"
#include "../db/db.h"

class CartController
{
public:
    CartController(Cart &cart, std::shared_ptr<DB> db);

    bool addItem(const Item &item);
    bool removeItem(int itemId);
    std::vector<Item> viewCart() const;
    bool updateItem(const Item &item);

private:
    Cart &cart;
    std::shared_ptr<DB> db_;
};

#endif // CART_CONTROLLER_H
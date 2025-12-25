#ifndef CART_H
#define CART_H

#include <vector>
#include "item.h"

class Cart
{
public:
    Cart();
    void addItem(const Item &item);
    void removeItem(int itemId);
    std::vector<Item> getItems() const;
    float getTotalPrice() const;
    float checkout() const;

private:
    std::vector<Item> items;
};

#endif // CART_H
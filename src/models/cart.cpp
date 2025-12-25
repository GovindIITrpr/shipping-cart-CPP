#include "cart.h"
#include "item.h"
#include <vector>
#include <algorithm>

Cart::Cart() {}

void Cart::addItem(const Item &item)
{
    items.push_back(item);
}

void Cart::removeItem(int itemId)
{
    items.erase(std::remove_if(items.begin(), items.end(),
                               [itemId](const Item &item)
                               { return item.getId() == itemId; }),
                items.end());
}

std::vector<Item> Cart::getItems() const
{
    return items;
}

float Cart::getTotalPrice() const
{
    float total = 0.0f;
    for (const auto &item : items)
    {
        total += item.getTotalPrice();
    }
    return total;
}

float Cart::checkout() const
{
    float total = 0.0f;
    for (const auto &item : items)
    {
        total += item.getTotalPrice();
    }
    return total;
}
#include "item.h"

Item::Item(int id, const std::string &name, float price, int quantity)
    : id(id), name(name), price(price), quantity(quantity) {}

int Item::getId() const
{
    return id;
}

std::string Item::getName() const
{
    return name;
}

float Item::getPrice() const
{
    return price;
}

int Item::getQuantity() const
{
    return quantity;
}

void Item::setQuantity(int quantity)
{
    this->quantity = quantity;
}

void Item::updatePrice(float newPrice)
{
    this->price = newPrice;
}

float Item::getTotalPrice() const
{
    return price * quantity;
}
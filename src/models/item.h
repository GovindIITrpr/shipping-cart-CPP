#ifndef ITEM_H
#define ITEM_H

#include <string>

class Item
{
public:
    Item(int id, const std::string &name, float price, int quantity);

    int getId() const;
    std::string getName() const;
    float getPrice() const;
    int getQuantity() const;
    float getTotalPrice() const;

    void setQuantity(int quantity);
    void updatePrice(float price);

private:
    int id;
    std::string name;
    float price;
    int quantity;
};

#endif // ITEM_H
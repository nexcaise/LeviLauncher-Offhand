#pragma once
#include "world/item/Item.hpp"

class ItemStack {
public:
    WeakPtr<Item> mItem;
    
    Item* getItem() const;
};
#pragma once
#include "world/item/Item.hpp"
#include "util/SharedPtr.hpp"

class ItemStack {
public:
    WeakPtr<Item> mItem;
    
    Item* getItem() const;
};
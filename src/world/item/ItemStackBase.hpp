#pragma once
#include "world/item/Item.hpp"
#include "util/SharedPtr.hpp"

class ItemStackBase {
public:
    WeakPtr<Item> mItem;
    
    Item* getItem() const;
};
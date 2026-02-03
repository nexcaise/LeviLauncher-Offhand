/// @symbols
#pragma once

class ItemStack {
public:
    WeakPtr<Item> mItem;
    
    Item* getItem() const;
};
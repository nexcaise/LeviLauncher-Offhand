#include "world/item/ItemStack.hpp"

Item* ItemStack::getItem() const {
    return mItem.get();
}
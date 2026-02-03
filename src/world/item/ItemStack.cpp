#include "world/item/Item.hpp"

Item* ItemStackBase::getItem() const {
    return mItem.get();
}
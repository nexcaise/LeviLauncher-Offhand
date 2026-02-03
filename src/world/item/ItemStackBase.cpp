#include "world/item/ItemStackBase.hpp"

Item* ItemStackBase::getItem() const {
    return mItem.get();
}
#include "world/item/Item.hpp"

Item& Item::setAllowOffhand(bool allowOffhand) {
    mAllowOffhand = allowOffhand;
    return *this;
}
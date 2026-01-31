#include "Item.hpp"

Item& Item::setAllowOffhand(bool allowOffhand) {
    mAllowOffhand = allowOffhand;
    return *this;
}
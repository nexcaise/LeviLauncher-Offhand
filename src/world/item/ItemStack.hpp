#pragma once
#include "world/item/Item.hpp"
#include "world/item/ItemStackBase.hpp"

class ItemStackNetIdVariant;

class ItemStack : public ItemStackBase {
public:
    ItemStackNetIdVariant* mNetIdVariant;
};
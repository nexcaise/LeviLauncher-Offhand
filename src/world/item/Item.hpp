#pragma once

#include <string>
#include "world/item/ItemStackBase.hpp"

class Actor;
class FacingID;

class Item {
public:
    bool mAllowOffhand;
	Item& setAllowOffhand(bool allowOffhand);
	std::string mNamespace;
	
	virtual bool calculatePlacePos(ItemStackBase& stack, Actor& actor, FacingID& face, BlockPos& pos) const;
};
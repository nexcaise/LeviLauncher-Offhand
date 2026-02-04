#pragma once

#include <string>

class Item {
public:
    bool mAllowOffhand;
	Item& setAllowOffhand(bool allowOffhand);
	std::string mNamespace;
};
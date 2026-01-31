#pragma once
#include <memory>
class ItemRegistry;

class HashedString;

class ItemRegistryRef {
public:
    std::weak_ptr<ItemRegistry> mWeakRegistry;
    std::weak_ptr<ItemRegistry> mItemRegistry;
    
    std::unordered_map<HashedString, WeakPtr<Item>> const& getNameToItemMap() const;

    std::shared_ptr<ItemRegistry> _lockRegistry() const;
};

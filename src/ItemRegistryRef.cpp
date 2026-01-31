#include "ItemRegistry.hpp"
#include "ItemRegistryRef.hpp"

std::unordered_map<StringHash, WeakPtr<Item>> ItemRegistryRef::getNameToItemMap() const
{
return nameToItemMap;
}

std::shared_ptr<ItemRegistry> ItemRegistryRef::_lockRegistry() const
{
    //Assert(!mItemRegistry.expired(), "Trying to lock a expired ItemRegistry!");
    auto lock = mItemRegistry.lock();
    //Assert(lock.operator bool(), "ItemRegistryRef cannot access ItemRegistry - attempt to lock weak_ptr returned null!");
    return lock;
}
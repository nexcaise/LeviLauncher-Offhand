#include "ItemRegistry.hpp"
#include "ItemRegistryRef.hpp"

std::shared_ptr<ItemRegistry> ItemRegistryRef::_lockRegistry() const
{
    //Assert(!mItemRegistry.expired(), "Trying to lock a expired ItemRegistry!");
    auto lock = mItemRegistry.lock();
    //Assert(lock.operator bool(), "ItemRegistryRef cannot access ItemRegistry - attempt to lock weak_ptr returned null!");
    return lock;
}
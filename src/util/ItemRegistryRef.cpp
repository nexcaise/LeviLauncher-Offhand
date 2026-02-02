#include "util/ItemRegistryRef.hpp"

std::shared_ptr<ItemRegistry> ItemRegistryRef::_lockRegistry() const
{
    auto lock = mItemRegistry.lock();
    return lock;
}

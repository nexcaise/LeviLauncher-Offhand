#include "util/ItemRegistryRef.hpp"

std::shared_ptr<ItemRegistry> ItemRegistryRef::_lockRegistry() const
{
    auto lock = mWeakRegistry.lock();
    return lock;
}

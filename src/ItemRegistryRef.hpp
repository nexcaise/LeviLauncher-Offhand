#pragma once
#include <memory>
class ItemRegistry;

class ItemRegistryRef {
public:
    std::shared_ptr<ItemRegistry> _lockRegistry() const;
};

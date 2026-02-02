#pragma once
#include <memory>
#include <unordered_map>
#include "util/ItemRegistry.hpp"

class ItemRegistryRef {
public:
    std::shared_ptr<ItemRegistry> _lockRegistry() const {
        auto wp = reinterpret_cast<const std::weak_ptr<ItemRegistry>*>(
            reinterpret_cast<const uint8_t*>(this) + 0x18
        );
        return wp->lock();
    }
};
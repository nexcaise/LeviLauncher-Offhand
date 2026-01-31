#pragma once
#include <cstddef>

class StringHash {
public:
    int mHash;
    bool operator==(StringHash const& o) const { return mHash == o.mHash; }
};

namespace std {
template<>
struct hash<StringHash> {
    size_t operator()(StringHash const& s) const noexcept {
        return std::hash<int>()(s.mHash);
    }
};
}
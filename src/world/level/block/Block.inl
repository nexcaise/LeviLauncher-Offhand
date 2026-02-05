#pragma once
#include "mc/src/common/world/level/block/Block.hpp"

template <typename T>
inline T Block::getState(const BlockState& blockState) const
{
    auto& states = mLegacyBlock->mStates;
    auto result = states.find(blockState.mID);

    if (blockState.mID >= result->first) {
        int value = (0xFFFF >> (16 - result->second.mNumBits)) & (mData >> (result->second.mEndBit - result->second.mNumBits + 1));
		return static_cast<T>(value);
    }

    auto alteredState = mLegacyBlock->_tryLookupAlteredStateCollection(blockState.mID, mData);

    if (alteredState.has_value()) 
		return static_cast<T>(alteredState.value());

    return static_cast<T>(0);
}

template <typename T>
gsl::strict_not_null<const Block*> Block::setState(const BlockState& stateType, T value) const {
    auto& states = mLegacyBlock->mStates;
    auto foundState = states.find(stateType.mID);

    if (stateType.mID >= foundState->first) {
        if ((int)value < (int64_t)foundState->second.mVariationCount) {
            uint64_t maskedData = mData & (~foundState->second.mMask);
            uint64_t index = maskedData | ((int)value << (foundState->second.mEndBit - foundState->second.mNumBits + 1));

            if (index < mLegacyBlock->mBlockPermutations.size()) {
                return gsl::make_not_null(mLegacyBlock->mBlockPermutations[index].get());
            }
        }
        else {
            Log::Info("more than found variation count");
        }

        AssertFail("Not handling this ong!");
    }

	AssertFail("Unhandled case in Block::setState");
}
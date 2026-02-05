/// @symbols
#pragma once
#include <amethyst/Imports.hpp>
#include <mc/src-deps/core/string/StringHash.hpp>
#include <mc/src/common/world/Direction.hpp>
#include <mc/src/common/nbt/CompoundTagVariant.hpp>
#include <mc/src/common/nbt/CompoundTag.hpp>

class CompoundTag;

class BlockState {
public:
    struct StateListNode {
        BlockState::StateListNode* mNext;
        BlockState::StateListNode* mPrev;
        BlockState* mState;

		/// @address {0x597AB78}
		MC static StateListNode* mHead;
    };

public:
    const uint64_t mID;
    const uint64_t mVariationCount;
    const HashedString mName;
    BlockState::StateListNode mNode;

    /** @vidx {0} */ 
	virtual ~BlockState() {
		unregisterSelf();
	}
    virtual void toNBT(CompoundTag&, int) const = 0;
    virtual bool fromNBT(const CompoundTag&, int&) const = 0;

	BlockState(uint64_t id, uint64_t variationCount, const HashedString& name)
        : mID(id), mVariationCount(variationCount), mName(name), mNode{} {}

private:
	void unregisterSelf() {
        auto* prev = mNode.mPrev;
        auto* next = mNode.mNext;

        if (prev)
            prev->mNext = next;
        else if (BlockState::StateListNode::mHead == &mNode)
            BlockState::StateListNode::mHead = next;

        if (next)
            next->mPrev = prev;

        mNode.mNext = nullptr;
        mNode.mPrev = nullptr;
        mNode.mState = nullptr;
    }
};

struct BlockStateInstance {
    uint32_t mEndBit;
    uint32_t mNumBits;
    uint32_t mVariationCount;
    uint32_t mMask;
    bool mInitialized;
    const BlockState* mState;
};

template<typename T>
class BlockStateVariant : public BlockState {
public:
};

template<typename T>
class BuiltInBlockStateVariant : public BlockStateVariant<T> {
public:
	static_assert(sizeof(T), "Instantiation of BuiltInBlockStateVariant with T not allowed!");
};

template<>
/** @vptr {0x4E62F20} */
class BuiltInBlockStateVariant<Direction::Type> : public BlockStateVariant<Direction::Type> {
public:
	/** @vidx {0} */ MC virtual ~BuiltInBlockStateVariant() override;
	/** @vidx {1} */ MC virtual void toNBT(CompoundTag&, int) const override;
	/** @vidx {2} */ MC virtual bool fromNBT(const CompoundTag&, int&) const override;
};

class BuiltInBlockStates {
public:
	/** @addr {0x59DF450} */
	MC static BuiltInBlockStateVariant<Direction::Type> CardinalDirection;
};

/////////////////////////////////////
// Amethyst Custom BlockStates stuff
/////////////////////////////////////

template<typename T, typename Enable = void>
struct CustomBlockStateSerializer {	
	static void write(CompoundTag& tag, const std::string& key, const T& value) {
		static_assert(sizeof(T) == 0, "No CustomBlockStateSerializer specialization for this type");
	}

	static bool read(const CompoundTag& tag, const std::string& key, T& value) {
		static_assert(sizeof(T) == 0, "No CustomBlockStateSerializer specialization for this type");
		return false;
	}
};

// specifialization for all enums
template<typename T>
struct CustomBlockStateSerializer<T, std::enable_if_t<std::is_enum_v<T>>> {
	using Underlying = std::underlying_type_t<T>;

	static void write(CompoundTag& tag, const std::string& key, const T& value) {
		const auto raw = static_cast<Underlying>(value);

		if constexpr(sizeof(Underlying) <= sizeof(int32_t)) {
			tag.putInt(key, static_cast<int32_t>(raw));
		} else {
			tag.putInt64(key, static_cast<int64_t>(raw));
		}
	}

	static bool read(const CompoundTag& tag, const std::string& key, T& value) {
		if constexpr(sizeof(Underlying) <= sizeof(int32_t)) {
			auto raw = tag.getInt(key);
			value = static_cast<T>(static_cast<Underlying>(raw));
			return true;
		} else {
			auto raw = tag.getInt64(key);
			value = static_cast<T>(static_cast<Underlying>(raw));
			return true;
		}

		return false;
	}
};

inline uint64_t getNextBlockStateID() {
    uint64_t lastID = 0;
    BlockState::StateListNode* node = BlockState::StateListNode::mHead;

    // Traverse to the last node in the list
    while (node && node->mNext) {
        node = node->mNext;
    }

    if (node && node->mState) {
        lastID = node->mState->mID;
    }

    return lastID + 1;
}

template<typename T>
class CustomBlockState : public BlockState {
public:
	CustomBlockState(const HashedString& name, uint64_t variationCount) 
		: BlockState(getNextBlockStateID(), variationCount, name) 
	{
		registerSelf();
		// Log::Info("Registered CustomBlockState: {} with ID {}", name.getString(), mID);
	}

	virtual void toNBT(CompoundTag& tag, int val) const override {
		T typedValue = static_cast<T>(val);
		CustomBlockStateSerializer<T>::write(tag, mName.getString(), typedValue);
	}

	virtual bool fromNBT(const CompoundTag& tag, int& val) const override {
		T typedValue{};

		if (!CustomBlockStateSerializer<T>::read(tag, mName.getString(), typedValue))
			return false;

		val = static_cast<int>(typedValue);
		return true;
	}

private:
    void registerSelf() {
        auto* last = BlockState::StateListNode::mHead;
		if (!last) {
			// List empty
			mNode.mPrev = nullptr;
			mNode.mNext = nullptr;
			mNode.mState = this;
			BlockState::StateListNode::mHead = &mNode;
			return;
		}

		// Traverse to tail
		while (last->mNext) {
			last = last->mNext;
		}

		// Append this node
		mNode.mPrev = last;
		mNode.mNext = nullptr;
		mNode.mState = this;
		last->mNext = &mNode;
    }
};
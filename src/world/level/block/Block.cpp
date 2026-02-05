#include "mc/src/common/world/level/block/Block.hpp"
#include "mc/src/common/world/level/block/BlockLegacy.hpp"
#include "mc/src/common/world/phys/AABB.hpp"
#include "mc/src/common/world/Facing.hpp"

bool Block::hasState(const BlockState& stateType) const {
    auto& states = mLegacyBlock->mStates;
	auto foundState = states.find(stateType.mID);
	return foundState != states.end();
}

// 1.20.71.1 - Partial reimplementation using `BlockLegacy::getMapColor`
mce::Color Block::getMapColor(BlockSource& region, const BlockPos& pos) const
{
    return this->mLegacyBlock->getMapColor(region, pos, *this);
}

bool Block::canBeBuiltOver(BlockSource& region, const BlockPos& pos) const {
    return mLegacyBlock->canBeBuiltOver(region, pos);
}

bool Block::isAir() const {
    return mLegacyBlock->isAir();
}

bool Block::isFenceBlock() const
{
    return mLegacyBlock->isFenceBlock();
}

bool Block::getCollisionShape(AABB& outAABB, const BlockSource& region, const BlockPos& pos, optional_ref<const GetCollisionShapeInterface> entity) const
{
    outAABB = this->mLegacyBlock->getCollisionShape(*this, region, pos, entity);
    return outAABB.min.x < outAABB.max.x && outAABB.min.y < outAABB.max.y && outAABB.min.z < outAABB.max.z;
}

const Material& Block::getMaterial() const
{
    return mLegacyBlock->mMaterial;
}

bool Block::isInteractiveBlock() const
{
    using function = decltype(&Block::isInteractiveBlock);
    static auto func = std::bit_cast<function>(SigScan("40 53 48 83 EC ? 48 8B D9 48 8B 49 ? 48 85 C9 74 ? F6 81"));
    return (this->*func)();
}

bool Block::hasProperty(BlockProperty property) const
{
    return mLegacyBlock->hasProperty(property);
}

bool Block::requiresCorrectToolForDrops() const {
    if (!mLegacyBlock)
		AssertFail("mLegacyBlock is null");
	return !mLegacyBlock->mMaterial.mAlwaysDestroyable && !mLegacyBlock->mCanDropWithAnyTool;
}
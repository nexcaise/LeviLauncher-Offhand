#include "world/gamemode/GameMode.hpp"
#include "world/level/BlockSource.hpp"
#include "world/level/block/Block.hpp"
#include "world/actor/player/Player.hpp"
#include "world/item/Item.hpp"

#include <pl/Signature.h>

// Complete implementation
// 1.21.0.3 - no address, inlined.
BlockPos GameMode::_calculatePlacePos(ItemStack& heldStack, const BlockPos& pos, FacingID& face) {
    const Item* item = heldStack.getItem();

    if (item) {
        BlockPos result = pos;
        item->calculatePlacePos(heldStack, mPlayer, face, result);
        return result;
    }

    BlockSource& blockSource = mPlayer.getDimensionBlockSource();
    const Block& block = blockSource.getBlock(pos);

    if (block.canBeBuiltOver(blockSource, pos)) {
        face = Facing::Name::UP;
        return pos;
    }
    
    return pos.neighbor(face);
}

bool GameMode::_canUseBlock(const Block& block) const
{
    return mPlayer.canUseOperatorBlocks() || !block.hasProperty(BlockProperty::OperatorBlock);
}

float GameMode::getMaxPickRange() const
{
    if (this->mPlayer.isCreative()) return 12.0f;
    return 7.0f;
}

bool GameMode::baseUseItem(ItemStack& stack)
{
    using function = decltype(&GameMode::baseUseItem);
    uintptr_t addr = pl::signature::pl_resolve_signature(
        "FF 83 04 D1 FD 7B 0D A9 FC 73 00 F9 F8 5F 0F A9 F6 57 10 A9 F4 4F 11 A9 FD 43 03 91 57 D0 3B D5 35 85 01 F0",
        "libminecraftpe.so"
    );
    static auto func = std::bit_cast<function>(addr);
    return (this->*func)(stack);
}
/*
InteractionResult _sendUseItemOnEvents(ItemStack& item, const BlockPos& pos, FacingID face, const Vec3& clickPos)
{
    using function = decltype(&GameMode::_sendUseItemOnEvents);
    static auto func = std::bit_cast<function>(SigScan("40 55 53 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 4D 8B F9 4D 8B E0 48 8B F2 4C 8B F1 4C 8B AD"));
    return (this->*func)(item, pos, face, clickPos);
}

void GameMode::_sendPlayerInteractWithBlockAfterEvent(Player& player, const BlockPos& at, FacingID face, const Vec3& hit)
{
    using function = decltype(&GameMode::_sendPlayerInteractWithBlockAfterEvent);
    static auto func = std::bit_cast<function>(SigScan("48 89 5C 24 ? 48 89 74 24 ? 55 57 41 56 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 41 0F B6 F1 49 8B D8 4C 8B F2 48 8B BD"));
    (this->*func)(player, at, face, hit);
}
*/
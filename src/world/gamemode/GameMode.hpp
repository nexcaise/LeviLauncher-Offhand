///@symbolgeneration
#pragma once
//#include <amethyst/Imports.hpp>
#include <memory>
#include <chrono>
#include <functional>
#include "world/level/BlockPos.hpp"
#include "world/phys/Vec3.hpp"
#include "world/Facing.hpp"
#include "world/item/Item.hpp"
//#include <mc/src-deps/input/InputMode.hpp>

class Player;
//class IGameModeTimer;
//class IGameModeMessenger;
class ItemStack;

class GameMode {
public:
    class BuildContext {
    public:
        bool mHasBuildDirection;
        bool mHasLastBuiltPosition;
        bool mLastBuildBlockWasInteractive;
        bool mLastBuildBlockWasSnappable;
        BlockPos mLastBuiltBlockPosition;
        BlockPos mBuildDirection;
        BlockPos mNextBuildPos;
        unsigned char mContinueFacing;
    };

    Player& mPlayer;
    Vec3 mPlayerLastPosition;
    GameMode::BuildContext mBuildContext;
    std::chrono::steady_clock::time_point mLastBuildTime;

    virtual InteractionResult useItemOn(ItemStack& item, const BlockPos pos, FacingID face, const Vec3& clickPos, const Block* block);
/*
    InteractionResult _sendUseItemOnEvents(ItemStack& item, const BlockPos& pos, FacingID face, const Vec3& clickPos);

    bool baseUseItem(ItemStack& stack);

    void _sendPlayerInteractWithBlockAfterEvent(Player& player, const BlockPos& pos, FacingID face, const Vec3& at);
*/
    bool _canUseBlock(const Block& block) const;

    float getMaxPickRange() const;

    bool isLastBuildBlockInteractive() const {
        return mBuildContext.mLastBuildBlockWasInteractive;
    }

    BlockPos _calculatePlacePos(ItemStack& heldStack, const BlockPos& pos, FacingID& face);
};
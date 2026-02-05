#include "hooks/GameModeHooks.hpp"

InteractionResult* GameMode_useItemOn(GameMode* self, ItemStack& item, const BlockPos& at, FacingID face, const Vec3& hit, const Block* targetBlock)
{
    InteractionResult* result;
    // todo: seems to set self+0x40 to 0
    bool isSneakDown = false; // todo: get this

    result->mResult = 0;

    // idk what this does but it doesn't seem to have anything to do with networking / decreasing item stack
    if ((self->_sendUseItemOnEvents(item, at, face, hit).mResult & (int)InteractionResult::Result::SUCCESS) == 0) return result;

    // todo: check that the player has use ability
    bool hasPermissions = true; 
    if (!hasPermissions) return result;

    BlockSource& region = self->mPlayer.getDimensionBlockSource();
	const Block& block = region.getBlock(at);
	const Block& liquidBlock = region.getLiquidBlock(at);
	const BlockLegacy& blockLegacy = *block.mLegacyBlock.get();

    // todo: game checks if its invisible bedrock

    if (targetBlock) {
		uint64_t blockSerializationHash = targetBlock->mSerializationIdHash;

        if (block.mSerializationIdHash != blockSerializationHash && liquidBlock.mSerializationIdHash != blockSerializationHash)
			return result;
    }

	const BlockSource& constRegion = self->mPlayer.getDimensionBlockSourceConst();

    AABB blockCollision = AABB::BLOCK_SHAPE;
    bool isBlockSolid = block.getCollisionShape(blockCollision, constRegion, at, nullptr);
    bool isItemBucket = item.isInstance("minecraft:bucket", false);

	const HashedString& blockFullName = blockLegacy.mNameInfo.mFullName;
	bool isPowderedSnow = blockFullName == "minecraft:powder_snow";
    bool canContainLiquid = blockLegacy.canContainLiquid();

	// Handles clicking with buckets on non-powdered snow blocks, or blocks that cannot contain liquids
	if (isBlockSolid && isItemBucket && !isPowderedSnow && !canContainLiquid) return result;

	BlockLegacy* blockToUse = nullptr;

    bool itemCanInteractWithBlock = item.hasTag("minecraft:is_axe")
        || item.hasTag("minecraft:is_shovel")
        || item.hasTag("minecraft:is_hoe")
        || item.isInstance("minecraft:honeycomb", false);

    if (!isSneakDown || itemCanInteractWithBlock || !item) {
		blockToUse = block.mLegacyBlock;
        bool isAir = blockToUse->isAir();

        if (!isAir && block.use(self->mPlayer, at, face, hit)) {
			self->mBuildContext.mLastBuildBlockWasInteractive = block.isInteractiveBlock();

            bool wasSnappable = false;

            if (!self->mBuildContext.mLastBuildBlockWasInteractive && item.isBlock()) {
                wasSnappable = blockToUse->isSnappableBlock();
            }

            // This is probably just for scripting so dont care
			// ItemEventCoordinator::sendEvent(v42, &v67); <- ItemUsedOnEvent
            self->_sendPlayerInteractWithBlockAfterEvent(self->mPlayer, at, face, hit);

			self->mBuildContext.mLastBuildBlockWasSnappable = wasSnappable;
			result->mResult = (int)InteractionResult::Result::SUCCESS | (int)InteractionResult::Result::SWING;

            return result;
        }
    }
     
    if (!item) {
        logger.info("Todo handle !item scenario");
		return result;
    }

    if (!item.isBlock()) {
        self->mBuildContext.mLastBuildBlockWasSnappable = blockLegacy.isSnappableBlock();

        bool isCreative = false;
		InteractionResult useResult;

        if (isCreative) {
            logger.info("todo handle creative");
        }
        else {
            useResult.mResult = item.useOn(self->mPlayer, at.x, at.y, at.z, face, hit).mResult;
        }

        if ((useResult.mResult & (int)InteractionResult::Result::SUCCESS) != 0) {
            
            result->mResult = useResult.mResult; 
            return result;
		}

        if (item) {
            logger.info("Todo: Player::setSelectedItem()");
        }
        else {
            logger.info("Todo: PlayerInventory::clearSlot()");
        }

        logger.info("Todo: GameMode::_sendPlayerInteractWithBlockAfterEvent");

        return result;
    }

    blockToUse = item.getLegacyBlock();
    const Block& renderBlock = blockToUse->getRenderBlock();

    if (self->_canUseBlock(renderBlock)) {
        bool isInCreative = false;

        if (isInCreative) {
			logger.info("Todo handle Using block in creative mode");
            return result;
        }
        else {
            result->mResult = item.useOn(self->mPlayer, at.x, at.y, at.z, face, hit).mResult;
        }
    }

    return result;
}

bool tryUseItem(GameMode &self, const ItemStack &stack, const Player &player, Container &container, int slot, ContainerID containerId, const BlockPos &pos, FacingID face, bool isSimTick, bool isOffhand)
{
    const Item *item = stack.getItem();
    // I don't know what the fuck this is, or what it filters out xD.
    bool realMayUse = !stack.mValid || stack.mItem == nullptr || stack.isNull() || stack.mCount == 0 || !isSimTick || !item || item->canUseOnSimTick();

    if (!realMayUse)
    {
        logger.info("May not use item: {}", stack);
        return false;
    }

    self.mPlayerLastPosition = *player.getPosition();
    std::unique_ptr<ItemUseInventoryTransaction> transaction = std::make_unique<ItemUseInventoryTransaction>();

    bool useLegacyTransaction = player.isClientSide() && player.mItemStackNetManager != nullptr;
    auto onScopeClosed = useLegacyTransaction
                             ? player.mItemStackNetManager->_tryBeginClientLegacyTransactionRequest()
                             : gsl::final_action<std::function<void()>>([]() {});

    InteractionResult result;

    container.createTransactionContext(
        [&transaction, &self, containerId](Container &container, unsigned int slot, const ItemStack &oldStack, const ItemStack &newStack)
        {
            InventorySource source(InventorySourceType::ContainerInventory, containerId, InventorySource::InventorySourceFlags::NoFlag);
            InventoryAction action(source, slot, oldStack, newStack);

            self.mPlayer.mTransactionManager.addExpectedAction(action);
            transaction->mTransaction.addAction(action);
        },
        [&transaction, &self, pos, face, &result, &stack, slot, isOffhand]()
        {
            Player &player = self.mPlayer;
            ILevel &level = *player.mLevel;

            ItemStack stackCopy = stack;
            ItemInstance itemInstance(stackCopy);
            NetworkItemStackDescriptor networkDescriptor(stackCopy);

            transaction->mItem = networkDescriptor;
            transaction->mSlot = slot;

            HitResult hitResult = level.getHitResult();
            HitResult liquidHit = level.getLiquidHitResult();

            const BlockSource &region = player.getDimensionBlockSourceConst();
            const Block &block = region.getBlock(pos);

            FacingID faceDir = face;
            BlockPos calculatedPlacePos = self._calculatePlacePos(stackCopy, pos, faceDir);

            if (hitResult.mType == HitResultType::TILE || liquidHit.mLiquid.y == 0 || self.mBuildContext.mHasBuildDirection)
            {
                transaction->mPos = pos;
                transaction->setTargetBlock(block);
                transaction->mFace = faceDir;

                Vec3 clickPos;

                if (hitResult.mType == HitResultType::NO_HIT)
                {
                    clickPos = Vec3::ZERO;
                }
                else
                {
                    clickPos = hitResult.mPos - Vec3(pos);
                }

                transaction->mClickPos = clickPos;
                transaction->mFromPos = *player.getPosition();
                transaction->mActionType = face == FacingID::MAX ? ItemUseInventoryTransaction::ActionType::Use : ItemUseInventoryTransaction::ActionType::Place;

                // if (block.isFenceBlock() && LeadItem::canBindPlayerMobs) {
                if (block.isFenceBlock() && false)
                {
                    result.mResult = (int)InteractionResult::Result::SUCCESS | (int)InteractionResult::Result::SWING;
                }
                else
                {
					GameMode_useItemOn(&self, &result, stackCopy, pos, face, hitResult.mPos, nullptr);
                }
            }

            // This section handles liquid clipped items, i.e. lillypads on water
            bool isLiquidClippingItem = (result.mResult & (int)InteractionResult::Result::SUCCESS) == 0 && !stackCopy.isNull() && stackCopy.isLiquidClipItem() && !stackCopy.shouldInteractionWithBlockBypassLiquid(block);

            if (isLiquidClippingItem)
            {
                logger.info("Todo: Handle liquid clipping items");
            }

            // Block placement timings
            if ((result.mResult & (int)InteractionResult::Result::SUCCESS) != 0)
            {
                self.mBuildContext.mLastBuiltBlockPosition = calculatedPlacePos;

                // Seems to get reset often enogu to be false each item use.
                if (!self.mBuildContext.mHasLastBuiltPosition)
                {
                    //self.mMessenger->startSendItemUseOn(pos, self.mBuildContext.mLastBuiltBlockPosition, (int)face);
                    self.mBuildContext.mHasLastBuiltPosition = true;
                }

                self.mLastBuildTime = std::chrono::steady_clock::now();
                if ((result.mResult & (int)InteractionResult::Result::SWING) != 0)
                {
                    if (!isOffhand) player.swing();

                    else {
						OffhandSwingComponent* _swingComp = player.tryGetComponent<OffhandSwingComponent>();
                        if (!_swingComp) {
							player.addComponent<OffhandSwingComponent>();
                        }

						OffhandSwingComponent* swingComp = player.tryGetComponent<OffhandSwingComponent>();
                        
                        // player.swing() implemented using OffhandSwingComponent instead off mainhand.
                        int currentSwingDuration = player.getCurrentSwingDuration();
                        int time = swingComp->mOffhandSwingTime;

                        if (!swingComp->mOffhandSwinging || time >= currentSwingDuration / 2 || time < 0) {
							swingComp->mOffhandSwinging = true;
							swingComp->mOffhandSwingTime = -1;
                        }
                    }
                }
            }
        });

    if (player.isClientSide())
    {
        player.sendComplexInventoryTransaction(std::move(transaction));
    }

    if (result.mResult & (int)InteractionResult::Result::SUCCESS)
    {
        return true;
	}

    return false;
}

// Modded version of GameMode::buildBlock that supports offhand items
// note: the game only seems to call this on the client?
// I wasn't able to see it called on the server when hooking it and logging the caller

bool GameMode_buildBlock(GameMode *self, BlockPos *pos, FacingID face, bool isSimTick)
{
    
    Player &player = self->mPlayer;
    //ILevel &level = *player.mLevel;
    PlayerInventory &playerInv = *player.playerInventory;
    Inventory &inv = *playerInv.mInventory.get();

    const ItemStack &mainHandItem = playerInv.getSelectedItem();
    ActorEquipmentComponent *equipment = player.tryGetComponent<ActorEquipmentComponent>();
    const ItemStack &offHandItem = equipment->mHand->mItems[1];

    bool usedMainhand = tryUseItem(*self, mainHandItem, player, inv, playerInv.mSelected, ContainerID::CONTAINER_ID_INVENTORY, *pos, face, isSimTick, false);
    if (usedMainhand) return true;
    
    return tryUseItem(*self, offHandItem, player, *equipment->mHand, 1, ContainerID::CONTAINER_ID_OFFHAND, *pos, face, isSimTick, true);
}
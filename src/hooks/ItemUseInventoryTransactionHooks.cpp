
InventoryTransactionError ItemUseInventoryTransaction_handle(ItemUseInventoryTransaction* self, Player& player, bool isSenderAuthority) {
    if (!player.isAlive()) {
		Log::Warning("InventoryTransactionError::StateMismatch - Player is not alive");
		return InventoryTransactionError::StateMismatch;
    }

	Level& level = *player.getLevel()->asLevel();
	bool isClientSide = level.isClientSide;
    BlockPalette& blockPalette = level.getBlockPalette();

    ItemStack stack = ItemStack::fromDescriptor(self->mItem, blockPalette, isClientSide);

    PlayerInventory& playerInv = *player.playerInventory;
    Inventory& inv = *playerInv.mInventory.get();
    const ItemStack& mainHandItem = playerInv.getSelectedItem();
    ActorEquipmentComponent* equipment = player.tryGetComponent<ActorEquipmentComponent>();
    const ItemStack& offHandItem = equipment->mHand->mItems[1];

    // This could probably be more verbose in checking, but the logic is if the two stacks are the same, it was probably the mainhand?
    // There probably is some weird edgecase around this
    bool usedMainhand = stack.mItem == mainHandItem.mItem;
    const ItemStack& stackToUse = usedMainhand ? mainHandItem : offHandItem;
    Container* containerToUse = usedMainhand ? &inv : (Container*)equipment->mHand.get();

    bool itemsMatch = true; // InventoryTransaction::checkTransactionItemsMatch
	bool slotsMatch = true; // self->mSlot == playerInv.mSelected; <- to check properly probs need to switch between offhand and inv slots

    bool areItemsAndSlotsValid = isSenderAuthority || itemsMatch && slotsMatch;

    if (!areItemsAndSlotsValid) {
		Log::Warning("InventoryTransactionError::ProbablyError - Items or slots do not match");
        return InventoryTransactionError::ProbablyError;
    }

    GameMode& gameMode = player.getGameMode();
    float maxPickRange = gameMode.getMaxPickRange() + 0.5f;
    bool distanceCheck = player.distanceTo(self->mPos) <= maxPickRange;

    bool isValidAction = self->mActionType == ItemUseInventoryTransaction::ActionType::Use || distanceCheck || isSenderAuthority;

    if (!isValidAction) {
        return InventoryTransactionError::ProbablyError;
    }

    ContainerID containerId = usedMainhand ? ContainerID::CONTAINER_ID_INVENTORY : ContainerID::CONTAINER_ID_OFFHAND;

	InventorySource source(
        InventorySourceType::ContainerInventory, 
        containerId,
        InventorySource::InventorySourceFlags::NoFlag
    );

	const std::vector<InventoryAction>& actions = self->mTransaction.getActions(source);
    
    for (auto& action : actions) {
        Log::Info("Todo: action not handled!!");
        // todo: game does something, there are no actions sent for placing blocks it seems?
    }

    playerInv.createTransactionContext(
        [&self, &player, containerId](Container& container, unsigned int slot, const ItemStack& oldStack, const ItemStack newStack) {
            InventorySource source(InventorySourceType::ContainerInventory, containerId, InventorySource::InventorySourceFlags::NoFlag);
            InventoryAction action(source, slot, oldStack, newStack);
            player.mTransactionManager.addExpectedAction(action);
        },
        [&self, &gameMode, &stackToUse, &level, &containerToUse]() {
            if (self->mActionType == ItemUseInventoryTransaction::ActionType::Use) {
				ItemStack stackCopy = ItemStack(stackToUse);

                // Seems to return false for things the server needs to update the client on
                // i.e. stone block returns true
                // but torch item returns false
                if (!gameMode.baseUseItem(stackCopy)) {
					self->resendPlayerState(gameMode.mPlayer);
                }
            }
            else if (self->mActionType == ItemUseInventoryTransaction::ActionType::Destroy) {
				Log::Info("todo impl self->mActionType == ItemUseInventoryTransaction::ActionType::Destroy");
            }
            else if (self->mActionType == ItemUseInventoryTransaction::ActionType::Place) {
				bool isClientSide = level.isClientSide;
                BlockPalette& blockPalette = level.getBlockPalette();
                ItemStack stackCopy = ItemStack(stackToUse);

                // Idk why this exists or what the fuck it does... 
                //if (!Actor::isSneaking(*(a1 + 8)))
                //{
                //    v8 = Player::getSelectedItem(*(a1 + 8));
                //    ItemStack::operator=(&itemStack, v8);
                //}

				const Block& blockToPlaceOn = blockPalette.getBlock(self->mTargetBlockId);

                Block* v18 = nullptr;
                //Actor::setPos(*(a1 + 8), (*a1 + 232i64));

                /*while (1)
                {
                    v18 = *v17;
                    if ((*v17)->mSerializationIdHash == v14->mSerializationIdHash)
                        break;
                    mLegacyBlock = v18->mLegacyBlock;
                    if (!mLegacyBlock || (v16 = v14->mLegacyBlock) == 0i64)
                        gsl::details::terminate(v16);
                    if (mLegacyBlock == v16 && Block::allowStateMismatchOnPlacement(*v17, v14))
                        break;
                    if (++v17 == &v42)
                        goto LABEL_17;
                }*/

                InteractionResult result = gameMode.useItemOn(stackCopy, self->mPos, self->mFace, self->mClickPos, v18);
                // Actor::setPos(*(a1 + 8), v39);

                if ((result.mResult & (int)InteractionResult::Result::SUCCESS) != 0)
                {
                    bool itemsMatch = false; // todo: ItemStackBase::operator!=(&itemStack, v25)

                    if (!itemsMatch) {
                        containerToUse->setItem(self->mSlot, stackCopy);
                    }   

                    if (gameMode.isLastBuildBlockInteractive()) {
                        // I'm pretty sure this is just for scripting and so i dont really care to impl it.
                         BlockEventCoordinator& blockEvents = level.getBlockEventCoordinator();
                         blockEvents.sendBlockInteractedWith(gameMode.mPlayer, self->mPos);
                    }
                }
                else {
					self->resendBlocksAroundArea(gameMode.mPlayer, self->mPos, self->mFace);
					self->resendPlayerState(gameMode.mPlayer);
                }
            }
        }
    );

    // What on earth does this mean
    // ComplexInventoryTransaction::_setDepenetrationOverride(this, (player + 8));

    return InventoryTransactionError::ProbablyError; // todo: return correct error code
}
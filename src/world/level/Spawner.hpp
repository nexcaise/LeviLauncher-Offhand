/// @symbols
#pragma once
#include <amethyst/Imports.hpp>

class ItemActor;
class BlockSource;
class ItemStack;
class Actor;
class Vec3;

class Spawner {
public:
	/// @sig {48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 45 ? 49 8B F9 49 8B F0 4C 8B FA 4C 8B F1}
	MC ItemActor* spawnItem(BlockSource& region, const ItemStack& stack, Actor* spawner, const Vec3& pos, int throwTime);
};
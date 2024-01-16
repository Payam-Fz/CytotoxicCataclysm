#pragma once

#include <vector>

#include "tiny_ecs_registry.hpp"
#include "common.hpp"
#include "world_init.hpp"
#include "world_system.hpp"

class AISystem
{
public:
	void step(float elapsed_ms);

private:
	Entity player; // Keep reference to player entity
	void move_enemies(float elapsed_ms);
	void enemy_shoot(float elapsed_ms);
	void move_articulated_part(float elapsed_seconds, Entity partEntity, Motion& partMotion, Transform& partTranform, Transform& playerTransform);
	void enemy_dash(float elapsed_ms);
	void enemy_special_attack(Entity enemy);
	void spread_attack(Entity enemy);
	void clone_attack(Entity enemy, int clones);
	void swarm_keep_distance(float elapsed_ms);
	void swarm_block_interestpoint(float elapsed_ms);
};

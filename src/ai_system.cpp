// internal
#include "ai_system.hpp"

void AISystem::step(float elapsed_ms)
{
	// update player entity
	auto players = registry.players.entities;
	if (!players.empty()) {
		player = players.back();
	}
	if (MOVE_ENEMIES) {
		move_enemies(elapsed_ms);
		swarm_keep_distance(elapsed_ms);
		swarm_block_interestpoint(elapsed_ms);
	};
	enemy_shoot(elapsed_ms);
	enemy_dash(elapsed_ms);
}

void AISystem::move_enemies(float elapsed_ms) {
	Transform& playerTransform = registry.transforms.get(player);
	for (Entity entity : registry.enemies.entities) {
		if (registry.motions.has(entity)) {
			assert(registry.transforms.has(entity));
			Transform& enemytransform = registry.transforms.get(entity);
			Motion& enemymotion = registry.motions.get(entity);
			if (registry.attachments.has(entity)) {
				float elapsed_seconds = elapsed_ms / 1000.f;
				move_articulated_part(elapsed_seconds, entity, enemymotion, enemytransform, playerTransform);
				continue;
			}
			if (enemymotion.allow_accel == false) {
				enemymotion.allow_accel = true;
				continue;
			}

			// Boss chases player forever after it's activated
			Enemy& enemyAttribute = registry.enemies.get(entity);
			vec2 target_point = playerTransform.position;
			if (enemyAttribute.type == ENEMY_ID::BOSS) {
				if (!registry.bosses.get(entity).activated) {
					enemymotion.max_velocity = 0.f;
					//target_point = enemytransform.position;
					//enemytransform.angle = atan2f(target_point.y - enemytransform.position.y, target_point.x - enemytransform.position.x) ;
					//for (auto& region : registry.regions.components) {
					//	if (region.goal == REGION_GOAL_ID::CURE) {
					//		target_point = region.interest_point;
					//	}
					//}
				}
			} else if (enemyAttribute.type == ENEMY_ID::FRIENDBOSS) {
				if (!registry.bosses.get(entity).activated) {
					enemymotion.max_velocity = 0.f;
					//target_point = enemytransform.position;
					//enemytransform.angle = atan2f(target_point.y - enemytransform.position.y, target_point.x - enemytransform.position.x) + enemytransform.angle_offset;
					//for (auto& region : registry.regions.components) {
					//	if (region.goal == REGION_GOAL_ID::CANCER_CELL) {
					//		target_point = region.interest_point;
					//	}
					//}
				}
				Dash& enemyDash = registry.dashes.get(entity);
				if (enemyDash.active_timer_ms > 0.f) {
					target_point = enemytransform.position;	// Do not move
				}
			}
			enemymotion.force += normalize(vec2(target_point.x - enemytransform.position.x, target_point.y - enemytransform.position.y));
		}
	}
}

void AISystem::move_articulated_part(float elapsed_seconds, Entity partEntity, Motion& partMotion, Transform& partTranform, Transform& playerTransform) {
	assert(registry.attachments.has(partEntity));

	vec2 dist = { playerTransform.position - partTranform.position };
	if (length(dist) <= CONTENT_HEIGHT_PX / 2.f) {
		partMotion.angular_velocity = 0.f;
		partMotion.force = normalize(dist);
	} else {
		// Retract the articulated part to the original position
		Attachment& att = registry.attachments.get(partEntity);
		if (fabs(att.moved_angle) > ANGLE_PRECISION) {
			float angle_remaining = -att.moved_angle;
			float required_velocity = fabs(angle_remaining) / elapsed_seconds;
			float velocity_magnitude = fmin(required_velocity, partMotion.max_angular_velocity);
			partMotion.angular_velocity = velocity_magnitude * sign(angle_remaining);
		}
		else {
			partMotion.angular_velocity = 0.f;
			att.moved_angle = 0.f;
		}
	}
}


void AISystem::swarm_keep_distance(float elapsed_ms) {
	Transform& playerTransform = registry.transforms.get(player);
	for (Entity entity : registry.enemies.entities) {
		if (registry.motions.has(entity)			// Ignore if can't move
			&& !registry.bosses.has(entity)			// Ignore if is a boss
			&& !registry.attachments.has(entity)) { // Ignore if is an attachment
			Motion& enemymotion = registry.motions.get(entity);
			if (enemymotion.max_velocity == 0.f) continue;	// Ignore if can't move

			assert(registry.transforms.has(entity));
			Transform& enemytransform = registry.transforms.get(entity);

			// Find the closest enemy
			float minDist = INFINITY;
			Entity closestEnemy;
			for (Entity otherEnemy : registry.enemies.entities) {
				if (entity != otherEnemy && registry.transforms.has(otherEnemy)) {
					vec2 otherEnemyPos = registry.transforms.get(otherEnemy).position;
					float dist = length(otherEnemyPos - enemytransform.position);
					if (dist < minDist) {
						minDist = dist;
						closestEnemy = otherEnemy;
					}
				}
			}
			// Add a small repelling force from the closest enemy
			if (registry.transforms.has(closestEnemy)) {
				vec2 closestEnemyPos = registry.transforms.get(closestEnemy).position;
				enemymotion.force += normalize(enemytransform.position - closestEnemyPos) / 2.f;
			}
		}
	}
}


void AISystem::swarm_block_interestpoint(float elapsed_ms) {
	Transform& playerTransform = registry.transforms.get(player);
	// Find closest interest point to the player
	vec2 closest_interest_point;
	float min_dist = INFINITY;
	for (Waypoint& wp : registry.waypoints.components) {
		float dist = length(wp.interest_point - playerTransform.position);
		if (dist <= min_dist) {
			closest_interest_point = wp.interest_point;
			min_dist = dist;
		}
	}
	for (uint i = 0; i < registry.enemies.size(); i++) {
		Entity entity = registry.enemies.entities[i];
		Enemy& enemyAttrib = registry.enemies.components[i];
		if (registry.motions.has(entity)						// Ignore if can't move
			&& enemyAttrib.type != ENEMY_ID::FRIENDBOSSCLONE	// Ignore if is boss clone
			&& enemyAttrib.type != ENEMY_ID::FRIENDBOSS			// Ignore if is boss
			&& enemyAttrib.type != ENEMY_ID::BOSS				// Ignore if is boss
			&& enemyAttrib.type != ENEMY_ID::BOSS_ARM) {		// Ignore if is boss arm
			Motion& enemymotion = registry.motions.get(entity);
			if (enemymotion.max_velocity == 0.f) continue;	// Ignore if can't move
			
			assert(registry.transforms.has(entity));
			Transform& enemytransform = registry.transforms.get(entity);
			if (length(closest_interest_point - enemytransform.position) > SCREEN_RADIUS * 1.5f) continue;	// Ignore if too far

			// Add a small attraction force towards the interest-point
			enemymotion.force += (closest_interest_point - enemytransform.position) / (SCREEN_RADIUS * 1.5f) * 1.5f;
		}
	}
}

void AISystem::enemy_shoot(float elapsed_ms) {
	for (Entity entity : registry.guns.entities) {
		Gun& enemyGun = registry.guns.get(entity);
		vec2 playerposition = registry.transforms.get(player).position;
		Transform& enemy_transform = registry.transforms.get(entity);
		vec2 distance = abs(playerposition - enemy_transform.position) - length(enemy_transform.scale / 2.f);	// As soon as the enemy is partially visible
		if (registry.enemies.has(entity) && distance.x <= CONTENT_WIDTH_PX / 2 && distance.y <= CONTENT_HEIGHT_PX / 2) {
			if (enemyGun.attack_timer <= 0) {
				Enemy& enemy = registry.enemies.get(entity);
				if (enemy.type == ENEMY_ID::BOSS) {
					if (registry.bosses.get(entity).activated) {
						createBullet(entity, enemyGun.bullet_size, enemyGun.bullet_color);
					}
				} else if (enemy.type == ENEMY_ID::FRIENDBOSS) {
					if (registry.bosses.get(entity).activated) {
						float decision = (static_cast<float>(rand()) / RAND_MAX); //This generates num between 0 and 1
						if (decision <= 0.7f) {
							createBullet(entity, enemyGun.bullet_size, enemyGun.bullet_color);
						}
						else {
							enemy_special_attack(entity);
						}
					}
				} else {
					createBullet(entity, { 13.f, 13.f }, { 0.718f, 1.f, 0.f, 1.f });
				}
				enemyGun.attack_timer = enemyGun.attack_delay;
			}
		}
		enemyGun.attack_timer = max(enemyGun.attack_timer - elapsed_ms, 0.f);

	}
}

void AISystem::enemy_dash(float elapsed_ms) {
	vec2 playerposition = registry.transforms.get(player).position;
	for (Entity entity : registry.dashes.entities) {
		Transform enemyTransform = registry.transforms.get(entity);
		if (registry.enemies.has(entity) && registry.bosses.has(entity) && registry.bosses.get(entity).activated) {
			Enemy& enemyAttrib = registry.enemies.get(entity);
			Dash& enemyDash = registry.dashes.get(entity);
			
			if (enemyDash.delay_timer_ms <= 0.f) {
				// setup a new dash
				Motion& enemymotion = registry.motions.get(entity);
				float currAngle = enemyTransform.angle - enemyTransform.angle_offset;
				vec2 targetDiff = playerposition - enemyTransform.position;
				float targetAngle = atan2f(targetDiff.y, targetDiff.x);
				float distance = length(targetDiff) - length(enemyTransform.scale) - 100.f;
				float angleRemaining = fabs(targetAngle - currAngle);
				float dashVelocity; vec2 dashDirection;
				if (distance < 300.f && enemyAttrib.type == ENEMY_ID::FRIENDBOSS) {
					// Dashing around the player
					float decision = (static_cast<float>(rand()) / RAND_MAX); //This generates num between 0 and 1
					float rightOrleft = decision < 0.5 ? M_PI / 2.f : -M_PI / 2.f;
					dashDirection = normalize(vec2(cos(currAngle + rightOrleft), sin(currAngle + rightOrleft)));
					dashVelocity = enemyDash.max_dash_velocity;
				}
				else if (angleRemaining <= M_PI / 9.f || angleRemaining - M_PI < ANGLE_PRECISION) {
					// Dashing towards the player
					dashDirection = normalize(vec2(cos(currAngle), sin(currAngle)));
					// Do not pass the player
					if (enemyDash.max_dash_velocity * enemyDash.active_duration_ms / 1000.f > distance) {
						dashVelocity = distance / (enemyDash.active_duration_ms / 1000.f);
					}
					else {
						dashVelocity = enemyDash.max_dash_velocity;
					}
					
				} else {
					return;	// Do not dash
				}
				enemymotion.velocity += dashVelocity * dashDirection;
				enemyDash.delay_timer_ms = enemyDash.delay_duration_ms;
				enemyDash.active_timer_ms = enemyDash.active_duration_ms;
			}
			else {
				enemyDash.delay_timer_ms = max(enemyDash.delay_timer_ms - elapsed_ms, 0.f);
				enemyDash.active_timer_ms = max(enemyDash.active_timer_ms - elapsed_ms, 0.f);
			}
		}
	}

}


void AISystem::enemy_special_attack(Entity enemy) {
	Transform& enemytransform = registry.transforms.get(enemy);
	Transform& playertransform = registry.transforms.get(player);

	float decision = (static_cast<float>(rand()) / RAND_MAX); //This generates num between 0 and 1

	if (decision <= 0.9f){ //90% of the time the boss will scattershot
		spread_attack(enemy);
	}
	else {
		clone_attack(enemy, 5);
	}
}

void AISystem::spread_attack(Entity enemy) {
	Transform& enemytransform = registry.transforms.get(enemy);

	for (int i = 0; i < 6; i++) {
		enemytransform.angle += 1;
		createBullet(enemy, { 13.f, 13.f }, { 1.f, 0.8f, 0.8f, 1.f });
	}

}

void AISystem::clone_attack(Entity enemy, int clones) {
	Transform& playertransform = registry.transforms.get(player);
	
	for (int i = 0; i < clones; i++) {
		playertransform.angle += 1;
		float xpos = playertransform.position.x + cos(playertransform.angle) * 800;
		float ypos = playertransform.position.y + sin(playertransform.angle) * 800;
		createBossClone({ xpos,ypos });

	}

}

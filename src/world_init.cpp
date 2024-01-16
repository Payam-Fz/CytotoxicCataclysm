// internal
#include "world_init.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"

// stlib
#include <vector>
#include <random>
#include <algorithm>
#include <iostream>

Entity createPlayer(vec2 pos)
{
	auto entity = Entity();

	// Setting initial bullet_transform values
	Transform& transform = registry.transforms.emplace(entity);
	transform.position = pos;
	transform.scale = PLAYER_SIZE;
	transform.angle_offset = IMMUNITY_TEXTURE_ANGLE;
	transform.angle = transform.angle_offset;

	registry.motions.insert(entity, { { 0.f, 0.f } });

	// Create an (empty) Player component to be able to refer to all players
	Player& playerComp = registry.players.emplace(entity);

	createGun(entity);

	registry.collideEnemies.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::IMMUNITY_BLINKING,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITESHEET_IMMUNITY_BLINKING,
			RENDER_ORDER::PLAYER });
	Animation& animation = registry.animations.emplace(entity);
	animation.total_frame = (int)ANIMATION_FRAME_COUNT::IMMUNITY_BLINKING;
	animation.update_period_ms = 120;

	// Create a brand new health with 100 health value
	registry.healthValues.emplace(entity);


	// Add color for player
	registry.colors.insert(entity, { 1.f,1.f,1.f,1.f });
	return entity;
}

Entity createDashing(Entity dasher) {
	if (!registry.dashes.has(dasher)) {
		registry.dashes.emplace(dasher);
	}
	Entity dash_entity = Entity();
	Attachment& attachment = registry.attachments.emplace(dash_entity);
	attachment.parent = dasher;
	attachment.relative_transform_2.scale(DASHING_TEXTURE_SIZE / vec2(8, 1));
	attachment.type = ATTACHMENT_ID::DASHING;

	Animation& animation = registry.animations.emplace(dash_entity);
	animation.total_frame = (int)ANIMATION_FRAME_COUNT::DASHING;
	animation.update_period_ms = 50;

	registry.transforms.emplace(dash_entity);
	registry.motions.emplace(dash_entity);		// This motion is with respect to parent
	registry.colors.insert(dash_entity, dashing_default_color);

	registry.renderRequests.insert(
		dash_entity,
		{ TEXTURE_ASSET_ID::DASHING,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITESHEET_DASHING,
			RENDER_ORDER::OBJECTS });

	return dash_entity;
}


Entity createGun(Entity holder) {
	Gun& gun_component = registry.guns.emplace(holder);
	gun_component.angle_offset = IMMUNITY_TEXTURE_ANGLE;
	gun_component.offset = { 40.f, 50.f };
	gun_component.bullet_speed = 1400.f;
	gun_component.bullet_size = { 15.f, 15.f };
	gun_component.damage = 10.f;

	Entity gun_entity = Entity();
	Attachment& attachment = registry.attachments.emplace(gun_entity);
	attachment.type = ATTACHMENT_ID::GUN;
	attachment.parent = holder;
	attachment.relative_transform_1.translate({ 40.f, -15.f });
	attachment.relative_transform_1.rotate(IMMUNITY_TEXTURE_ANGLE + M_PI / 2);
	attachment.relative_transform_2.scale(GUN_TEXTURE_SIZE * vec2(0.5f, 1.f));
	//attachment.relative_transform_2.translate({ 0.1f, 0.f }); // This can bring the gun forward or back

	registry.transforms.emplace(gun_entity);
	registry.motions.emplace(gun_entity);		// This motion is with respect to parent
	registry.colors.insert(gun_entity, { 1.f,1.f,1.f,1.f });

	registry.renderRequests.insert(
		gun_entity,
		{ TEXTURE_ASSET_ID::GUN,
		  EFFECT_ASSET_ID::TEXTURED,
		  GEOMETRY_BUFFER_ID::SPRITE,
		  RENDER_ORDER::PLAYER_ATTACHMENTS });

	return gun_entity;
}

Entity createSword(RenderSystem* renderer, Entity& holder) {
	if (!registry.melees.has(holder)) {
		registry.melees.emplace(holder);
	}
	Entity melee_entity = Entity();
	registry.melees.get(holder).melee_entity = melee_entity;
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SWORD);
	registry.meshPtrs.emplace(melee_entity, &mesh);

	registry.collideEnemies.emplace(melee_entity);
	
	Attachment& attachment = registry.attachments.emplace(melee_entity);
	attachment.parent = holder;
	attachment.type = ATTACHMENT_ID::SWORD;
	attachment.angle_freedom = M_PI / 2.f;

	attachment.relative_transform_1.rotate(M_PI  - M_PI / 4.f);	// Setting the sword to face in correct direction
	attachment.relative_transform_1.translate({15.f, -20.f});	// Putting the center of sword in the hand
	attachment.relative_transform_1.rotate(-M_PI / 12);			// Rotate to be closer to body
	//attachment.relative_transform_1.rotate(M_PI / 4);			// Rotate to be closer to body
	attachment.relative_transform_2.scale(SWORD_SIZE);
	attachment.relative_transform_2.translate({ -0.29f, -0.39f }); // Adjust the pivot point so the handle is in hand

	registry.transforms.emplace(melee_entity);
	Motion& motion = registry.motions.emplace(melee_entity);		// This motion is with respect to parent
	motion.max_angular_velocity = 5 * M_PI;
	registry.colors.insert(melee_entity, { 1.f,1.f,1.f,1.f });

	registry.renderRequests.insert(
		melee_entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
			EFFECT_ASSET_ID::COLOURED,
			GEOMETRY_BUFFER_ID::SWORD,
			RENDER_ORDER::PLAYER_ATTACHMENTS });

	return melee_entity;

}

Entity createBoss(RenderSystem* renderer, vec2 pos, float health) {
	// Create boss components
	GameMode& gameMode = registry.gameMode.components.back();
	auto entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::BACTERIOPHAGE);
	registry.meshPtrs.emplace(entity, &mesh);

	Motion& motion = registry.motions.emplace(entity);
	motion.max_velocity = 250.f;
	motion.max_angular_velocity = gameMode.id == GAME_MODE_ID::EASY_MODE ? M_PI / 6.f : M_PI / 4.f;
	Dash& dash = registry.dashes.emplace(entity);
	dash.active_duration_ms = 1000.f;
	dash.delay_duration_ms = gameMode.id == GAME_MODE_ID::EASY_MODE ? 12000.f : 6000.f;
	dash.max_dash_velocity = 400.f;

	Transform& transform = registry.transforms.emplace(entity);
	transform.position = pos;
	transform.scale = BACTERIOPHAGE_BOSS_SIZE;
	transform.angle_offset = M_PI / 2;
	transform.angle = transform.angle_offset;

	Health& enemyHealth = registry.healthValues.emplace(entity);
	enemyHealth.health = fmin(health, gameMode.enemy_health_map[ENEMY_ID::BOSS]);
	enemyHealth.maxHealth = gameMode.enemy_health_map[ENEMY_ID::BOSS];

	// Setting initial components values
	Enemy& new_enemy = registry.enemies.emplace(entity);
	new_enemy.type = ENEMY_ID::BOSS;
	registry.bosses.emplace(entity);

	Gun& weapon = registry.guns.emplace(entity);
	weapon.damage = 15.f;
	weapon.offset = { -BACTERIOPHAGE_BOSS_SIZE.y / 2.f + 20.f, 0.f };
	weapon.angle_offset = transform.angle_offset;
	weapon.bullet_speed = 800.f;
	weapon.bullet_size = { 45.f, 45.f };
	weapon.bullet_color = { 0.f, 0.992f, 1.f, 1.f };


	// Add to render_request
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::BACTERIOPHAGE,
		  EFFECT_ASSET_ID::TEXTURED,
		  GEOMETRY_BUFFER_ID::SPRITE,
		  RENDER_ORDER::BOSS });

	createBossArms(renderer, entity, transform.scale);
	std::cout << "Creating boss at position: " << pos.x << ", " << pos.y << std::endl;
	return entity;
}

void createBossArms(RenderSystem* renderer, Entity bossEntity, vec2 bossSize) {
	// Create boss arm parts
	// Odd arms are on one side and even arms on the other.
	uint arms_count = 6;
	uint arm_parts_count = 3;
	vec2 first_joint_pos[] = { {0.21f, -0.32f}, {0.22f, -0.32f}, {0.21f, -0.32f} };	// Relative to the size of the boss
	vec2 middle_joint_pos = { 0.5, 0.f };	// Relative to the size of the arm

	for (uint arm_idx = 0; arm_idx < arms_count; arm_idx++) {
		Entity parent_entity = bossEntity;
		vec2 parent_size = bossSize;
		for (uint arm_part_idx = 0; arm_part_idx < arm_parts_count; arm_part_idx++) {
			uint arm_pair_idx = (uint)(arm_idx / 2);

			auto entity = Entity();
			Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::BACTERIOPHAGE_ARM);
			registry.meshPtrs.emplace(entity, &mesh);

			Attachment& articulated = registry.attachments.emplace(entity);
			articulated.type = ATTACHMENT_ID::BACTERIOPHAGE_ARM;
			articulated.parent = parent_entity;
			articulated.angle_freedom = M_PI / 12.f + M_PI / 12.f * (arm_part_idx + 1);

			// Calculate and store the relative position of joint (pivot) with respect to its parent
			vec2 flipper = { 1.f, 1.f };
			if (arm_idx % 2 == 1 ) {
				// Flip the left arms from origin
				flipper = { -1.f, 1.f };
			}
			vec2 pos = (arm_part_idx == 0 ? first_joint_pos[arm_pair_idx] : middle_joint_pos) * parent_size;
			float angle = 0.f;
			if (arm_part_idx == 0) {
				// Angle of the first joint
				float base_angle = M_PI / 6.f;
				float arm_separation = 2.f;
				float delta_angle = -M_PI / 6.f * ((float)arm_pair_idx / (arms_count / 2)) * arm_separation;
				angle = base_angle + delta_angle;
			}
			else {
				// Angle of the middle joints
				angle = -M_PI / 3;
			}

			// Set arm part size
			float length_to_thickness_ratio = 1.f;
			length_to_thickness_ratio += (1.f - (float)arm_pair_idx / (arms_count / 2.f));
			length_to_thickness_ratio += ((float)arm_part_idx / arm_parts_count) * 1.5f;
			length_to_thickness_ratio /= 1.5f;
			float boss_size_factor = length(bossSize) / 800.f;
			vec2 size = BACTERIOPHAGE_ARM_TEXTURE_SIZE * vec2(length_to_thickness_ratio, 2.f / length_to_thickness_ratio) * boss_size_factor;

			articulated.relative_transform_1.scale(flipper);
			articulated.relative_transform_1.translate(pos);
			articulated.relative_transform_1.rotate(angle);
			articulated.angle_offset = angle;
			articulated.relative_transform_2.scale(size);
			articulated.relative_transform_2.translate({ 0.47f, -0.3f }); // Setting the pivot point to the joint

			// The following transform will be adjusted in physics_system to follow its parent
			registry.transforms.emplace(entity);
			Motion& motion = registry.motions.emplace(entity);		// This motion is with respect to parent
			motion.max_angular_velocity = M_PI / 4.f;
			registry.enemies.insert(entity, { ENEMY_ID::BOSS_ARM });
			registry.healthValues.insert(entity, { static_cast<float>(registry.gameMode.components.back().enemy_health_map[ENEMY_ID::BOSS_ARM]) });

			// Add to render_request
			registry.renderRequests.insert(
				entity,
				{ TEXTURE_ASSET_ID::BACTERIOPHAGE_ARM,
				  EFFECT_ASSET_ID::TEXTURED,
				  GEOMETRY_BUFFER_ID::SPRITE,
				  RENDER_ORDER::ENEMIES_FR });

			// Update parent to current part
			parent_entity = entity;
			parent_size = size;
		}
	}
}

Entity createSecondBoss(RenderSystem* renderer, vec2 pos, float health) {
	// Create boss components
	auto boss_entity = Entity();
	// Assuming boss is a type of enemy
	Dash& enemy_dash = registry.dashes.emplace(boss_entity);
	enemy_dash.delay_duration_ms = PLAYER_DASH_DELAY / registry.gameMode.components.back().FRIEND_BOSS_DIFFICULTY * 2.f;
	enemy_dash.active_duration_ms = 50.f;

	Enemy& new_enemy = registry.enemies.emplace(boss_entity);
	// Setting initial components values
	new_enemy.type = ENEMY_ID::FRIENDBOSS;
	registry.bosses.emplace(boss_entity);

	Transform& transform = registry.transforms.emplace(boss_entity);
	transform.position = pos;
	transform.angle_offset = IMMUNITY_TEXTURE_ANGLE;
	transform.angle = transform.angle_offset;
	transform.scale = FRIEND_BOSS_SIZE;

	Motion& motion = registry.motions.emplace(boss_entity);

	motion.max_velocity = 350.f;

	Health& enemyHealth = registry.healthValues.emplace(boss_entity);
	enemyHealth.health = fmin(health, registry.gameMode.components.back().enemy_health_map[ENEMY_ID::FRIENDBOSS]);
	enemyHealth.maxHealth = registry.gameMode.components.back().enemy_health_map[ENEMY_ID::FRIENDBOSS];

	Gun& weapon = registry.guns.emplace(boss_entity);
	weapon.damage = 15.f;
	weapon.bullet_color = {1.f, 0.8f, 0.8f, 1.f};
	weapon.bullet_speed = 800.f * registry.gameMode.components.back().FRIEND_BOSS_DIFFICULTY;
	weapon.bullet_size = { 20.f, 20.f };
	weapon.angle_offset = IMMUNITY_TEXTURE_ANGLE;
	weapon.attack_delay = PLAYER_ATTACK_DELAY / registry.gameMode.components.back().FRIEND_BOSS_DIFFICULTY * 1.2f;


	// Add to render_request
	registry.renderRequests.insert(
		boss_entity,
		{ TEXTURE_ASSET_ID::FRIEND,
		  EFFECT_ASSET_ID::TEXTURED,
		  GEOMETRY_BUFFER_ID::SPRITE,
		  RENDER_ORDER::BOSS });

	std::cout << "Creating second boss at position: " << pos.x << ", " << pos.y << std::endl;

	return boss_entity;
}

Entity createBossClone(vec2 pos, float health) {
	// Create boss components
	auto entity = Entity();
	// Assuming boss is a type of enemy
	registry.collidePlayers.emplace(entity);

	Enemy& new_enemy = registry.enemies.emplace(entity);
	// Setting initial components values
	new_enemy.type = ENEMY_ID::FRIENDBOSSCLONE;

	Transform& transform = registry.transforms.emplace(entity);
	transform.position = pos;
	transform.angle_offset = IMMUNITY_TEXTURE_ANGLE;
	transform.angle = transform.angle_offset;
	transform.scale = FRIEND_BOSS_SIZE;

	Motion& motion = registry.motions.emplace(entity);

	motion.max_velocity = 300.f; // TODO: Dummy boss for now, change this later

	Health& enemyHealth = registry.healthValues.emplace(entity);
	enemyHealth.health = registry.gameMode.components.back().enemy_health_map[ENEMY_ID::FRIENDBOSSCLONE];


	// Add to render_request
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::FRIEND,
		  EFFECT_ASSET_ID::TEXTURED,
		  GEOMETRY_BUFFER_ID::SPRITE,
		  RENDER_ORDER::BOSS });
	return entity;
}


Entity createRedEnemy(vec2 pos, float health) {
	
	// Create enemy components
	auto entity = Entity();
	Enemy& new_enemy = registry.enemies.emplace(entity);
	new_enemy.type = ENEMY_ID::RED;
	registry.collidePlayers.emplace(entity);

	Transform& transform = registry.transforms.emplace(entity);
	transform.position = pos;
	transform.angle_offset = M_PI / 2;
	transform.angle = transform.angle_offset;
	transform.scale = RED_ENEMY_SIZE;

	Motion& motion = registry.motions.emplace(entity);
	motion.max_velocity = 400;
	Health& enemyHealth = registry.healthValues.emplace(entity);
	enemyHealth.health = registry.gameMode.components.back().enemy_health_map[ENEMY_ID::RED];


	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::RED_ENEMY,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_ORDER::ENEMIES_BK });

	return entity;
}

Entity createGreenEnemy(vec2 pos, float health) {
	
	// Create enemy components
	auto entity = Entity();
	Enemy& new_enemy = registry.enemies.emplace(entity);
	new_enemy.type = ENEMY_ID::GREEN;
	registry.collidePlayers.emplace(entity);

	Transform& transform = registry.transforms.emplace(entity);
	transform.position = pos;
	transform.scale = GREEN_ENEMY_SIZE;
	transform.angle_offset = 3 * M_PI / 4;
	transform.angle = transform.angle_offset;

	Motion& motion = registry.motions.emplace(entity);
	motion.max_velocity = 200;
	Health& enemyHealth = registry.healthValues.emplace(entity);
	enemyHealth.health = registry.gameMode.components.back().enemy_health_map[ENEMY_ID::GREEN];;

	// Add to render_request
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::GREEN_ENEMY_MOVING,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITESHEET_GREEN_ENEMY_MOVING,
			RENDER_ORDER::ENEMIES_BK });

	Animation& animation = registry.animations.emplace(entity);
	animation.update_period_ms *= 2;
	animation.total_frame = (int)ANIMATION_FRAME_COUNT::GREEN_ENEMY_MOVING;
	animation.pause_animation = true;
	return entity;
}


Entity createYellowEnemy(vec2 pos, float health) {
	// Create enemy components
	auto entity = Entity();
	Enemy& new_enemy = registry.enemies.emplace(entity);
	new_enemy.type = ENEMY_ID::YELLOW;
	registry.collidePlayers.emplace(entity);

	Gun& weapon = registry.guns.emplace(entity);
	weapon.attack_delay = 900.f;
	weapon.bullet_speed = 400.f;
	weapon.bullet_size = { 25.f, 25.f };
	weapon.bullet_color = { 0.718f, 1.f, 0.f, 1.f };

	Transform& transform = registry.transforms.emplace(entity);
	transform.position = pos;
	transform.scale = YELLOW_ENEMY_SIZE;

	Motion& motion = registry.motions.emplace(entity);
	motion.max_velocity = 0.0f;
	motion.max_angular_velocity = M_PI;

	Health& enemyHealth = registry.healthValues.emplace(entity);
	enemyHealth.health = registry.gameMode.components.back().enemy_health_map[ENEMY_ID::YELLOW];;

	// Add tp render_request
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::YELLOW_ENEMY,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_ORDER::ENEMIES_BK });
	return entity;
}

Entity createChest(vec2 pos, REGION_GOAL_ID ability) {
    auto entity = Entity();
	registry.collidePlayers.emplace(entity);

    // Set up the transform for the chest
    Transform& transform = registry.transforms.emplace(entity);
    transform.position = pos;
	transform.scale = CHEST_SIZE;

	// Motion component only needed for collision check, set all to 0
	Motion& motion = registry.motions.emplace(entity);
	motion.velocity = { 0.f, 0.f };
	motion.max_velocity = 50.f;
	motion.acceleration_unit = 0.f;
	motion.deceleration_unit = 0.f;
	motion.allow_accel = false;

    // Create the chest component
    Chest& chest = registry.chests.emplace(entity);
    chest.ability = ability;
    chest.position = pos;

    // Add render requests for the chest
    registry.renderRequests.insert(
        entity,
        { TEXTURE_ASSET_ID::CHEST,
          EFFECT_ASSET_ID::TEXTURED,
          GEOMETRY_BUFFER_ID::SPRITE,
          RENDER_ORDER::OBJECTS }
    );

	std::cout << "Creating chest (Entity ID: " << entity << ") at position: " << chest.position.x << ", " << chest.position.y << " with ability: " << static_cast<int>(chest.ability) << std::endl;

    return entity;
}

Entity createCure(vec2 pos) {
	auto entity = Entity();
	registry.collidePlayers.emplace(entity);

	Transform& transform = registry.transforms.emplace(entity);
    transform.position = pos;
	transform.scale = CURE_SIZE;

	// Motion component only needed for collision check, set all to 0
	Motion& motion = registry.motions.emplace(entity);
	motion.velocity = { 0.f, 0.f };
	motion.max_velocity = 50.f;
	motion.acceleration_unit = 0.f;
	motion.deceleration_unit = 0.f;
	motion.allow_accel = false;

	Cure& cure = registry.cure.emplace(entity);

	registry.renderRequests.insert(
        entity,
        { TEXTURE_ASSET_ID::CURE,
          EFFECT_ASSET_ID::TEXTURED,
          GEOMETRY_BUFFER_ID::SPRITE,
          RENDER_ORDER::OBJECTS }
    );

	std::cout << "Cure dropped at position: " << pos.x << ", " << pos.y << std::endl;

	return entity;
}

void createRandomRegions(size_t num_regions, std::default_random_engine& rng) {
	assert(region_theme_count >= num_regions);
	assert(region_goal_count >= num_regions);

	std::vector<REGION_THEME_ID> unused_themes;
	for (uint i = 0; i < region_theme_count; i++) {
		unused_themes.push_back(static_cast<REGION_THEME_ID>(i));
	}
	std::shuffle(unused_themes.begin(), unused_themes.end(), rng);
	std::vector<REGION_GOAL_ID> unused_goals;
	for (uint i = 0; i < region_goal_count; i++) {
		unused_goals.push_back(static_cast<REGION_GOAL_ID>(i));
	}
	std::shuffle(unused_goals.begin(), unused_goals.end(), rng);
	std::vector<BOSS_ID> unused_bosses;
	for (uint i = 0; i < boss_type_count; i++) {
		unused_bosses.push_back(static_cast<BOSS_ID>(i));
	}
	//std::shuffle(unused_bosses.begin(), unused_bosses.end(), rng);

	float angle = 0.f;

	for (int i = 0; i < num_regions; i++) {
		auto entity = Entity();
		Region& region = registry.regions.emplace(entity);

		// Set region unique theme
		region.theme = unused_themes.back();
		unused_themes.pop_back();
		// Set region unique goal
		region.goal = unused_goals.back();
		unused_goals.pop_back();

		// Set region boss (if needs a boss)
		if (region.goal == REGION_GOAL_ID::CURE) {
			region.boss = BOSS_ID::BACTERIOPHAGE;
		} else if (region.goal == REGION_GOAL_ID::CANCER_CELL) {
			region.boss = BOSS_ID::FRIEND;
		} else {
			region.boss = BOSS_ID::BOSS_COUNT;
		}

		// Calculate interest point for the region
		float interest_distance = MAP_RADIUS * 0.6;
		float center_angle = angle + (M_PI * 2 / num_regions) / 2; // Center of the angle span for the region
		vec2 interest_point;
		interest_point.x = interest_distance * cos(center_angle);
		interest_point.y = interest_distance * sin(center_angle);
		region.interest_point = vec2(round(interest_point.x), round(interest_point.y));
		std::cout << "Region " << i+1 << ": Interest Point (X, Y) = (" << region.interest_point.x << ", " << region.interest_point.y << ")\n";


		// Set region regular enemy (non-unique)
		// Did not implement this feature
		//std::uniform_int_distribution<int> int_dist(0, enemy_type_count - 1);
		//ENEMY_ID enemy = static_cast<ENEMY_ID>(int_dist(rng));
		//region.enemy = enemy;

		// Add region to renderRequests and transforms
		registry.renderRequests.insert(
			entity,
			{ region_texture_map[region.theme],
				EFFECT_ASSET_ID::REGION,
				GEOMETRY_BUFFER_ID::REGION_TRIANGLE,
				RENDER_ORDER::BACKGROUND }
		);
		registry.transforms.insert(
			entity,
			{ { 0.f, 0.f }, { MAP_RADIUS * 1.5f, MAP_RADIUS * 1.5f }, angle, false }
		);

		// Update angle
		angle += (M_PI * 2 / num_regions);
	}
}

void createRandomCysts(std::default_random_engine& rng) {
	const float ANGLE = (M_PI * 2 / NUM_REGIONS);
	const int TOTAL_CYSTS = 132; 
	const float MAX_CLOSENESS = SCREEN_RADIUS / 2;

	const float LOWER_RADIUS = 0.17f * MAP_RADIUS;
	const float UPPER_RADIUS = 0.98f * MAP_RADIUS;
	const float MEAN_POINT = 0.28f * MAP_RADIUS; // actual mean is approx = max_radius - mean_point

	std::exponential_distribution<float> radius_distribution(1.f / MEAN_POINT);
	std::uniform_real_distribution<float> angle_distribution(0.f, ANGLE);

	std::vector<vec2> positions;
	// generate cysts
	for (int i = 0; i < registry.regions.components.size(); i++) {
		for (int j = 0; j < TOTAL_CYSTS / registry.regions.components.size(); j++) {
			// generate radius in the correct bounds
			float radius;
			do {
				radius = MAP_RADIUS - radius_distribution(rng);
			} while (radius > UPPER_RADIUS || radius < LOWER_RADIUS);

			// generate angle
			float angle = angle_distribution(rng) + ANGLE * i;

			// calculate position and check closeness
			vec2 pos = vec2(cos(angle), sin(angle)) * radius;
			bool tooClose = false;
			for (auto p : positions) {
				if (distance(pos, p) < MAX_CLOSENESS) {
					tooClose = true;
					break;
				}
			}
			if (tooClose) {
				--j;
				continue;
			}

			// finally create cyst
			createCyst(pos);
			positions.push_back(pos);
		}
	}
}

void createCyst(vec2 pos, float health) {
	auto cyst_entity = Entity();
	registry.cysts.emplace(cyst_entity);
	registry.healthValues.insert(cyst_entity, {health});
	registry.collidePlayers.emplace(cyst_entity);

	// Motion component only needed for collision check, set all to 0
	Motion& motion = registry.motions.emplace(cyst_entity);
	motion.velocity = { 0.f, 0.f };
	motion.max_velocity = 0.f;
	motion.acceleration_unit = 0.f;
	motion.deceleration_unit = 0.f;
	motion.allow_accel = false;

	Transform& transform = registry.transforms.emplace(cyst_entity);
	transform.position = pos;
	transform.scale = CYST_TEXTURE_SIZE * 4.f;

	Animation& animation = registry.animations.emplace(cyst_entity);
	animation.total_frame = (int)ANIMATION_FRAME_COUNT::CYST_SHINE;
	animation.update_period_ms = 80;
	animation.loop_interval = 1500.f;

	registry.renderRequests.insert(
		cyst_entity,
		{ TEXTURE_ASSET_ID::CYST_SHINE,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITESHEET_CYST_SHINE,
			RENDER_ORDER::ENEMIES_BK });
}

Entity createLine(vec2 position, float angle, vec2 scale) {
	Entity entity = Entity();

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
			EFFECT_ASSET_ID::COLOURED,
			GEOMETRY_BUFFER_ID::DEBUG_LINE,
			RENDER_ORDER::UI });

	registry.colors.insert(entity, { 1.f,0.f,0.f,1.f });

	Transform& transform = registry.transforms.emplace(entity);
	transform.angle = angle;
	transform.position = position;
	transform.scale = scale;
	transform.is_screen_coord = false;


	registry.debugComponents.emplace(entity);
	return entity;
}

Entity createHoldGuide(vec2 position, vec2 scale) {
	Entity entity = Entity();

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::HOLD_SPACE,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_ORDER::UI });

	registry.colors.insert(entity, { 1.f,1.f,1.f,0.f });

	Transform& transform = registry.transforms.emplace(entity);
	transform.position = position;
	transform.scale = scale;
	transform.is_screen_coord = true;

	return entity;
}

std::tuple<Entity, Entity> createHealthbar(vec2 position, vec2 scale) {
	Entity bar = Entity();
	Entity frame = Entity();

	Healthbar& healthbar = registry.healthbar.emplace(bar);
	healthbar.full_health_color = { 0.f,1.f,0.f,1.f };

	// Add the components to the renderRequest in order
	registry.renderRequests.insert(
		bar,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
			EFFECT_ASSET_ID::COLOURED,
			GEOMETRY_BUFFER_ID::STATUSBAR_RECTANGLE,
			RENDER_ORDER::UI });
	registry.renderRequests.insert(
		frame,
		{ TEXTURE_ASSET_ID::HEALTHBAR_FRAME,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_ORDER::UI_FR });

	// Create transform for bar and frame
	vec2 frameScale = HEALTHBAR_TEXTURE_SIZE * scale;
	registry.transforms.insert(frame, { position, frameScale, 0.f, true });
	vec2 barPosition = position - vec2(frameScale.x * 0.25, 0.f);
	vec2 barScale = frameScale * vec2(0.72, 0.18);
	registry.transforms.insert(bar, { barPosition, barScale, 0.f, true });

	// Create color for bar
	registry.colors.insert(bar, healthbar.full_health_color);

	return std::make_tuple(bar, frame);
}

std::tuple<Entity, Entity> createBossHealthbar(vec2 position, vec2 scale) {
	Entity bar = Entity();
	Entity frame = Entity();

	Healthbar& healthbar = registry.healthbar.emplace(bar);
	healthbar.full_health_color = { 1.f,0.f,0.f,0.f };

	// Add the components to the renderRequest in order
	registry.renderRequests.insert(
		bar,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
			EFFECT_ASSET_ID::COLOURED,
			GEOMETRY_BUFFER_ID::STATUSBAR_RECTANGLE,
			RENDER_ORDER::UI });
	registry.renderRequests.insert(
		frame,
		{ TEXTURE_ASSET_ID::HEALTHBAR_FRAME_BOSS,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_ORDER::UI });

	// Create transform for bar and frame
	vec2 frameScale = BOSS_HEALTHBAR_TEXTURE_SIZE * scale;
	registry.transforms.insert(frame, { position, frameScale, 0.f, true });
	vec2 barPosition = position - vec2(frameScale.x * 0.26, 0.f);
	vec2 barScale = frameScale * vec2(0.75, 0.18);
	registry.transforms.insert(bar, { barPosition, barScale, 0.f, true });

	// Create color for bar and hide boss health bar at the beginning
	registry.colors.insert(bar, healthbar.full_health_color);
	registry.colors.insert(frame, { 1.f,1.f,1.f,0.f });

	return std::make_tuple(bar, frame);
}

void createBullet(Entity shooter, vec2 scale, vec4 color) {
	if (registry.lotsOfBullets.has(shooter)) {
		// not efficient but haven't noticed any performance issues
		createBulletHelper(shooter, scale, color, -M_PI / 5);
		createBulletHelper(shooter, scale, color, -2 * M_PI / 5);
		createBulletHelper(shooter, scale, color, -3 * M_PI / 5);
		createBulletHelper(shooter, scale, color, -4 * M_PI / 5);
		createBulletHelper(shooter, scale, color, M_PI / 5);
		createBulletHelper(shooter, scale, color, 2 * M_PI / 5);
		createBulletHelper(shooter, scale, color, 3 * M_PI / 5);
		createBulletHelper(shooter, scale, color, 4 * M_PI / 5);
		createBulletHelper(shooter, scale, color, 0.f);
		createBulletHelper(shooter, scale, color, M_PI);
	} else if (registry.tripleBullets.has(shooter)) {
		createBulletHelper(shooter, scale*0.8f, color, -0.21f);
		createBulletHelper(shooter, scale, color, 0.f);
		createBulletHelper(shooter, scale*0.8f, color, 0.21f);
	}
	else {
		createBulletHelper(shooter, scale, color, 0.f);
	}
}

// Can be used for either player or enemy
void createBulletHelper(Entity shooter, vec2 scale, vec4 color, float angle_offset) {
	assert(registry.transforms.has(shooter));
	// Create bullet's components
	auto bullet_entity = Entity();

	Transform& bullet_transform = registry.transforms.emplace(bullet_entity);
	Motion& bullet_motion = registry.motions.emplace(bullet_entity);
	registry.colors.insert(bullet_entity, color);

	// Set initial position and velocity
	Transform& shooter_transform = registry.transforms.get(shooter);
	Gun& weapon = registry.guns.get(shooter);

	Transformation t;
	t.translate(shooter_transform.position);
	t.rotate(shooter_transform.angle + shooter_transform.angle_offset);
	t.translate(weapon.offset);
	//t.rotate(weapon.angle_offset);

	bullet_transform.position = t.mat[2];
	bullet_transform.scale = scale;
	bullet_transform.angle = 0.f;

	Motion shooter_motion = registry.motions.get(shooter);

	if (registry.enemies.has(shooter)) {
		bullet_motion.velocity = { cos(shooter_transform.angle - weapon.angle_offset) * weapon.bullet_speed, 
			sin(shooter_transform.angle - weapon.angle_offset) * weapon.bullet_speed};
	}
	else {
		vec2 bullet_direction = normalize(vec2(cos(shooter_transform.angle + angle_offset - weapon.angle_offset), sin(shooter_transform.angle + angle_offset - weapon.angle_offset)));

		float projection = dot(shooter_motion.velocity, bullet_direction);
		// add players sideways velocity
		vec2 shooter_velocity = shooter_motion.velocity - bullet_direction * projection;
		// add half of players forward/backward velocity
		//shooter_velocity += bullet_direction * projection * 0.5f;

		// scale to length of max_velocity (ie when dashing)
		if (length(shooter_velocity) > shooter_motion.max_velocity) {
			float h = hypot(shooter_velocity.x, shooter_velocity.y);
			shooter_velocity.x = shooter_motion.max_velocity * shooter_velocity.x / h;
			shooter_velocity.y = shooter_motion.max_velocity * shooter_velocity.y / h;
		}

		bullet_motion.velocity = bullet_direction * weapon.bullet_speed + shooter_velocity;
	}

	// Set projectile damage based on weapon
	registry.projectiles.insert(bullet_entity, { weapon.damage });

	if (registry.collideEnemies.has(shooter)) {
		registry.collideEnemies.emplace(bullet_entity);
	}
	if (registry.collidePlayers.has(shooter)) {
		registry.collidePlayers.emplace(bullet_entity);
	}

	// Add bullet to render request
	registry.renderRequests.insert(
		bullet_entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT, // TEXTURE_COUNT indicates that no txture is needed
			EFFECT_ASSET_ID::COLOURED,
			GEOMETRY_BUFFER_ID::BULLET,
			RENDER_ORDER::OBJECTS });
}

Entity createCamera(vec2 pos) {
	Entity camera = Entity();
	registry.camera.insert(camera, { pos });
	return camera;
}

Entity createCrosshair() {
	Entity entity = Entity();

	registry.transforms.insert(entity, { vec2(0.f,0.f), vec2(20.f,20.f), 0.f, true});

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::CROSSHAIR,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_ORDER::UI });
	registry.colors.insert(entity, { 1.f,1.f,1.f,1.f });

	return entity;
}

void createWaypoint(REGION_GOAL_ID goal, Entity target) {
	assert(registry.transforms.has(target));
	
	Entity entity = Entity();
	Waypoint& waypoint = registry.waypoints.emplace(entity);
	waypoint.target = target;
	waypoint.goal = goal;
	registry.transforms.insert(entity, {vec2(0.f, 0.f), vec2(30.f, 30.f), 0.f, true });

	TEXTURE_ASSET_ID texture;
	if (goal == REGION_GOAL_ID::CURE || goal == REGION_GOAL_ID::CANCER_CELL) {
		waypoint.icon_scale = SKULL_TEXTURE_SIZE;
		texture = TEXTURE_ASSET_ID::ICON_SKULL;
	} else {
		waypoint.icon_scale = QUESTION_TEXTURE_SIZE;
		texture = TEXTURE_ASSET_ID::ICON_QUESTION;
	}

	registry.renderRequests.insert(
		entity,
		{ texture,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_ORDER::UI });

	registry.colors.insert(entity, { 1.f,1.f,1.f,0.f });
}

Entity createDeathScreen(int scenario) {
	Entity entity = Entity();

	Transform& transform = registry.transforms.emplace(entity);
	transform.scale = { DIALOG_TEXTURE_SIZE.x, DIALOG_TEXTURE_SIZE.y };
	transform.is_screen_coord = true;

	registry.renderRequests.insert(
		entity,
		{ scenario == 1 ? TEXTURE_ASSET_ID::DEATH_SCREEN_1 : TEXTURE_ASSET_ID::DEATH_SCREEN_2,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_ORDER::UI });

	registry.colors.insert(entity, { 1.f,1.f,1.f,0.f });

	return entity;
}

Entity createCredits() {
	// Background texture
	Entity black_screen = Entity();

	registry.renderRequests.insert(
		black_screen,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
			EFFECT_ASSET_ID::COLOURED,
			GEOMETRY_BUFFER_ID::STATUSBAR_RECTANGLE,
			RENDER_ORDER::CREDITS_BG });
	Transform& bg_transform = registry.transforms.emplace(black_screen);
	bg_transform.is_screen_coord = true;
	bg_transform.position.x -= CONTENT_WIDTH_PX / 2;
	bg_transform.scale = { CONTENT_WIDTH_PX, CONTENT_HEIGHT_PX };
	registry.colors.insert(black_screen, { 0.f,0.f,0.f,1.f, });

	// Credits texture
	Entity entity = Entity();

	Credits& credits = registry.credits.emplace(entity);
	Transform& transform = registry.transforms.emplace(entity);
	transform.position.y = -DIALOG_TEXTURE_SIZE.y;
	transform.scale = { DIALOG_TEXTURE_SIZE.x, DIALOG_TEXTURE_SIZE.y };
	transform.is_screen_coord = true;

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::CREDITS,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_ORDER::CREDITS });

	// Title texture
	Entity title = Entity();

	Transform& title_transform = registry.transforms.emplace(title);
	title_transform.position.y = -DIALOG_TEXTURE_SIZE.y;
	title_transform.scale = START_TITLE_TEXTURE_SIZE * 0.4f;
	title_transform.is_screen_coord = true;
	registry.colors.insert(title, { 1.f,1.f,1.f,1.f, });

	registry.renderRequests.insert(
		title,
		{ TEXTURE_ASSET_ID::START_MENU_TITLE,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_ORDER::CREDITS });

	// Set credit fields
	credits.background = black_screen;
	credits.title = title;

	return entity;
}

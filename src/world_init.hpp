#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"

#include <random>


// These are hard coded to the dimensions of the entity texture
const vec2 IMMUNITY_TEXTURE_SIZE = { 30.f, 30.f };
const vec2 RED_ENEMY_TEXTURE_SIZE = { 48.f, 40.f };
const vec2 GREEN_ENEMY_TEXTURE_SIZE = { 148.f, 140.f };
const vec2 YELLOW_ENEMY_TEXTURE_SIZE = { 63.f, 60.f };
const vec2 HEALTHBAR_TEXTURE_SIZE = { 530.f, 80.f };
const vec2 BOSS_HEALTHBAR_TEXTURE_SIZE = {1000.f, 126.f};
const vec2 CHEST_TEXTURE_SIZE = { 24.f, 76.f };
const vec2 BACTERIOPHAGE_TEXTURE_SIZE = { 180.f, 689.f };
const vec2 BACTERIOPHAGE_ARM_TEXTURE_SIZE = { 174.f, 33.f };
const vec2 DIALOG_TEXTURE_SIZE = { 1920.f, 1080.f };
const vec2 CYST_TEXTURE_SIZE = { 23.f, 22.f };
const vec2 GUN_TEXTURE_SIZE = { 128.f, 24.f };
const vec2 DASHING_TEXTURE_SIZE = { 1024.f, 128.f };
const vec2 SWORD_SIZE = { 200.f, 200.f };
const vec2 FRIEND_TEXTURE_SIZE = { 30.f, 30.f };
const vec2 MENU_BUTTON_TEXTURE_SIZE = {882.f, 262.f};
const vec2 START_TITLE_TEXTURE_SIZE = {2928.f, 824.f};
const vec2 PAUSE_TITLE_TEXTURE_SIZE = {1193.f, 143.f};
const vec2 QUESTION_TEXTURE_SIZE = {30.f, 54.f};
const vec2 SKULL_TEXTURE_SIZE = {45.f, 42.f};
const vec2 CURE_TEXTURE_SIZE = {21.f, 39.f};
const vec2 HOLD_GUIDE_TEXTURE_SIZE = {530.f, 110.f};

const float IMMUNITY_TEXTURE_ANGLE = -M_PI / 4;

const vec2 STATUSBAR_SCALE = { 1.f, 1.f };
const vec2 PLAYER_SIZE = IMMUNITY_TEXTURE_SIZE * 3.f;
const vec2 RED_ENEMY_SIZE = RED_ENEMY_TEXTURE_SIZE * 2.f;
const vec2 GREEN_ENEMY_SIZE = GREEN_ENEMY_TEXTURE_SIZE * 1.f;
const vec2 YELLOW_ENEMY_SIZE = YELLOW_ENEMY_TEXTURE_SIZE * 1.5f;
const vec2 CHEST_SIZE = CHEST_TEXTURE_SIZE * 3.f;
const vec2 CURE_SIZE = CURE_TEXTURE_SIZE * 2.0f;
const vec2 BACTERIOPHAGE_BOSS_SIZE = BACTERIOPHAGE_TEXTURE_SIZE * 0.7f;
const vec2 FRIEND_BOSS_SIZE = FRIEND_TEXTURE_SIZE * 3.f;

/*************************[ characters ]*************************/
// the player
Entity createPlayer(vec2 pos);
Entity createDashing(Entity dasher);
Entity createSword(RenderSystem* renderer, Entity& playerEntity);
Entity createCure(vec2 pos);
Entity createGun(Entity holder);
// enemies
Entity createRedEnemy(vec2 pos, float health = 40.0f);
Entity createGreenEnemy(vec2 pos, float health = 150.0f);
Entity createYellowEnemy(vec2 pos, float health = 50.0f);
Entity createBossClone(vec2 pos, float health = 10.0f);
Entity createBoss(RenderSystem* renderer, vec2 pos, float health = INFINITY);
void createBossArms(RenderSystem* renderer, Entity bossEntity, vec2 bossSize);
Entity createSecondBoss(RenderSystem* renderer, vec2 pos, float health = INFINITY);

/*************************[ environment ]*************************/
// the random regions
void createRandomRegions(size_t num_regions, std::default_random_engine& rng);
void createRandomCysts(std::default_random_engine& rng);
void createCyst(vec2 pos, float health = 50.0f);
Entity createChest(vec2 pos, REGION_GOAL_ID ability);
void createBullet(Entity shooter, vec2 scale, vec4 color);
void createBulletHelper(Entity shooter, vec2 scale, vec4 color, float angle_offset);

/*************************[ UI ]*************************/
// a red line for debugging purposes
Entity createLine(vec2 position, float angle, vec2 scale);
// create healthbar and its frame. Returns healthbar entity
std::tuple<Entity, Entity> createHealthbar(vec2 position, vec2 scale);
std::tuple<Entity, Entity> createBossHealthbar(vec2 position, vec2 scale);
Entity createCrosshair();
void createWaypoint(REGION_GOAL_ID goal, Entity target);
Entity createDeathScreen(int scenario);
Entity createCredits();
Entity createHoldGuide(vec2 position, vec2 scale);

/*************************[ other ]*************************/
Entity createCamera(vec2 pos);

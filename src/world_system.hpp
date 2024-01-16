#pragma once

// internal
#include "common.hpp"
#include "render_system.hpp"
#include "./sub_systems/dialog_system.hpp"
#include "./sub_systems/effects_system.hpp"
#include "./sub_systems/menu_system.hpp"

// stlib
#include <vector>
#include <random>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

// Container for all our entities and game logic. Individual rendering / update is
// deferred to the relative update() methods
class WorldSystem
{
public:
	WorldSystem();

	// Creates a window
	GLFWwindow* create_window();

	void on_controller_joy(int joy, int event);

	// starts the game
	void init(RenderSystem* renderer);

	// Releases all associated resources
	~WorldSystem();

	// Steps the game ahead by ms milliseconds
	bool step(float elapsed_ms);


	void update_camera(float elapsed_ms);

	// Check for collisions
	void resolve_collisions();

	// Should the game be over ?
	bool is_over()const;



	void startEntityDeath(Entity entity);

private:
	// Input callback functions
	void on_key(int key, int, int action, int mod);
	void on_mouse_move(vec2 pos);

	void menu_controller(float elapsed_ms_since_last_update);

	void step_controller();

	// User input handlers
	void control_movement(float elapsed_ms);
	void control_direction();
	void control_action();

	void player_shoot();
	void player_sword_slash();
	void player_dash();

	// restart level
	void restart_game(bool hard_reset = false);
	void reset_game_state(bool hard_reset = false);
	template <typename Component>
	void clearSpecificEntities(ComponentContainer<Component>& componentRegistry);
	void populate_player_abilities();
	void populate_region_goals();

	// OpenGL window handle
	GLFWwindow* window;

	// Game state
	RenderSystem* renderer;
	EffectsSystem* effects_system;
	float current_speed;
	Entity player;
	Entity game_entity;
	GAME_STATE state;
	BUTTON_SELECT button_select= BUTTON_SELECT::NONE;
	GameMode easyMode = {GAME_MODE_ID::EASY_MODE, 
		max_green_easyMode, max_red_easyMode, max_yellow_easyMode,
	enemy_health_map_easy,
	FRIEND_BOSS_DIFFICULTY_easy};
	GameMode regularMode = { GAME_MODE_ID::REGULAR_MODE, 
		max_green_regularMode, max_red_regularMode, max_yellow_regularMode,
	enemy_health_map_regular,
	FRIEND_BOSS_DIFFICULTY};

	// UI references
	Entity healthbar;
	Entity healthbar_frame;
	Entity boss_healthbar;
	Entity boss_healthbar_frame;
	Entity cursor;
	Entity death_screen;
	Entity hold_to_collect;

	// sound references and handler
	std::unordered_map<std::string, Mix_Music*> backgroundMusic;
	std::unordered_map<std::string, Mix_Chunk*> soundChunks;
	std::unordered_map<std::string, int> chunkToChannel;
	void handle_shooting_sound_effect();
	bool isShootingSoundQueued;

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1

	DialogSystem* dialog_system = nullptr;
	MenuSystem* menu_system = nullptr;
	
	// Maintain enemy counts
	std::unordered_map<ENEMY_ID, int> enemyCounts;
	std::unordered_map<ENEMY_ID, int> maxEnemies;

	bool allow_accel;
	void shakeCamera(float amount, float ms, float shake_scale = 2.f, vec2 direction = vec2(1.f, 0.f));
	void squish(Entity entity, float squish_amount);


	// Step different sub-systems
	void step_deathTimer(float elapsed_ms);
	void step_health();
	void step_healthbar(float elapsed_ms, Entity healthbar, Entity target, float max_bar_len, bool update_color);
	void step_invincibility(float elapsed_ms);
	void step_attack(float elapsed_ms);
	void step_dash(float elapsed_ms);
	void step_enemySpawn(float elapsed_ms);
	void step_timer_with_callback(float elapsed_ms);
	void step_waypoints();
	void step_menu();
	void detect_bossfight();
	void step_healthBoost(float elapsed_ms);
	void step_roll_credits(float elapsed_ms);
	void step_chests();

	void spawnEnemiesNearInterestPoint(vec2 player_position);
	void spawnEnemyOfType(ENEMY_ID type, vec2 player_position, vec2 player_velocity);
	int getMaxEnemiesForType(ENEMY_ID type);

	void remove_garbage();
	void remove_entity(Entity entity);
	void create_debug_lines();

	void load_game();
	void loadRegions(const json& regionsData);
	void save_game();
	json serializeGameState();

	Entity& getAttachment(Entity character, ATTACHMENT_ID type);
	bool hasPlayerAbility(PLAYER_ABILITY_ID abilityId);
	void show_hold_to_collect();

	void setGameMode(GAME_MODE_ID id);
};

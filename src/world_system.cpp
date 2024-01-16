// Header
#include "world_system.hpp"
#include "world_init.hpp"
#include "sub_systems/dialog_system.hpp"
#include "sub_systems/effects_system.hpp"
#include "sub_systems/menu_system.hpp"
#include "sub_systems/menu_system.hpp"

// stlib
#include <cassert>
#include <sstream>

#include "physics_system.hpp"
#include <unordered_map>
#include <iostream>

std::unordered_map < int, int > keys_pressed;
float spaceBarPressDuration = 0.0f;
std::unordered_map < int, float > axes_state;
const unsigned char* controller_buttons = nullptr;
vec2 mouse;
const float ENEMY_SPAWN_PADDING = 50.f; // Padding to ensure off-screen spawn
float enemy_spawn_cooldown = 5000.f;
float individual_spawn_interval = 1000.f;
std::vector<ENEMY_ID> enemyTypes = { ENEMY_ID::RED, ENEMY_ID::GREEN, ENEMY_ID::YELLOW }; // Add more types as needed
float menu_timer = 0.f;

bool controller_mode = FALSE; //if most recent input is controller set to 1, if mouse/keyboard set to 0


// Create the world
WorldSystem::WorldSystem() {
	// Seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
	allow_accel = true;
	enemyCounts[ENEMY_ID::RED] = 0;
	enemyCounts[ENEMY_ID::GREEN] = 0;
	enemyCounts[ENEMY_ID::YELLOW] = 0;
	maxEnemies[ENEMY_ID::RED] = 0;
	maxEnemies[ENEMY_ID::GREEN] = 0;
	maxEnemies[ENEMY_ID::YELLOW] = 0;
	state = GAME_STATE::START_MENU;
}

WorldSystem::~WorldSystem() {
	// Destroy music components
	for (const auto& pair : soundChunks) {
		if (pair.second != nullptr) {
			Mix_FreeChunk(pair.second);
		}
	}
	for (const auto& pair : backgroundMusic) {
		if (pair.second != nullptr) {
			Mix_FreeMusic(pair.second);
		}
	}
	Mix_CloseAudio();

	// Destroy all created components
	registry.clear_all_components();

	// Close the window
	glfwDestroyWindow(window);

	delete this->effects_system;

	delete this->menu_system;

	delete this->dialog_system;

	//the value of the this->renderer is passed in as an argument, no delete needed ;
}

// Debugging
namespace {
	void glfw_err_cb(int error, const char* desc) {
		fprintf(stderr, "%d: %s", error, desc);
	}
}

// World initialization
// Note, this has a lot of OpenGL specific things, could be moved to the renderer
GLFWwindow* WorldSystem::create_window() {
	///////////////////////////////////////
	// Initialize GLFW
	glfwSetErrorCallback(glfw_err_cb);
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW");
		return nullptr;
	}

	//-------------------------------------------------------------------------
	// If you are on Linux or Windows, you can change these 2 numbers to 4 and 3 and
	// enable the glDebugMessageCallback to have OpenGL catch your mistakes for you.
	// GLFW / OGL Initialization
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_RESIZABLE, 0);

	if (USE_FULLSCREEN) {
		// Fullscreen window
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		window = glfwCreateWindow(CONTENT_WIDTH_PX, CONTENT_HEIGHT_PX, "Cytotoxic Cataclysm", monitor, nullptr);

		// When fullscreen we attempt to set the video mode to use a refresh rate of TARGET_REFRESH_RATE
		glfwSetWindowMonitor(window, monitor, 0, 0, CONTENT_WIDTH_PX, CONTENT_HEIGHT_PX, TARGET_REFRESH_RATE);
		int actual_refresh_rate = glfwGetVideoMode(monitor)->refreshRate;
		if (actual_refresh_rate != TARGET_REFRESH_RATE) {
			printf("Warning: target refresh rate, %d, does not match actual refresh rate, %d\n", TARGET_REFRESH_RATE, actual_refresh_rate);
		}
	}
	else {
		// Windowed mode
		window = glfwCreateWindow(CONTENT_WIDTH_PX, CONTENT_HEIGHT_PX, "Cytotoxic Cataclysm", nullptr, nullptr);
	}

	// Create the main window (for rendering, keyboard, and mouse input)
	if (window == nullptr) {
		fprintf(stderr, "Failed to glfwCreateWindow");
		return nullptr;
	}

	// Setting callbacks to member functions (that's why the redirect is needed)
	// Input is handled using GLFW, for more info see
	// http://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowUserPointer(window, this);
	auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_move({ _0, _1 }); };
	auto mouse_button_callback = [](GLFWwindow* wnd, int button, int action, int mods) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_key(button, 0, action, mods); };
	auto controller_joystick_callback = [](int joy, int event) { ((WorldSystem*)glfwGetWindowUserPointer(glfwGetCurrentContext()))->on_controller_joy(joy, event); };

	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetJoystickCallback(controller_joystick_callback);



	//////////////////////////////////////
	// Loading music and sounds with SDL
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "Failed to initialize SDL Audio");
		return nullptr;
	}
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
		fprintf(stderr, "Failed to open audio device");
		return nullptr;
	}

	// TODO: For Voxel Revolution.wav must credit as below:
	/*
		"Voxel Revolution" Kevin MacLeod (incompetech.com)
		Licensed under Creative Commons: By Attribution 4.0 License
		http://creativecommons.org/licenses/by/4.0/
	*/

	backgroundMusic["main"] = Mix_LoadMUS(audio_path("music/Voxel Revolution.wav").c_str());
	backgroundMusic["boss"] = Mix_LoadMUS(audio_path("music/battleThemeA.wav").c_str());
	backgroundMusic["menu"] = Mix_LoadMUS(audio_path("music/Ending.wav").c_str());
	soundChunks["player_hit"] = Mix_LoadWAV(audio_path("sound/sfx_sounds_damage1.wav").c_str());
	soundChunks["player_dash"] = Mix_LoadWAV(audio_path("sound/sfx_sound_nagger2.wav").c_str());
	soundChunks["player_death"] = Mix_LoadWAV(audio_path("sound/sfx_sounds_falling3.wav").c_str());
	soundChunks["player_shoot_1"] = Mix_LoadWAV(audio_path("sound/sfx_wpn_laser8.wav").c_str());
	soundChunks["enemy_hit"] = Mix_LoadWAV(audio_path("sound/sfx_movement_footsteps5.wav").c_str());
	soundChunks["enemy_death"] = Mix_LoadWAV(audio_path("sound/sfx_deathscream_android8.wav").c_str());
	soundChunks["cyst_pos"] = Mix_LoadWAV(audio_path("sound/sfx_sounds_fanfare3.wav").c_str());
	soundChunks["cyst_neg"] = Mix_LoadWAV(audio_path("sound/sfx_deathscream_robot1.wav").c_str());
	soundChunks["cyst_empty"] = Mix_LoadWAV(audio_path("sound/sfx_sounds_interaction7.wav").c_str());
	soundChunks["no_ammo"] = Mix_LoadWAV(audio_path("sound/sfx_wpn_noammo3.wav").c_str());
	soundChunks["sword_unlock"] = Mix_LoadWAV(audio_path("sound/sword.wav").c_str());
	soundChunks["dash_unlock"] = Mix_LoadWAV(audio_path("sound/dash.wav").c_str());
	soundChunks["health_unlock"] = Mix_LoadWAV(audio_path("sound/health.wav").c_str());
	soundChunks["bullet_unlock"] = Mix_LoadWAV(audio_path("sound/reload.wav").c_str());

	// Check for failures
	for (const auto& pair : backgroundMusic) {
		if (pair.second == nullptr) {
			fprintf(stderr, "Failed to load music make sure the data directory is present");
			return nullptr;
		}
	}
	for (const auto& pair : soundChunks) {
		if (pair.second == nullptr) {
			fprintf(stderr, "Failed to load sound chunk \'%s\' make sure the data directory is present", pair.first.c_str());
			return nullptr;
		}
	}

	// Assign channels
	Mix_AllocateChannels(14);
	chunkToChannel["bullet_unlock"] = 13;
	chunkToChannel["health_unlock"] = 12;
	chunkToChannel["dash_unlock"] = 11;
	chunkToChannel["sword_unlock"] = 10;
	chunkToChannel["player_hit"] = 9;
	chunkToChannel["player_dash"] = 8;
	chunkToChannel["player_death"] = 7;
	chunkToChannel["player_shoot_1"] = 6;
	chunkToChannel["enemy_hit"] = 5;
	chunkToChannel["enemy_death"] = 4;

	// Adjust music and CHUNK volume in range [0,128]
	Mix_VolumeMusic(45);
	Mix_VolumeChunk(soundChunks["player_shoot_1"], 60);
	Mix_VolumeChunk(soundChunks["player_dash"], 45);
	Mix_VolumeChunk(soundChunks["enemy_hit"], 45);
	Mix_VolumeChunk(soundChunks["sword_unlock"], 60);
	Mix_VolumeChunk(soundChunks["dash_unlock"], 60);
	Mix_VolumeChunk(soundChunks["health_unlock"], 60);
	Mix_VolumeChunk(soundChunks["bullet_unlock"], 60);

	return window;
}

void WorldSystem::on_controller_joy(int joy, int event) {
	if (event == GLFW_CONNECTED) {
		printf("Controller %d connected\n", joy);
	}
	else if (event == GLFW_DISCONNECTED) {
		controller_mode = false;
		printf("Controller %d disconnected\n", joy);
	}

}

void WorldSystem::init(RenderSystem* renderer_arg) {
	this->renderer = renderer_arg;
	this->effects_system = new EffectsSystem(player, rng, soundChunks, *this);
	this->menu_system = new MenuSystem(mouse);

	// Create world entities that don't reset
	cursor = createCrosshair();

	createCamera({ 0.f, 0.f });
	hold_to_collect = createHoldGuide({ 0.f, CONTENT_HEIGHT_PX * -0.42 }, HOLD_GUIDE_TEXTURE_SIZE * 0.5f);
	dialog_system = new DialogSystem(keys_pressed, mouse, controller_buttons);
	registry.gameMode.insert(Entity(), regularMode);

	// Set all states to default
	restart_game(true);
	// Playing background music indefinitely
	Mix_FadeInMusic(backgroundMusic["menu"], -1, 5000);
	fprintf(stderr, "Loaded music\n");

	button_select = BUTTON_SELECT::NONE;
	state = GAME_STATE::START_MENU;
}

bool isKeyPressed() {
	for (auto keyVal = keys_pressed.begin(); keyVal != keys_pressed.end(); ++keyVal) {
		if (keyVal->second) return true;
	}
	if (controller_buttons != nullptr) {
		for (uint i = 0; i < 19; i++) {
			if (controller_buttons[i]) return true;
		}
	}
	return false;
}

void WorldSystem::step_deathTimer(float elapsed_ms) {
	for (uint i = 0; i < registry.deathTimers.components.size(); i++) {
		DeathTimer& timer = registry.deathTimers.components[i];
		Entity entity = registry.deathTimers.entities[i];
		// progress timer
		timer.timer_ms -= elapsed_ms;

		if (entity == player) {

			if (timer.timer_ms <= 0) {
				// Player is dead -> restart
				if (isKeyPressed()) {
					registry.deathTimers.remove(entity);
					registry.remove_all_components_of(death_screen);
					restart_game();
				}
				return;
			}
			else {
				// Player is dying -> darken death screen texture
				registry.colors.get(death_screen).a = min(1.f, 1 - timer.timer_ms / DEATH_EFFECT_DURATION);
			}
		}
		else if (timer.timer_ms <= 0) {
			// Remove other entities (enemies, cysts, etc.)

			if (registry.enemies.has(entity)) {
				// Enemy is dead -> remove
				Enemy enemyComponent = registry.enemies.get(entity);
				ENEMY_ID type = enemyComponent.type;

				if (type == ENEMY_ID::BOSS) {
					vec2 enemyDeathSpot = registry.transforms.get(entity).position;
					Entity cure = createCure(enemyDeathSpot);
					createWaypoint(REGION_GOAL_ID::CURE, cure);
					registry.game.get(game_entity).isFirstBossDefeated = true;
					registry.colors.get(boss_healthbar).a = 0.f;
					registry.colors.get(boss_healthbar_frame).a = 0.f;
					vec2 player_pos = registry.transforms.get(player).position;
					dialog_system->add_camera_movement(player_pos, enemyDeathSpot, 1000.f);
					dialog_system->add_dialog(TEXTURE_ASSET_ID::POST_BOSS_DIALOG);
					dialog_system->add_camera_movement(enemyDeathSpot, player_pos, 1000.f);
					Mix_FadeInMusic(backgroundMusic["main"], -1, 2000);
				}
				else if (type == ENEMY_ID::FRIENDBOSS) {
					registry.game.get(game_entity).isSecondBossDefeated = true;
					registry.colors.get(boss_healthbar).a = 0.f;
					registry.colors.get(boss_healthbar_frame).a = 0.f;

					// clear region
					for (Region& region : registry.regions.components) {
						if (region.goal == REGION_GOAL_ID::CANCER_CELL) {
							region.is_cleared = true;
							break;
						}
					}

					dialog_system->add_dialog(TEXTURE_ASSET_ID::POST_FRIEND_BOSS_DIALOG, 1000.f);
					state = GAME_STATE::CREDITS;
				}

				enemyCounts[type]--;
			}
			remove_entity(entity);
		}
	}
}

void WorldSystem::remove_entity(Entity entity) {
	// Remove all attachments of the entity recursively before removing itself
	// Iterating backwards and updating i since multiple entities can be removed
	for (int i = (int)registry.attachments.components.size() - 1;
		i >= 0 && registry.attachments.components.size() > 0;
		i = min(i - 1, (int)registry.attachments.components.size() - 1)
		) {
		if (registry.attachments.components[i].parent == entity) {
			remove_entity(registry.attachments.entities[i]);
		}
	}
	registry.remove_all_components_of(entity);
}

void WorldSystem::step_roll_credits(float elapsed_ms) {
	const float start_position = -DIALOG_TEXTURE_SIZE.y;
	const float distance = CONTENT_HEIGHT_PX + DIALOG_TEXTURE_SIZE.y;
	const float fade_time = 2500.f;
	const float title_start_position = -DIALOG_TEXTURE_SIZE.y * 1.4;

	for (auto entity : registry.credits.entities) {
		// rolling the credits
		Credits& credits = registry.credits.get(entity);
		credits.timer += elapsed_ms;
		registry.transforms.get(entity).position.y = start_position + credits.timer / credits.total_time * distance;
		float title_y = min(0.f, title_start_position + credits.timer / credits.total_time * distance);
		registry.transforms.get(credits.title).position.y = title_y;

		// start fading background and title
		if (credits.timer > credits.total_time - fade_time) {
			registry.colors.get(credits.background).a = 1.f - (credits.timer - (credits.total_time - fade_time)) / fade_time;
			registry.colors.get(credits.title).a = 1.f - (credits.timer - (credits.total_time - fade_time)) / fade_time;
		}

		// end condition
		if (credits.timer > credits.total_time) {
			state = GAME_STATE::RUNNING;
			registry.remove_all_components_of(credits.background);
			registry.remove_all_components_of(credits.title);
			registry.remove_all_components_of(entity);
			return;
		}
		break;
	}
}


void WorldSystem::startEntityDeath(Entity entity) {
	if (registry.deathTimers.has(entity)) return;
	DeathTimer& dt = registry.deathTimers.emplace(entity);
	int buffer = 20; // to fix timing between animation and death

	if (entity == player) {
		dt.timer_ms = DEATH_EFFECT_DURATION;

		Mix_PlayChannel(chunkToChannel["player_death"], soundChunks["player_death"], 0);

		RenderSystem::animationSys_switchAnimation(player,
			ANIMATION_FRAME_COUNT::IMMUNITY_DYING,
			static_cast<int>(ceil((DEATH_EFFECT_DURATION + buffer) / static_cast<int>(ANIMATION_FRAME_COUNT::IMMUNITY_DYING))));

		int scenario = 1;
		for (auto boss : registry.bosses.entities) {
			if (length(vec2(renderer->createViewMatrix() * vec3(registry.transforms.get(boss).position, 1.f))) < SCREEN_RADIUS) {
				scenario = 2;
				break;
			}
		}
		death_screen = createDeathScreen(scenario);
		registry.motions.get(player).max_velocity *= 0.f;
		registry.collideEnemies.remove(player);
		registry.guns.get(player).attack_timer = 9999.f;
		if (registry.melees.has(player)) {
			registry.melees.get(player).attack_timer = 9999.f;
		}

	}
	else if (registry.enemies.has(entity)) {
		// Immobilize enemy
		assert(registry.motions.has(entity));
		registry.motions.remove(entity);

		if (registry.guns.has(entity)) {
			registry.guns.remove(entity);
		}
		if (registry.dashes.has(entity)) {
			registry.dashes.remove(entity);
		}

		dt.timer_ms = DEATH_EFFECT_DURATION_ENEMY;

		Mix_PlayChannel(chunkToChannel["enemy_death"], soundChunks["enemy_death"], 0);

		Enemy& enemy = registry.enemies.get(entity);
		if (enemy.type == ENEMY_ID::GREEN) {
			RenderSystem::animationSys_switchAnimation(entity,
				ANIMATION_FRAME_COUNT::GREEN_ENEMY_DYING,
				static_cast<int>(ceil((DEATH_EFFECT_DURATION_ENEMY + buffer) / static_cast<int>(ANIMATION_FRAME_COUNT::GREEN_ENEMY_DYING))));
		}
		if (enemy.type == ENEMY_ID::FRIENDBOSS) {
			for (Entity enemyentity : registry.enemies.entities) {
				Enemy& enemyclone = registry.enemies.get(enemyentity);
				if (enemyclone.type == ENEMY_ID::FRIENDBOSSCLONE) {
					startEntityDeath(enemyentity);
				}
			}
		}
		if (enemy.type == ENEMY_ID::BOSS_ARM) {
			dt.timer_ms = INFINITY;
			registry.renderRequests.get(entity).order = RENDER_ORDER::OBJECTS;
		}
	}
	else if (registry.cysts.has(entity)) {
		Game& game_state = registry.game.get(game_entity);
		dt.timer_ms = 50.f; // TODO set based on animation length
		// If this is the first time breaking a cyst, show tutorial
		if (!game_state.isCystTutorialDisplayed) {
			dialog_system->add_dialog(TEXTURE_ASSET_ID::TUTORIAL_CYSTS, 1500.f);
			game_state.isCystTutorialDisplayed = true;
		}
		effects_system->apply_random_effect();
		// TODO death animation
	}
}

void WorldSystem::step_health() {
	for (uint i = 0; i < registry.healthValues.components.size(); i++) {
		Health& health = registry.healthValues.components[i];
		Entity entity = registry.healthValues.entities[i];

		if (health.health <= 0 && !registry.deathTimers.has(entity)) {
			startEntityDeath(entity);
		}
	}
}

void WorldSystem::step_healthBoost(float elapsed_ms) {
	Health& health = registry.healthValues.get(player);
	if (health.healthIncrement > 0) {
		// Increment health
		health.health = min(health.health + health.healthIncrement * (elapsed_ms / 1000.0f), health.maxHealth);
	}
}

void WorldSystem::step_healthbar(float elapsed_ms, Entity healthbar, Entity target, float max_bar_len, bool update_color) {
	Healthbar& bar = registry.healthbar.get(healthbar);
	Health& health = registry.healthValues.get(target);
	float current_health = health.health;

	if (bar.previous_health != current_health) {
		bar.timer_ms -= elapsed_ms;
		if (bar.timer_ms <= 0) {
			bar.previous_health = current_health;
			bar.timer_ms = HEALTH_BAR_UPDATE_TIME_SLAP;
		}

		// Interpolate to get value of displayed healthbar
		float healthbar_scale = 0.f;
		float normalized_time = bar.timer_ms / HEALTH_BAR_UPDATE_TIME_SLAP;
		float disp_health_value = bar.previous_health * normalized_time + current_health * (1.f - normalized_time);
		healthbar_scale = max(0.f, disp_health_value / health.maxHealth);

		// Update the scale of the healthbar
		assert(registry.transforms.has(healthbar));
		Transform& transform = registry.transforms.get(healthbar);
		transform.scale.x = max_bar_len * STATUSBAR_SCALE.x * healthbar_scale;

		// Update the color of the healthbar
		if (update_color) {
			assert(registry.colors.has(healthbar));
			vec4& color = registry.colors.get(healthbar);
			color.r = 1.f - healthbar_scale;
			color.g = fmin(healthbar_scale, bar.full_health_color.g);
			color.b = fmin(healthbar_scale, bar.full_health_color.b);
		}
	}
}

void WorldSystem::step_invincibility(float elapsed_ms) {
	if (registry.invincibility.has(player)) {
		assert(registry.colors.has(player));

		// Reduce timer and change set player flashing
		float& timer = registry.invincibility.get(player).timer_ms;
		timer -= elapsed_ms;
		float flashAlpha;
		if (timer <= 0) {
			registry.invincibility.remove(player);
			flashAlpha = 1.f;
		}
		else {
			// Flash once every 200ms
			flashAlpha = mod(ceil(timer / 100.f), 2.f);
		}

		// set alpha values
		vec4& player_color = registry.colors.get(player);
		player_color.a = flashAlpha;

		// set alpha value for all attachments of  the player
		for (uint i = 0; i < registry.attachments.size(); i++) {
			Attachment& att = registry.attachments.components[i];
			if (att.parent == player) {
				Entity att_entity = registry.attachments.entities[i];
				if (registry.colors.has(att_entity)) {
					registry.colors.get(att_entity).a = flashAlpha;
				}
			}
		}
	}
	// Step Eneny and cyst invincibility to sword
	for (uint i = 0; i < registry.enemies.components.size(); i++) {
		Enemy& enemy_attrib = registry.enemies.components[i];
		if (enemy_attrib.sword_attack_cd > 0.f) {
			enemy_attrib.sword_attack_cd -= elapsed_ms;
		}
	}
	for (uint i = 0; i < registry.cysts.components.size(); i++) {
		Cyst& cyst_attrib = registry.cysts.components[i];
		if (cyst_attrib.sword_attack_cd > 0.f) {
			cyst_attrib.sword_attack_cd -= elapsed_ms;
		}
	}
}

void WorldSystem::step_attack(float elapsed_ms) {
	if (registry.guns.has(player)) {
		Gun& player_gun = registry.guns.get(player);
		player_gun.attack_timer = max(player_gun.attack_timer - elapsed_ms, 0.f);
	}
	if (registry.melees.has(player)) {
		Melee& player_sword = registry.melees.get(player);
		// More efficient with if-else
		float old_timer = player_sword.attack_timer;
		if (old_timer > 0.f) {
			player_sword.attack_timer = max(old_timer - elapsed_ms, 0.f);
			if (player_sword.animation_timer > 0.f) {
				player_sword.animation_timer = max(player_sword.animation_timer - elapsed_ms, 0.f);
				if (player_sword.animation_timer <= 0) {
					// Stop sword and set position to the original
					Entity sword_entity = player_sword.melee_entity;
					assert(registry.attachments.has(sword_entity) && registry.motions.has(sword_entity));
					Attachment& att = registry.attachments.get(sword_entity);
					Motion& sword_motion = registry.motions.get(sword_entity);
					sword_motion.angular_velocity = 0;
					att.moved_angle = att.angle_offset;
				}
			}
		}


	}
}

void WorldSystem::step_dash(float elapsed_ms) {
	if (registry.dashes.has(player)) {
		Dash& playerDash = registry.dashes.get(player);
		Entity dashingAnimationEntity = getAttachment(player, ATTACHMENT_ID::DASHING);
		playerDash.active_timer_ms = max(playerDash.active_timer_ms - elapsed_ms, 0.f);
		playerDash.delay_timer_ms = max(playerDash.delay_timer_ms - elapsed_ms, 0.f);

		if (playerDash.active_timer_ms <= 0.f) {
			//make dashing animation invisible
			vec4& color = registry.colors.get(dashingAnimationEntity);
			color = no_color;
		}
		else {
			vec4& color = registry.colors.get(dashingAnimationEntity);
			color = dashing_default_color;
		}
	}
}

bool out_of_boundary_check(vec2 entityScale, vec2 entityPos) {
	if (length(entityPos) + length(entityScale) / 2.f > MAP_RADIUS) {
		return true;
	}
	return false;
}

void WorldSystem::spawnEnemyOfType(ENEMY_ID type, vec2 player_position, vec2 player_velocity) {
	std::uniform_real_distribution<float> angle_randomness(-M_PI / 4, M_PI / 4);  // 45 degrees randomness

	// Determine the angle for spawning based on player's velocity
	float player_movement_angle = atan2(player_velocity.y, player_velocity.x);
	float random_offset = angle_randomness(rng);
	float angle = length(player_velocity) > 0.001f
		? player_movement_angle + random_offset
		: uniform_dist(rng) * 2 * M_PI;

	// Calculate spawn position around the player, but off-screen
	vec2 offset = { cos(angle) * (SCREEN_RADIUS + ENEMY_SPAWN_PADDING), sin(angle) * (SCREEN_RADIUS + ENEMY_SPAWN_PADDING) };
	vec2 spawn_position = player_position + offset;

	switch (type) {
	case ENEMY_ID::RED:
		if (out_of_boundary_check(RED_ENEMY_SIZE, spawn_position)) break;
		//std::cout << "Spawning Red Enemy at: (" << spawn_position.x << ", " << spawn_position.y << ")" << std::endl;
		createRedEnemy(spawn_position);
		enemyCounts[ENEMY_ID::RED]++;
		break;
	case ENEMY_ID::GREEN:
		if (out_of_boundary_check(GREEN_ENEMY_SIZE, spawn_position)) break;
		//std::cout << "Spawning Green Enemy at: (" << spawn_position.x << ", " << spawn_position.y << ")" << std::endl;
		createGreenEnemy(spawn_position);
		enemyCounts[ENEMY_ID::GREEN]++;
		break;
	case ENEMY_ID::YELLOW:
		if (out_of_boundary_check(YELLOW_ENEMY_SIZE, spawn_position)) break;
		createYellowEnemy(spawn_position);
		enemyCounts[ENEMY_ID::YELLOW]++;
	default:
		break;
	}
}

void WorldSystem::spawnEnemiesNearInterestPoint(vec2 player_position) {
	vec2 player_velocity = registry.motions.get(player).velocity;

	std::uniform_int_distribution<int> type_dist(0, static_cast<int>(enemyTypes.size()) - 1);
	ENEMY_ID randomType = enemyTypes[type_dist(rng)];

	if (enemyCounts[randomType] < getMaxEnemiesForType(randomType)) {
		spawnEnemyOfType(randomType, player_position, player_velocity);
	}

}

int WorldSystem::getMaxEnemiesForType(ENEMY_ID type) {
	auto it = maxEnemies.find(type);
	if (it != maxEnemies.end()) {
		return it->second;
	}
	return 0;
}

float calculateSpawnProbability(vec2 player_position) {
	float nearest_distance = FLT_MAX;
	for (const Region& region : registry.regions.components) {
		float distance = length(player_position - region.interest_point);
		nearest_distance = std::min(nearest_distance, distance);
	}
	// Convert distance to a probability (closer to interest point = higher probability)
	return 1.0f - (nearest_distance / MAP_RADIUS);
}

void WorldSystem::step_enemySpawn(float elapsed_ms) {
	// Decrease the spawn cooldown timer
	enemy_spawn_cooldown -= elapsed_ms;

	if (enemy_spawn_cooldown <= 0.f) {
		vec2 player_position = registry.transforms.get(player).position;
		float spawn_probability = calculateSpawnProbability(player_position);

		std::uniform_real_distribution<float> spawn_chance(0.f, 1.f);
		if (spawn_chance(rng) < spawn_probability) {
			spawnEnemiesNearInterestPoint(player_position);
		}

		// Reset the cooldown timer for the next individual enemy spawn
		enemy_spawn_cooldown = individual_spawn_interval;
	}
}

// steps timers and invoke associated callback upon expiration
void WorldSystem::step_timer_with_callback(float elapsed_ms) {
	std::vector<Entity> garbage; // another garbage collector
	for (uint i = 0; i < registry.timedEvents.components.size(); i++) {
		TimedEvent& timedEvent = registry.timedEvents.components[i];
		timedEvent.timer_ms -= elapsed_ms;
		if (timedEvent.timer_ms <= 0.f) {
			timedEvent.callback();
			garbage.push_back(registry.timedEvents.entities[i]);
		}
	}
	for (Entity e : garbage) {
		registry.remove_all_components_of(e);
	}
}

void WorldSystem::step_waypoints() {
	static const float padding = 23.f;
	static const float top = -CONTENT_HEIGHT_PX / 2 + padding + 4.f;
	static const float bot = CONTENT_HEIGHT_PX / 2 - padding - 4.f;
	static const float left = -CONTENT_WIDTH_PX / 2 + padding;
	static const float right = CONTENT_WIDTH_PX / 2 - padding;

	// ip is interest point
	auto findIntersectionPoint = [](vec2 ip) {
		vec2 p; // p is position to render icon
		float t; // t is parameter for parametric equation
		p.y = top; // p intersects top of screen
		p.x = (p.y) / (ip.y) * (ip.x);
		t = p.x / ip.x;
		if (t > 0.f && t < 1.f && p.x > left && p.x < right) {
			return p;
		}
		p.y = bot; // p intersects bottom of screen
		p.x = (p.y) / (ip.y) * (ip.x);
		t = p.x / ip.x;
		if (t > 0.f && t < 1.f && p.x > left && p.x < right) {
			return p;
		}
		p.x = left; // p intersects left of screen
		p.y = (p.x) / (ip.x) * (ip.y);
		t = p.y / ip.y;
		if (t > 0.f && t < 1.f && p.y > top && p.y < bot) {
			return p;
		}
		p.x = right; // p intersects right of screen
		p.y = (p.x) / (ip.x) * (ip.y);
		t = p.y / ip.y;
		if (t > 0.f && t < 1.f && p.y > top && p.y < bot) {
			return p;
		}
		return ip;
		};

	// hide all if boss onscreen
	for (auto boss : registry.bosses.entities) {
		if (length(vec2(renderer->createViewMatrix() * vec3(registry.transforms.get(boss).position, 1.f))) < SCREEN_RADIUS) {
			for (Entity wp : registry.waypoints.entities) {
				registry.colors.get(wp).a = 0.f;
			}
			return;
		}
	}


	float minDistance = MAP_RADIUS;
	Entity closestWP;
	Game& game = registry.game.get(game_entity);
	for (int i = (int)registry.waypoints.size() - 1; i >= 0; i--) {
		Entity wp = registry.waypoints.entities[i];
		Waypoint& waypoint = registry.waypoints.components[i];

		// Update waypoint targets to their live position
		// waypoint.interest_point will be used in AI system
		if (registry.transforms.has(waypoint.target)) {
			waypoint.interest_point = registry.transforms.get(waypoint.target).position;
		}
		else {
			// if target doesn't exist, remove waypoint
			registry.remove_all_components_of(wp);
			continue;
		}

		vec2 interest_point_screen_coord = vec2(renderer->createViewMatrix() * vec3(waypoint.interest_point, 1.0));
		vec2 result = findIntersectionPoint(interest_point_screen_coord);

		registry.transforms.get(wp).position = result;

		float distance = length(interest_point_screen_coord);
		if (distance < minDistance) {
			minDistance = distance;
			closestWP = wp;
		}

		const float minDistanceThreshold = 1500.f;
		const float maxDistanceThreshold = 7500.f;
		float scale = clamp(distance, minDistanceThreshold, maxDistanceThreshold);
		scale = 1.f - ((scale - minDistanceThreshold) / (maxDistanceThreshold - minDistanceThreshold));
		registry.transforms.get(wp).scale = waypoint.icon_scale * 0.35f * scale + 27.f;

		if (scale > 0.4) {
			registry.colors.get(wp).a = scale + 0.1f;
		}
		else {
			registry.colors.get(wp).a = 0.00001f; // since 0.f will indicate on-screen
		}

		// hide if interest point is on-screen
		if (result == interest_point_screen_coord) {
			registry.colors.get(wp).a = 0.f;
		}
	}
	// set closest icon alpha to 1 if not on screen and player not in center
	if (registry.waypoints.has(closestWP) && length(registry.transforms.get(player).position) > SPAWN_REGION_RADIUS / 2.f) {
		float& alpha = registry.colors.get(closestWP).a;
		alpha = alpha == 0.f ? 0.f : 1.f;
	}
}

void WorldSystem::shakeCamera(float amount, float ms, float shake_scale, vec2 direction) {
	Camera& camera = registry.camera.components[0];
	camera.shake += amount;
	camera.shake_scale = shake_scale;
	camera.shake_direction = direction;

	TimedEvent& timer = registry.timedEvents.emplace(Entity());
	timer.timer_ms = ms;
	timer.callback = [amount]() {
		registry.camera.components[0].shake -= amount;
		};
}

void WorldSystem::update_camera(float elapsed_ms) {
	static float total_time = 0.f;
	total_time += elapsed_ms;
	Camera& camera = registry.camera.components[0];
	camera.position = registry.transforms.get(player).position + camera.shake_direction * camera.shake * sin(total_time / camera.shake_scale);
}

void WorldSystem::remove_garbage() {
	Transform player_transform = registry.transforms.get(player);
	vec2 player_pos = player_transform.position;
	// Remove off-screen bullets to improve performance
	for (Entity bullet : registry.projectiles.entities) {
		Transform transform = registry.transforms.get(bullet);
		if (length(transform.position - player_pos) > SCREEN_RADIUS + 200.f) {
			registry.remove_all_components_of(bullet);
		}
	}

	for (int i = 0; i < registry.enemies.components.size(); i++) {
		Enemy enemyComponent = registry.enemies.components[i];
		if (enemyComponent.type == ENEMY_ID::GREEN || enemyComponent.type == ENEMY_ID::RED || enemyComponent.type == ENEMY_ID::YELLOW) {
			Entity enemyEntity = registry.enemies.entities[i];
			Transform enemyTransform = registry.transforms.get(enemyEntity);
			//consider player_pos as the center pointer of the player's view screen. despawn enemy if it is out of the view
			if (length(enemyTransform.position - player_pos) > SCREEN_RADIUS + 200.f) {
				registry.remove_all_components_of(enemyEntity);
				enemyCounts[enemyComponent.type]--;
				printf("remove enemy with id = %d at position <%f, %f>\n", static_cast<int>(enemyEntity), enemyTransform.position.x, enemyTransform.position.y);
			}
		}
	}
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {
	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
		registry.remove_all_components_of(registry.debugComponents.entities.back());

	ScreenState& screen = registry.screenStates.components[0];

	if (state == GAME_STATE::ENDED) {
		return false;
	}
	else if (state == GAME_STATE::START_MENU || state == GAME_STATE::PAUSE_MENU) {
		step_menu();
		menu_controller(elapsed_ms_since_last_update);
		// No screen darkening effect or crosshair in menus
		screen.screen_darken_factor = 0.f;
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		registry.colors.get(cursor).a = 0.f;
	}
	else if (dialog_system->has_pending()) {
		// Step dialog
		dialog_system->step(elapsed_ms_since_last_update);
		step_invincibility(elapsed_ms_since_last_update);
		step_controller();
		registry.colors.get(cursor).a = 0.f;
	}
	else if (state == GAME_STATE::CREDITS) {
		if (registry.credits.size() == 0) {
			createCredits();
		}
		step_roll_credits(elapsed_ms_since_last_update);
	}
	else if (state == GAME_STATE::RUNNING) {
		/*************************[ gameplay ]*************************/
		// Code below this line will happen only during gameplay

		if (DEBUG_MODE) create_debug_lines();

		// Input updates
		control_direction();
		control_action();
		step_controller();
		// Block velocity update for one step after collision to
		// avoid going out of border / going through enemy
		if (allow_accel) {
			control_movement(elapsed_ms_since_last_update);
		}
		else {
			allow_accel = true;
		}

		// Input feedbacks
		handle_shooting_sound_effect();
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

		// Game logic
		step_deathTimer(elapsed_ms_since_last_update);
		step_health();
		step_healthbar(elapsed_ms_since_last_update, healthbar, player, HEALTHBAR_TEXTURE_SIZE.x * 0.72f, true);
		if (registry.bosses.size() > 0 && registry.bosses.components[0].activated) {
			registry.colors.get(boss_healthbar).a = 1.f;
			registry.colors.get(boss_healthbar_frame).a = 1.f;
			Entity current_boss = registry.bosses.entities[0];
			step_healthbar(elapsed_ms_since_last_update, boss_healthbar, current_boss, BOSS_HEALTHBAR_TEXTURE_SIZE.x * 0.75f, false);
		}
		step_healthBoost(elapsed_ms_since_last_update);
		step_invincibility(elapsed_ms_since_last_update);
		step_attack(elapsed_ms_since_last_update);
		step_dash(elapsed_ms_since_last_update);
		if (!(registry.bosses.size() > 0 && registry.bosses.components[0].activated)) {
			step_enemySpawn(elapsed_ms_since_last_update);
		}
		step_timer_with_callback(elapsed_ms_since_last_update);
		step_waypoints();
		detect_bossfight();
		step_chests();

		// Cleanup
		remove_garbage();
	}
	else {
		assert(false);	// Invalid game state
	}

	return state == GAME_STATE::RUNNING && !dialog_system->has_pending();
}

// Draw debug line for all mesh edges in debug mode
void WorldSystem::create_debug_lines() {
	for (uint i = 0; i < registry.meshPtrs.components.size(); i++) {
		Entity entity = registry.meshPtrs.entities[i];
		if (registry.transforms.has(entity)) {
			const Transform transform = registry.transforms.get(entity);
			const Mesh* mesh = registry.meshPtrs.components[i];

			Transformation t_matrix;
			t_matrix.translate(transform.position);
			t_matrix.rotate(transform.angle);
			t_matrix.scale(transform.scale);

			std::vector<vec2> vertex_pos;
			for (TexturedVertex v : mesh->texture_vertices) {
				vec3 world_pos = t_matrix.mat * vec3(v.position.x, v.position.y, 1.f);
				vertex_pos.push_back(vec2(world_pos.x, world_pos.y));

			}
			// For each edge of triangle, create a debug line
			for (int v = 0; v < vertex_pos.size(); v++) {
				vec2 point_1 = vertex_pos[v];
				vec2 point_2 = vertex_pos[(v + 1) % vertex_pos.size()];
				vec2 line_pos = (point_1 + point_2) / 2.f;
				vec2 diff = point_2 - point_1;
				float line_len = length(diff);
				float line_angle = atan2f(diff.y, diff.x);
				createLine(line_pos, line_angle, { line_len, 2.f });
			}
		}
	}
}

// Create region goal objects (boss, chest, etc.) and waypoint to them
// REQUIRES: regions to be set up
void WorldSystem::populate_region_goals() {
	Game& game = registry.game.get(game_entity);
	for (Region& region : registry.regions.components) {
		if (region.is_cleared) continue;

		if (region.goal == REGION_GOAL_ID::CURE) {
			if (!game.isFirstBossDefeated) {
				Entity firstBoss = createBoss(renderer, region.interest_point);
				createWaypoint(region.goal, firstBoss);
			}
			else if (!game.isCureObtained) {
				Entity cure = createCure(region.interest_point);
				createWaypoint(region.goal, cure);
			}
		}
		else if (region.goal == REGION_GOAL_ID::CANCER_CELL) {
			if (game.isCureObtained && !game.isSecondBossDefeated) {
				Entity secondBoss = createSecondBoss(renderer, region.interest_point);
				createWaypoint(region.goal, secondBoss);
			}
		}
		else {
			Entity chest = createChest(region.interest_point, region.goal);
			createWaypoint(region.goal, chest);
		}
	}
}

void WorldSystem::populate_player_abilities() {
	for (PlayerAbility& ability_component : registry.playerAbilities.components) {
		PLAYER_ABILITY_ID ability = ability_component.id;
		switch (ability) {
		case PLAYER_ABILITY_ID::SWORD: {
			createSword(renderer, player);
			break;
		}
		case PLAYER_ABILITY_ID::BULLET_BOOST: {
			assert(registry.guns.has(player));
			Gun& gun_component = registry.guns.get(player);
			gun_component.attack_delay = PLAYER_ATTACK_DELAY / 2;
			Entity gun_entity = getAttachment(player, ATTACHMENT_ID::GUN);
			assert(registry.colors.has(gun_entity));
			vec4& gun_color = registry.colors.get(gun_entity);
			gun_color.g = 0.5f;
			gun_color.b = 0.5f;
			assert(registry.attachments.has(gun_entity));
			Attachment& att = registry.attachments.get(gun_entity);
			att.relative_transform_2.scale({ 1.5f, 1.5f });
			break;
		}
		case PLAYER_ABILITY_ID::HEALTH_BOOST: {
			assert(registry.healthValues.has(player));
			Health& health = registry.healthValues.get(player);
			assert(registry.renderRequests.has(healthbar_frame));
			registry.renderRequests.get(healthbar_frame).used_texture = TEXTURE_ASSET_ID::HEALTHBAR_FRAME_BOOST;
			assert(registry.healthbar.has(healthbar));
			registry.healthbar.get(healthbar).full_health_color = { 0.f, 1.f, 1.f, 1.f };
			health.healthMultiplier = 2.0f;
			health.maxHealth *= health.healthMultiplier;
			health.health = health.maxHealth;
			health.healthIncrement = 1.0f;
			break;
		}
		case PLAYER_ABILITY_ID::DASHING: {
			createDashing(player);
			break;
		}
		default:
			std::cerr << "Unknown ability loaded: " << static_cast<int>(ability) << std::endl;
			break;
		}
	}
}

// Reset the world state to its initial state
void WorldSystem::restart_game(bool hard_reset) {
	printf("\n=========================\n|\tRestarting\t|\n=========================\n");

	reset_game_state(hard_reset);

	/*************************[ setup new world ]*************************/

	if (hard_reset) {
		// Populate initial dialogs
		dialog_system->add_dialog(TEXTURE_ASSET_ID::DIALOG_INTRO1);
		dialog_system->add_dialog(TEXTURE_ASSET_ID::DIALOG_INTRO2);
		dialog_system->add_dialog(TEXTURE_ASSET_ID::DIALOG_INTRO3);
		if (controller_mode) {
			dialog_system->add_dialog(TEXTURE_ASSET_ID::TUTORIAL_CONTROLS_CONTROLLER);
		}
		else {
			dialog_system->add_dialog(TEXTURE_ASSET_ID::TUTORIAL_CONTROLS_KEYBOAD);
		}
		dialog_system->add_dialog(TEXTURE_ASSET_ID::TUTORIAL_WAYPOINTS);
		dialog_system->add_dialog(TEXTURE_ASSET_ID::TUTORIAL_GAME_START);
	}

	maxEnemies[ENEMY_ID::RED] = registry.gameMode.components.back().max_red;
	maxEnemies[ENEMY_ID::GREEN] = registry.gameMode.components.back().max_green;
	maxEnemies[ENEMY_ID::YELLOW] = registry.gameMode.components.back().max_yellow;

	// Create a new player
	player = createPlayer({ 0, 0 });
	effects_system->player = player;
	populate_player_abilities();

	// Populate regions
	if (DEBUG_MODE) {
		createBoss(renderer, { 100.f, 100.f });
		createSecondBoss(renderer, { -100.f, 100.f });
	}
	else {
		populate_region_goals();
	}

	// Hide boss health bar in case player dies during boss fight
	registry.colors.get(boss_healthbar).a = 0.f;
	registry.colors.get(boss_healthbar_frame).a = 0.f;

	createRandomCysts(rng);
	update_camera(0.f);
}

// Call this method each frame to update the space bar duration
void updateSpaceBarPressDuration() {
	if (keys_pressed[GLFW_KEY_SPACE] || (controller_buttons != nullptr && controller_buttons[2])) {
		spaceBarPressDuration += 1;
	}
	else {
		spaceBarPressDuration = 0.0f;
	}
}

bool isHoldingSpace(float time_ms) {
	if (spaceBarPressDuration >= time_ms) {
		return true;
	}
	return false;
}

void WorldSystem::squish(Entity entity, float squish_amount) {
	registry.transforms.get(entity).scale *= squish_amount;
	TimedEvent& timer = registry.timedEvents.emplace(Entity());
	timer.timer_ms = 30.f;
	timer.callback = [entity, squish_amount]() {
		if (registry.transforms.has(entity)) {
			registry.transforms.get(entity).scale /= squish_amount;
		}
		};
}

void WorldSystem::show_hold_to_collect() {
	assert(registry.renderRequests.has(hold_to_collect));
	assert(registry.colors.has(hold_to_collect));
	RenderRequest& rr = registry.renderRequests.get(hold_to_collect);
	if (controller_mode) {
		rr.used_texture = TEXTURE_ASSET_ID::HOLD_O;
	}
	else {
		rr.used_texture = TEXTURE_ASSET_ID::HOLD_SPACE;
	}
	registry.colors.get(hold_to_collect).a = 1.f;
}

// Compute collisions between entities
void WorldSystem::resolve_collisions() {
	// Loop over all collisions detected by the physics system
	std::list<Entity> garbage{};
	auto& collisionsRegistry = registry.collisions;
	bool show_hold_guide = false;
	for (uint i = 0; i < collisionsRegistry.components.size(); i++) {
		// The entity and its collider
		Entity entity = collisionsRegistry.entities[i];
		Collision collision = collisionsRegistry.components[i];
		Transform& transform = registry.transforms.get(entity);
		Motion& motion = registry.motions.get(entity);
		// When any moving object collides with the boundary, it gets bounced towards the 
		// reflected direction (similar to the physics model of reflection of light)
		if (collision.collision_type == COLLISION_TYPE::WITH_BOUNDARY) {
			vec2 normal_vec = -normalize(transform.position);
			// Only reflect when velocity is pointing out of the boundary to avoid being stuck
			if (dot(motion.velocity, normal_vec) < 0) {
				vec2 reflection = motion.velocity - 2 * dot(motion.velocity, normal_vec) * normal_vec;
				// Gradually lose some momentum each collision
				motion.velocity = 0.95f * reflection;
				allow_accel = false;
			}
		}
		else if (collision.collision_type == COLLISION_TYPE::PLAYER_WITH_REGION_BOUNDARY) {
			motion.velocity = 0.95f * collision.knockback_dir;
			allow_accel = false;
		}
		else if (collision.collision_type == COLLISION_TYPE::PLAYER_WITH_ENEMY
			&& !registry.invincibility.has(entity)) {

			if (!registry.collideEnemies.has(player)) continue;

			registry.invincibility.emplace(entity);

			// When player collides with enemy, only player gets knocked back,
			// towards its relative direction from the enemy
			Entity enemy_entity = collision.other_entity;
			Transform& enemy_transform = registry.transforms.get(enemy_entity);
			Motion& enemy_motion = registry.motions.get(enemy_entity);
			vec2 knockback_direction = normalize(transform.position - enemy_transform.position);
			motion.velocity = (enemy_motion.max_velocity + 1000) * knockback_direction;
			allow_accel = false;

			// Update player health
			if (!DEBUG_MODE) {
				Health& playerHealth = registry.healthValues.get(entity);
				playerHealth.health -= 10.0;
			}

			shakeCamera(4.f, 200.f, 10.f, knockback_direction);
			Mix_PlayChannel(chunkToChannel["player_hit"], soundChunks["player_hit"], 0);
			squish(player, 0.96f);
		}
		else if (collision.collision_type == COLLISION_TYPE::PLAYER_WITH_CHEST) {
			updateSpaceBarPressDuration();
			Entity chestEntity = collision.other_entity;
			Chest& chest = registry.chests.get(chestEntity);

			if (!chest.isOpened && isHoldingSpace(100.0f)) {
				enemy_spawn_cooldown = 1000.f; // lower cooldown set by step_chests()

				chest.isOpened = true;
				Entity abilityEntity = Entity();

				// Grant the ability to the player
				switch (chest.ability) {
				case REGION_GOAL_ID::SWORD_ATTACK: {
					registry.playerAbilities.insert(abilityEntity, { PLAYER_ABILITY_ID::SWORD });
					createSword(renderer, player);
					dialog_system->add_dialog(TEXTURE_ASSET_ID::TUTORIAL_UNLOCK_SWORD, 1500.f);
					if (controller_mode) {
						dialog_system->add_dialog(TEXTURE_ASSET_ID::TUTORIAL_UNLOCK_SWORD_CONTROLLER, 1000.f);
					}
					else {
						dialog_system->add_dialog(TEXTURE_ASSET_ID::TUTORIAL_UNLOCK_SWORD_MOUSE, 1000.f);
					}
					Mix_PlayChannel(chunkToChannel["sword_unlock"], soundChunks["sword_unlock"], 0);
					std::cout << "Player received SWORD ability from chest." << std::endl;
					break;
				}
				case REGION_GOAL_ID::MULTIPLE_BULLETS: {
					registry.playerAbilities.insert(abilityEntity, { PLAYER_ABILITY_ID::BULLET_BOOST });
					assert(registry.guns.has(player));
					Gun& gun_component = registry.guns.get(player);
					gun_component.attack_delay /= 2;
					Entity gun_entity = getAttachment(player, ATTACHMENT_ID::GUN);
					assert(registry.colors.has(gun_entity));
					vec4& gun_color = registry.colors.get(gun_entity);
					gun_color.g = 0.5f;
					gun_color.b = 0.5f;
					assert(registry.attachments.has(gun_entity));
					Attachment& att = registry.attachments.get(gun_entity);	// This is a quick workaround
					att.relative_transform_2.scale({ 1.5f, 1.5f });
					dialog_system->add_dialog(TEXTURE_ASSET_ID::TUTORIAL_UNLOCK_BULLETBOOST, 1500.f);
					Mix_PlayChannel(chunkToChannel["bullet_unlock"], soundChunks["bullet_unlock"], 0);
					std::cout << "Player received BULLET_BOOST ability from chest." << std::endl;
					break;
				}
				case REGION_GOAL_ID::HEALTH_BOOST: {
					registry.playerAbilities.insert(abilityEntity, { PLAYER_ABILITY_ID::HEALTH_BOOST });
					Health& health = registry.healthValues.get(player);
					assert(registry.renderRequests.has(healthbar_frame));
					registry.renderRequests.get(healthbar_frame).used_texture = TEXTURE_ASSET_ID::HEALTHBAR_FRAME_BOOST;
					assert(registry.healthbar.has(healthbar));
					registry.healthbar.get(healthbar).full_health_color = { 0.f, 1.f, 1.f, 1.f };
					dialog_system->add_dialog(TEXTURE_ASSET_ID::TUTORIAL_UNLOCK_HEALTHBOOST, 1500.f);
					health.healthMultiplier = 2.0f; // Double the health multiplier
					health.maxHealth *= health.healthMultiplier; // Update maximum health
					health.health = health.maxHealth; // Set current health to max
					health.healthIncrement = 1.0f; // Set health increment, e.g., 1 health point per second
					Mix_PlayChannel(chunkToChannel["health_unlock"], soundChunks["health_unlock"], 0);
					std::cout << "Player received HEALTH_BOOST ability from chest." << std::endl;
					break;
				}
				case REGION_GOAL_ID::DASH: {
					registry.playerAbilities.insert(abilityEntity, { PLAYER_ABILITY_ID::DASHING });
					createDashing(player);
					dialog_system->add_dialog(TEXTURE_ASSET_ID::TUTORIAL_UNLOCK_DASHING, 1500.f);
					if (controller_mode) {
						dialog_system->add_dialog(TEXTURE_ASSET_ID::TUTORIAL_UNLOCK_DASHING_CONTROLLER, 1000.f);
					}
					else {
						dialog_system->add_dialog(TEXTURE_ASSET_ID::TUTORIAL_UNLOCK_DASHING_KEYBOARD, 1000.f);
					}
					Mix_PlayChannel(chunkToChannel["dash_unlock"], soundChunks["dash_unlock"], 0);
					std::cout << "Player received DASHING ability from chest." << std::endl;
					break;
				}
				default:
					std::cout << "Unknown ability. Error." << std::endl;
				}

				for (auto& region : registry.regions.components) {
					if (region.goal == chest.ability) {
						region.is_cleared = true;
					}
				}

				garbage.push_back(chestEntity);
			}
			else if (!chest.isOpened) {
				show_hold_guide = true;
			}
		}
		else if (collision.collision_type == COLLISION_TYPE::PLAYER_WITH_CURE) {
			Entity cureEntity = collision.other_entity;
			registry.game.get(game_entity).isCureObtained = true;

			for (Region& region : registry.regions.components) {
				if (region.goal == REGION_GOAL_ID::CURE) {
					region.is_cleared = true;
				}
				else if (region.goal == REGION_GOAL_ID::CANCER_CELL) {
					Entity secondBoss = createSecondBoss(renderer, region.interest_point);
					createWaypoint(region.goal, secondBoss);
				}
			}

			garbage.push_back(cureEntity);
		}
		else if (collision.collision_type == COLLISION_TYPE::BULLET_WITH_ENEMY) {
			// When bullet collides with enemy, only enemy gets knocked back,
			// towards its relative direction from the enemy
			Entity enemy_entity = collision.other_entity;
			Enemy& enemyAttrib = registry.enemies.get(enemy_entity);
			if (enemyAttrib.type != ENEMY_ID::BOSS) {
				Transform& enemy_transform = registry.transforms.get(enemy_entity);
				Motion& enemy_motion = registry.motions.get(enemy_entity);
				vec2 knockback_direction = normalize(enemy_transform.position - transform.position);
				enemy_motion.velocity = enemy_motion.max_velocity * knockback_direction;
				enemy_motion.allow_accel = false;
				squish(enemy_entity, 0.95f);
			}

			// Deal damage to enemy
			Health& enemyHealth = registry.healthValues.get(enemy_entity);
			enemyHealth.health -= registry.projectiles.get(entity).damage;

			Mix_PlayChannel(chunkToChannel["enemy_hit"], soundChunks["enemy_hit"], 0);

			garbage.push_back(entity);

		}
		else if (collision.collision_type == COLLISION_TYPE::BULLET_WITH_CYST) {
			Entity cyst = collision.other_entity;

			// Deal damage to enemy
			Health& health = registry.healthValues.get(cyst);
			health.health -= registry.projectiles.get(entity).damage;

			Mix_PlayChannel(chunkToChannel["enemy_hit"], soundChunks["enemy_hit"], 0);
			squish(cyst, 0.9f);
			garbage.push_back(entity);
		}
		else if (collision.collision_type == COLLISION_TYPE::SWORD_WITH_ENEMY) {
			assert(registry.attachments.has(entity));
			Entity sword_holder = registry.attachments.get(entity).parent;
			assert(registry.melees.has(sword_holder));
			Entity enemy_entity = collision.other_entity;
			Enemy& enemyAttrib = registry.enemies.get(enemy_entity);
			if (enemyAttrib.sword_attack_cd <= 0.f) {
				Transform& enemy_transform = registry.transforms.get(enemy_entity);
				Motion& enemy_motion = registry.motions.get(enemy_entity);
				vec2 knockback_direction = normalize(enemy_transform.position - transform.position);
				// No knockback on boss
				if (enemyAttrib.type != ENEMY_ID::BOSS) {
					if (registry.melees.get(sword_holder).animation_timer > 0.f) {
						enemy_motion.velocity = enemy_motion.max_velocity * knockback_direction;
					}
					else {
						// Less knockback when sword is not slashing
						enemy_motion.velocity = enemy_motion.max_velocity / 2.f * knockback_direction;
					}
					squish(enemy_entity, 0.95f);
				}
				enemy_motion.allow_accel = false;

				// Deal damage to enemy
				Health& enemyHealth = registry.healthValues.get(enemy_entity);
				if (registry.melees.get(sword_holder).animation_timer > 0.f) {
					enemyHealth.health -= registry.melees.get(sword_holder).damage;
				}
				else {
					// Deal less damage when sword is not slashing
					enemyHealth.health -= registry.melees.get(sword_holder).damage / 4.f;
				}

				// Give enemy invincibility to sword for a moment after taking an attack
				enemyAttrib.sword_attack_cd = 500.f;

				Mix_PlayChannel(chunkToChannel["enemy_hit"], soundChunks["enemy_hit"], 0);
			}

		}
		else if (collision.collision_type == COLLISION_TYPE::SWORD_WITH_CYST) {
			assert(registry.attachments.has(entity));
			Entity sword_holder = registry.attachments.get(entity).parent;
			assert(registry.melees.has(sword_holder));

			Entity cyst = collision.other_entity;
			Cyst& cyst_attrib = registry.cysts.get(cyst);

			if (cyst_attrib.sword_attack_cd <= 0.f) {
				// Deal damage to cyst
				Health& health = registry.healthValues.get(cyst);
				if (registry.melees.get(sword_holder).animation_timer > 0.f) {
					health.health -= registry.melees.get(sword_holder).damage;
				}
				else {
					health.health -= registry.melees.get(sword_holder).damage / 8.f;
				}

				// Give cyst invincibility to sword for a moment after taking an attack
				cyst_attrib.sword_attack_cd = 500.f;
				squish(cyst, 0.9f);

				Mix_PlayChannel(chunkToChannel["enemy_hit"], soundChunks["enemy_hit"], 0);
			}
		}
		else if (collision.collision_type == COLLISION_TYPE::PLAYER_WITH_CYST
			&& !registry.invincibility.has(entity)) {
			Entity cyst = collision.other_entity;
			Transform& cyst_transform = registry.transforms.get(cyst);
			vec2 knockback_direction = normalize(transform.position - cyst_transform.position);
			motion.velocity = 150.f * knockback_direction;
			allow_accel = false;
		}
		else if (collision.collision_type == COLLISION_TYPE::BULLET_WITH_PLAYER
			&& !registry.invincibility.has(player)) {

			if (!registry.collideEnemies.has(player)) continue;

			registry.invincibility.emplace(player);

			Transform& player_transform = registry.transforms.get(player);
			Motion& player_motion = registry.motions.get(player);
			vec2 knockback_direction = normalize(player_transform.position - transform.position);

			player_motion.velocity = (motion.max_velocity + 1000) * knockback_direction;
			allow_accel = false;

			// Deal damage to player
			Health& playerHealth = registry.healthValues.get(player);
			playerHealth.health -= registry.projectiles.get(entity).damage;
			shakeCamera(3.f, 150.f, 3.f, knockback_direction);
			Mix_PlayChannel(chunkToChannel["player_hit"], soundChunks["player_hit"], 0);
			squish(player, 0.96f);

			garbage.push_back(entity);
		}
		else if (collision.collision_type == COLLISION_TYPE::BULLET_WITH_BOUNDARY) {
			garbage.push_back(entity);
		}
	}
	if (show_hold_guide) {
		show_hold_to_collect();
	}
	else {
		registry.colors.get(hold_to_collect).a = 0.f;
	}
	for (Entity i : garbage) {
		registry.remove_all_components_of(i);
	}

	// Remove all collisions from this simulation step
	registry.collisions.clear();
}


// Should the game be over ?
bool WorldSystem::is_over() const {
	return state == GAME_STATE::ENDED || bool(glfwWindowShouldClose(window));
}


// On key callback
void WorldSystem::on_key(int key, int, int action, int mod) {
	// key is of 'type' GLFW_KEY_
	// action can be GLFW_PRESS GLFW_RELEASE GLFW_REPEAT

	// Pausing game
	if (action == GLFW_RELEASE && key == GLFW_KEY_ESCAPE) {
		if (state == GAME_STATE::RUNNING) {
			state = GAME_STATE::PAUSE_MENU;
		}
	}

	if (action == GLFW_PRESS && key == GLFW_MOUSE_BUTTON_LEFT) {
		if (state == GAME_STATE::PAUSE_MENU || state == GAME_STATE::START_MENU) {
			menu_system->recent_click_coord = mouse;
			printf("click at %f, %f\n", mouse.x, mouse.y);
		}
	}

	if (key == GLFW_KEY_SPACE) {
		if (action == GLFW_PRESS) {
			keys_pressed[key] = 1;
			spaceBarPressDuration = 0.0f; // Reset duration on new press
		}
		else if (action == GLFW_RELEASE) {
			keys_pressed[key] = 0;
		}
	}

	if (action == GLFW_PRESS) {
		keys_pressed[key] = 1;
	}
	if (action == GLFW_RELEASE) {
		keys_pressed[key] = 0;
	}
	// Debugging
	if (key == GLFW_KEY_F) {
		if (action == GLFW_RELEASE)
			debugging.in_debug_mode = false;
		else
			debugging.in_debug_mode = true;
	}

	// Control the current speed with `<` `>`
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_COMMA) {
		current_speed -= 0.1f;
		printf("Current speed = %f\n", current_speed);
	}
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_PERIOD) {
		current_speed += 0.1f;
		printf("Current speed = %f\n", current_speed);
	}
	current_speed = fmax(0.f, current_speed);
}

void WorldSystem::on_mouse_move(vec2 pos) {
	vec2 mouseScreenCoord = vec2(pos.x - CONTENT_WIDTH_PX / 2, CONTENT_HEIGHT_PX / 2 - pos.y);
	Transform player_transform = registry.transforms.get(player);
	float angle = player_transform.angle;
	float offsetX = -60 * sin(angle);
	float offsetY = -60 * cos(angle);
	vec2 cursor_position = { clamp(mouseScreenCoord.x + offsetX, -CONTENT_WIDTH_PX / 2.f + 60, CONTENT_WIDTH_PX / 2.f - 60),
							 clamp(mouseScreenCoord.y + offsetY, -CONTENT_HEIGHT_PX / 2.f + 60, CONTENT_HEIGHT_PX / 2.f - 60) };

	//set cursor/crosshair location
	registry.transforms.get(cursor).position = cursor_position;

	// hide cursor if on top of player
	vec2 mouseWorldCoord = vec2(inverse(renderer->createViewMatrix()) * vec3(cursor_position, 1.0));
	if (distance(mouseWorldCoord, player_transform.position) < 70) {
		registry.colors.get(cursor).a = 0.f;
	}
	else {
		registry.colors.get(cursor).a = 1.f;
	}

	//set mouse for player angle
	mouse = mouseScreenCoord;
}

void axesupdate() {
	int axesCount;
	const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axesCount);
	axes_state[0] = axes[0];
	axes_state[1] = axes[1];
	axes_state[2] = axes[2];
	axes_state[3] = axes[3];
	axes_state[4] = axes[4];
	axes_state[5] = axes[5];

}

void WorldSystem::menu_controller(float elapsed_ms_since_last_update) {
	int present = glfwJoystickPresent(GLFW_JOYSTICK_1);

	if (present) {
		menu_timer -= elapsed_ms_since_last_update;
		if (menu_timer > 0) {
			return;
		}
		menu_timer = 150;

		axesupdate();
		int count;
		const unsigned char* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &count);
		controller_buttons = buttons;



		if (state == GAME_STATE::START_MENU && button_select == BUTTON_SELECT::NONE) {
			if (axes_state[1] > 0.3 || axes_state[1] < -0.3 || buttons[15] || buttons[17]) {
				button_select = BUTTON_SELECT::START;
				mouse = { 0,40 };
			}

		}
		else if (state == GAME_STATE::START_MENU && button_select == BUTTON_SELECT::START) {

			if (axes_state[1] > 0.3 || buttons[17]) {
				button_select = BUTTON_SELECT::LOAD;
				mouse = { 0,-150 };

			}

			if (axes_state[1] < -0.3 || buttons[15]) {
				button_select = BUTTON_SELECT::EXIT;
				mouse = { 0,-350 };

			}

			if (axes_state[0] < -0.3 || buttons[18]) {
				button_select = BUTTON_SELECT::EASY_MODE;
				mouse = { -550, -150 };
			}

		}
		else if (state == GAME_STATE::START_MENU && button_select == BUTTON_SELECT::LOAD) {

			if (axes_state[1] > 0.3 || buttons[17]) {
				button_select = BUTTON_SELECT::EXIT;
				mouse = { 0,-350 };

			}

			if (axes_state[1] < -0.3 || buttons[15]) {
				button_select = BUTTON_SELECT::START;
				mouse = { 0,50 };

			}
			if (axes_state[0] < -0.3 || buttons[18]) {
				button_select = BUTTON_SELECT::EASY_MODE;
				mouse = { -550, -150 };
			}


		}
		else if (state == GAME_STATE::START_MENU && button_select == BUTTON_SELECT::EXIT) {

			if (axes_state[1] > 0.3 || buttons[17]) {
				button_select = BUTTON_SELECT::START;
				mouse = { 0,50 };

			}

			if (axes_state[1] < -0.3 || buttons[15]) {
				button_select = BUTTON_SELECT::LOAD;
				mouse = { 0,-150 };

			}
			if (axes_state[0] < -0.3 || buttons[18]) {
				button_select = BUTTON_SELECT::HARD_MODE;
				mouse = { -540, -350 };
			}


		}

		else if (state == GAME_STATE::START_MENU && button_select == BUTTON_SELECT::EASY_MODE) {

			if (axes_state[1] > 0.3 || buttons[17]) {
				button_select = BUTTON_SELECT::HARD_MODE;
				mouse = { -540, -350 };

			}

			if (axes_state[1] < -0.3 || buttons[15]) {
				button_select = BUTTON_SELECT::HARD_MODE;
				mouse = { -540, -350 };

			}
			if (axes_state[0] > 0.3 || buttons[16]) {
				button_select = BUTTON_SELECT::LOAD;
				mouse = { 0,-150 };
			}
		}
		else if (state == GAME_STATE::START_MENU && button_select == BUTTON_SELECT::HARD_MODE) {

			if (axes_state[1] > 0.3 || buttons[17]) {
				button_select = BUTTON_SELECT::EASY_MODE;
				mouse = { -550, -150 };

			}

			if (axes_state[1] < -0.3 || buttons[15]) {
				button_select = BUTTON_SELECT::EASY_MODE;
				mouse = { -550, -150 };

			}
			if (axes_state[0] > 0.3 || buttons[16]) {
				button_select = BUTTON_SELECT::EXIT;
				mouse = { 0,-350 };
			}
		}

		else if (state == GAME_STATE::PAUSE_MENU && button_select == BUTTON_SELECT::NONE) {
			if (axes_state[1] > 0.3 || axes_state[1] < -0.3 || buttons[17] || buttons[15]) {
				button_select = BUTTON_SELECT::RESUME;
				mouse = { 0,200 };
			}

		}
		else if (state == GAME_STATE::PAUSE_MENU && button_select == BUTTON_SELECT::RESUME) {
			if (axes_state[1] > 0.3 || buttons[17]) {
				button_select = BUTTON_SELECT::SAVE;
				mouse = { 0,0 };

			}

			if (axes_state[1] < -0.3 || buttons[15]) {
				button_select = BUTTON_SELECT::EXIT_CURR_PLAY;
				mouse = { 0,-400 };

			}



		}
		else if (state == GAME_STATE::PAUSE_MENU && button_select == BUTTON_SELECT::SAVE) {
			if (axes_state[1] > 0.3 || buttons[17]) {
				button_select = BUTTON_SELECT::MUTE;
				mouse = { 0,-200 };

			}

			if (axes_state[1] < -0.3 || buttons[15]) {
				button_select = BUTTON_SELECT::RESUME;
				mouse = { 0,200 };

			}

		}
		else if (state == GAME_STATE::PAUSE_MENU && button_select == BUTTON_SELECT::MUTE) {
			if (axes_state[1] > 0.3 || buttons[17]) {
				button_select = BUTTON_SELECT::EXIT_CURR_PLAY;
				mouse = { 0,-400 };

			}

			if (axes_state[1] < -0.3 || buttons[15]) {
				button_select = BUTTON_SELECT::SAVE;
				mouse = { 0,0 };

			}

		}
		else if (state == GAME_STATE::PAUSE_MENU && button_select == BUTTON_SELECT::EXIT_CURR_PLAY) {
			if (axes_state[1] > 0.3 || buttons[17]) {
				button_select = BUTTON_SELECT::RESUME;
				mouse = { 0,200 };

			}

			if (axes_state[1] < -0.3 || buttons[15]) {
				button_select = BUTTON_SELECT::MUTE;
				mouse = { 0,-200 };

			}

		}


		if (buttons[1]) {
			if (state == GAME_STATE::PAUSE_MENU || state == GAME_STATE::START_MENU) {
				menu_system->recent_click_coord = mouse;
				controller_mode = 1;
			}
		}
	}


}


void WorldSystem::step_controller() {
	int present = glfwJoystickPresent(GLFW_JOYSTICK_1);
	if (present) {
		axesupdate();



		int count;
		const unsigned char* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &count);
		controller_buttons = buttons;

		if (buttons[9]) {
			if (state == GAME_STATE::RUNNING) {
				state = GAME_STATE::PAUSE_MENU;
				button_select = BUTTON_SELECT::NONE;
			}
		}
		if (buttons[18]) {
			controller_mode = 0;

		}
		if (buttons[16]) {
			controller_mode = 1;

		}
	}
	else {
		axes_state[0] = 0.0;
		axes_state[1] = 0.0;
		axes_state[2] = 0.0;
		axes_state[3] = 0.0;
		axes_state[4] = 0.0;
		axes_state[5] = 0.0;

	}
}

void WorldSystem::control_movement(float elapsed_ms) {
	Motion& playermovement = registry.motions.get(player);

	// Vertical movement
	if (keys_pressed[GLFW_KEY_W]) {
		playermovement.velocity.y += elapsed_ms * playermovement.acceleration_unit;
	}
	if (keys_pressed[GLFW_KEY_S]) {
		playermovement.velocity.y -= elapsed_ms * playermovement.acceleration_unit;
	}
	playermovement.velocity.y -= elapsed_ms * playermovement.acceleration_unit * axes_state[1];


	if ((!(keys_pressed[GLFW_KEY_S] || keys_pressed[GLFW_KEY_W]) || (keys_pressed[GLFW_KEY_S] && keys_pressed[GLFW_KEY_W])) && (axes_state[1] <= 0.3 && axes_state[1] >= -0.3)) {
		playermovement.velocity.y *= pow(playermovement.deceleration_unit, elapsed_ms);
	}

	// Horizontal movement
	if (keys_pressed[GLFW_KEY_D]) {
		playermovement.velocity.x += elapsed_ms * playermovement.acceleration_unit;
	}
	if (keys_pressed[GLFW_KEY_A]) {
		playermovement.velocity.x -= elapsed_ms * playermovement.acceleration_unit;
	}
	playermovement.velocity.x += elapsed_ms * playermovement.acceleration_unit * axes_state[0];

	if ((!(keys_pressed[GLFW_KEY_D] || keys_pressed[GLFW_KEY_A]) || (keys_pressed[GLFW_KEY_D] && keys_pressed[GLFW_KEY_A])) && (axes_state[0] <= 0.3 && axes_state[0] >= -0.3)) {
		playermovement.velocity.x *= pow(playermovement.deceleration_unit, elapsed_ms);
	}

	// If player is NOT dashing then its velocity need to be normalized
	if (!registry.dashes.has(player) || registry.dashes.get(player).active_timer_ms <= 0) {
		float magnitude = length(playermovement.velocity);
		if (magnitude > playermovement.max_velocity) {
			playermovement.velocity *= (playermovement.max_velocity / magnitude);
		}
	}
}

template <typename Component>
void WorldSystem::clearSpecificEntities(ComponentContainer<Component>& componentRegistry) {
	while (!componentRegistry.entities.empty()) {
		registry.remove_all_components_of(componentRegistry.entities.back());
	}
}

void WorldSystem::reset_game_state(bool hard_reset) {
	// Debugging for memory/component leaks
	std::cout << "\n------------ Before Cleanup ------------\n";
	registry.list_all_components();


	dialog_system->clear_pending_dialogs();
	// reverse active effects
	for (auto event : registry.timedEvents.components) {
		event.callback();
	}

	// Remove entities that will be recreated
	clearSpecificEntities(registry.timedEvents);
	clearSpecificEntities(registry.motions);
	clearSpecificEntities(registry.players);
	clearSpecificEntities(registry.collisions);
	clearSpecificEntities(registry.cysts);
	clearSpecificEntities(registry.waypoints);
	clearSpecificEntities(registry.chests);
	clearSpecificEntities(registry.cure);
	clearSpecificEntities(registry.projectiles);
	clearSpecificEntities(registry.enemies);
	clearSpecificEntities(registry.attachments);
	clearSpecificEntities(registry.deathTimers);
	clearSpecificEntities(registry.healthValues);
	clearSpecificEntities(registry.invincibility);
	clearSpecificEntities(registry.dashes);
	clearSpecificEntities(registry.collidePlayers);
	clearSpecificEntities(registry.collideEnemies);
	clearSpecificEntities(registry.melees);
	clearSpecificEntities(registry.guns);
	clearSpecificEntities(registry.bosses);

	if (hard_reset) {
		clearSpecificEntities(registry.game);
		clearSpecificEntities(registry.playerAbilities);
		clearSpecificEntities(registry.regions);

		// Clear individual entities
		registry.remove_all_components_of(healthbar);
		registry.remove_all_components_of(healthbar_frame);
		registry.remove_all_components_of(boss_healthbar_frame);
		registry.remove_all_components_of(boss_healthbar);
		registry.remove_all_components_of(death_screen);

		createRandomRegions(NUM_REGIONS, rng);
		registry.game.emplace(game_entity);

		// Recreate Healthbars
		std::tuple<Entity, Entity> healthbar_elems = createHealthbar({ -CONTENT_WIDTH_PX * 0.34, CONTENT_HEIGHT_PX * 0.43 }, STATUSBAR_SCALE);
		healthbar = std::get<0>(healthbar_elems);
		healthbar_frame = std::get<1>(healthbar_elems);
		healthbar_elems = createBossHealthbar({ 0.f, CONTENT_HEIGHT_PX * -0.42 }, STATUSBAR_SCALE);
		boss_healthbar = std::get<0>(healthbar_elems);
		boss_healthbar_frame = std::get<1>(healthbar_elems);
	}

	// Reset variables
	for (auto& count : enemyCounts) {
		count.second = 0;
	}
	current_speed = 1.f;
	individual_spawn_interval = 1000.f;
	enemy_spawn_cooldown = 5000.f;
	isShootingSoundQueued = false;
	state = GAME_STATE::RUNNING;
	Mix_FadeInMusic(backgroundMusic["main"], -1, 5000);

	std::cout << "\n------------ After Cleanup ------------\n";
	registry.list_all_components();
}

void WorldSystem::load_game() {
	printf("\n=========================\n|\tLoading\t\t|\n=========================\n");

	std::ifstream inFile("savegame.json");
	if (!inFile) {
		std::cerr << "Error opening file for reading." << std::endl;
		return;
	}
	json gameState;
	inFile >> gameState;

	// Clear current game state before loading
	reset_game_state(true);

	// Recreate required entities with the saved values

	// Deserialize game mode
	auto& gameModeData = gameState["gameMode"][0];
	assert(static_cast<int>(gameModeData["id"]) < game_mode_id_count);
	GAME_MODE_ID gm_id = static_cast<GAME_MODE_ID>(gameModeData["id"]);
	setGameMode(gm_id);

	// Deserialize Game Data
	assert(registry.game.has(game_entity));
	const auto& gameData = gameState["game"];
	Game& gameComponent = registry.game.get(game_entity);
	gameComponent.isFirstBossDefeated = gameData["isFirstBossDefeated"];
	gameComponent.isCureObtained = gameData["isCureObtained"];
	gameComponent.isSecondBossDefeated = gameData["isSecondBossDefeated"];
	gameComponent.isCystTutorialDisplayed = gameData["isCystTutorialDisplayed"];

	// Deserialize and recreate Regions
	loadRegions(gameState["regions"]);

	// Deserialize Player
	const auto& playerData = gameState["player"];
	player = createPlayer({ playerData["position"][0], playerData["position"][1] });
	effects_system->player = player;

	// Deserialize Player Abilities
	for (const auto& abilityId : gameState["playerAbilities"]) {
		PLAYER_ABILITY_ID ability = static_cast<PLAYER_ABILITY_ID>(abilityId);

		Entity abilityEntity = Entity();
		PlayerAbility abilityComponent;
		abilityComponent.id = ability;

		registry.playerAbilities.insert(abilityEntity, abilityComponent);
	}
	populate_player_abilities();

	Health& playerHealth = registry.healthValues.get(player);
	playerHealth.health = playerData["health"];

	// Deserialize Enemies
	for (const auto& enemyData : gameState["enemies"]) {
		vec2 position = { enemyData["position"][0], enemyData["position"][1] };
		float enemyHealth = enemyData["health"];
		ENEMY_ID enemyType = static_cast<ENEMY_ID>(enemyData["type"]);
		switch (enemyType) {
		case ENEMY_ID::BOSS:
			if (!gameComponent.isFirstBossDefeated) {
				Entity boss1 = createBoss(renderer, position, enemyHealth);
				createWaypoint(REGION_GOAL_ID::CURE, boss1);
			}
			break;
		case ENEMY_ID::FRIENDBOSS:
			if (!gameComponent.isSecondBossDefeated) {
				Entity boss2 = createSecondBoss(renderer, position, enemyHealth);
				createWaypoint(REGION_GOAL_ID::CANCER_CELL, boss2);
			}
			break;
		case ENEMY_ID::RED:
			createRedEnemy(position, enemyHealth);
			enemyCounts[ENEMY_ID::RED]++;
			break;
		case ENEMY_ID::GREEN:
			createGreenEnemy(position, enemyHealth);
			enemyCounts[ENEMY_ID::GREEN]++;
			break;
		case ENEMY_ID::YELLOW:
			createYellowEnemy(position, enemyHealth);
			enemyCounts[ENEMY_ID::YELLOW]++;
			break;
		default:
			// Handle unknown type
			break;
		}

		//std::cout << "Enemy loaded: Type = " << static_cast<int>(enemyType)
		//          << ", Position = (" << position.x << ", " << position.y << ")"
		//          << ", Health = " << enemyHealth << "\n";
	}

	// Deserialize Cysts
	for (const auto& cystData : gameState["cysts"]) {
		vec2 position = { cystData["position"][0], cystData["position"][1] };
		float cystHealth = cystData["health"];
		createCyst(position, cystHealth);

		//std::cout << "Cyst loaded: Position = (" << position.x << ", " << position.y << ")"
		//		<< ", Health = " << cystHealth << "\n";
	}

	// Deserialize Chests
	for (const auto& chestData : gameState["chests"]) {
		vec2 position = { chestData["position"][0], chestData["position"][1] };
		REGION_GOAL_ID ability = static_cast<REGION_GOAL_ID>(chestData["ability"]);
		Entity chest = createChest(position, ability);
		createWaypoint(ability, chest);
	}

	// Deserialize Cure
	if (!gameComponent.isCureObtained && gameState["cure"] != nullptr) {
		vec2 curePosition = { gameState["cure"]["position"][0], gameState["cure"]["position"][1] };
		Entity cure = createCure(curePosition);
		createWaypoint(REGION_GOAL_ID::CURE, cure);
	}

	inFile.close();
	std::cout << "Game state loaded." << std::endl;
}

void WorldSystem::loadRegions(const json& regionsData) {
	// Assuming the regions are saved in the same order they were created
	assert(regionsData.size() == registry.regions.components.size());

	bool cureUnlocked = registry.game.get(game_entity).isCureObtained;
	bool secondBossDefeated = registry.game.get(game_entity).isSecondBossDefeated;

	for (size_t i = 0; i < regionsData.size(); ++i) {
		const auto& regionData = regionsData[i];
		auto entity = registry.regions.entities[i];
		Region& region = registry.regions.get(entity);

		// Update region details from JSON data
		region.theme = static_cast<REGION_THEME_ID>(regionData["theme"]);
		region.goal = static_cast<REGION_GOAL_ID>(regionData["goal"]);
		region.enemy = static_cast<ENEMY_ID>(regionData["enemy"]);
		region.boss = static_cast<BOSS_ID>(regionData["boss"]);
		region.is_cleared = regionData["is_cleared"];
		vec2 interest_point = { regionData["interest_point"][0], regionData["interest_point"][1] };
		region.interest_point = interest_point;

		// Update RenderRequest component
		RenderRequest& renderReq = registry.renderRequests.get(entity);
		renderReq.used_texture = region_texture_map[region.theme];
	}
}

void WorldSystem::save_game() {
	json gameState = serializeGameState();
	std::ofstream outFile("savegame.json");
	if (!outFile) {
		std::cerr << "Error opening file for writing." << std::endl;
		return;
	}
	outFile << gameState.dump(4);
	outFile.close();
	std::cout << "Game state saved." << std::endl;
}

json WorldSystem::serializeGameState() {
	json gameState;

	// Serialize Regions
	for (const Region& region : registry.regions.components) {
		json regionData;
		regionData["theme"] = static_cast<int>(region.theme);
		regionData["goal"] = static_cast<int>(region.goal);
		regionData["enemy"] = static_cast<int>(region.enemy);
		regionData["boss"] = static_cast<int>(region.boss);
		regionData["is_cleared"] = region.is_cleared;
		regionData["interest_point"] = { region.interest_point.x, region.interest_point.y };
		gameState["regions"].push_back(regionData);
	}

	// Serialize Player
	const auto& playerTransform = registry.transforms.get(player);
	const auto& playerHealth = registry.healthValues.get(player);
	gameState["player"]["position"] = { playerTransform.position.x, playerTransform.position.y };
	gameState["player"]["health"] = playerHealth.health;

	const auto& gameComponent = registry.game.get(game_entity);
	gameState["game"]["isFirstBossDefeated"] = gameComponent.isFirstBossDefeated;
	gameState["game"]["isCureObtained"] = gameComponent.isCureObtained;
	gameState["game"]["isSecondBossDefeated"] = gameComponent.isSecondBossDefeated;
	gameState["game"]["isCystTutorialDisplayed"] = gameComponent.isCystTutorialDisplayed;

	for (const auto& abilityEntity : registry.playerAbilities.entities) {
		const auto& ability = registry.playerAbilities.get(abilityEntity);
		gameState["playerAbilities"].push_back(static_cast<int>(ability.id));
	}

	// Serialize Enemies
	auto& enemiesContainer = registry.enemies;
	for (uint i = 0; i < enemiesContainer.size(); i++) {
		const Enemy& enemyType = enemiesContainer.components[i];
		if (enemyType.type == ENEMY_ID::BOSS_ARM || enemyType.type == ENEMY_ID::FRIENDBOSSCLONE) continue;	// don't save these
		Entity enemyEntity = enemiesContainer.entities[i];
		const auto& enemyTransform = registry.transforms.get(enemyEntity);
		const auto& enemyHealth = registry.healthValues.get(enemyEntity);
		json enemyData;
		enemyData["position"] = { enemyTransform.position.x, enemyTransform.position.y };
		enemyData["health"] = enemyHealth.health;
		enemyData["type"] = static_cast<int>(enemyType.type);
		gameState["enemies"].push_back(enemyData);
	}

	// Serialize Cysts
	auto& cystsContainer = registry.cysts;
	for (uint i = 0; i < cystsContainer.size(); i++) {
		Entity cystEntity = cystsContainer.entities[i];
		const auto& cystTransform = registry.transforms.get(cystEntity);
		const auto& cystHealth = registry.healthValues.get(cystEntity);
		json cystData;
		cystData["position"] = { cystTransform.position.x, cystTransform.position.y };
		cystData["health"] = cystHealth.health;
		gameState["cysts"].push_back(cystData);
	}

	// Serialize Chests
	for (const Chest& chest : registry.chests.components) {
		if (!chest.isOpened) {
			json chestData;
			chestData["position"] = { chest.position.x, chest.position.y };
			chestData["ability"] = static_cast<int>(chest.ability);
			gameState["chests"].push_back(chestData);
		}
	}

	// Serialize Cure (if exists and not picked up)
	bool cureFound = false;
	for (const auto& cureEntity : registry.cure.entities) {
		const auto& cureTransform = registry.transforms.get(cureEntity);
		json cureData;
		cureData["position"] = { cureTransform.position.x, cureTransform.position.y };
		gameState["cure"] = cureData;
		cureFound = true;
		break;
	}
	if (!cureFound) {
		gameState["cure"] = nullptr;
	}

	// Serialize game mode
	GameMode& gameMode = registry.gameMode.components.back();
	json gameModeData;
	gameModeData["id"] = gameMode.id;

	gameState["gameMode"].push_back(gameModeData);

	return gameState;
}

bool WorldSystem::hasPlayerAbility(PLAYER_ABILITY_ID abilityId) {
	for (uint i = 0; i < registry.playerAbilities.components.size(); i++) {
		if (registry.playerAbilities.components[i].id == abilityId) {
			return true;
		}
	}
	return false;
}

Entity& WorldSystem::getAttachment(Entity character, ATTACHMENT_ID type) {
	Entity attachment_entity;
	for (uint i = 0; i < registry.attachments.size(); i++) {
		Attachment& att = registry.attachments.components[i];
		if (att.parent == character && att.type == type)
			attachment_entity = registry.attachments.entities[i];
	}
	return attachment_entity;
}

void WorldSystem::control_action() {
	if (controller_buttons != NULL) {
		if (axes_state[4] > 0.1) {
			player_shoot();
		}
		if (axes_state[3] > 0.1) {
			player_sword_slash();
		}
		if (controller_buttons[4]) {
			player_dash();
		}

	}
	if (keys_pressed[GLFW_MOUSE_BUTTON_LEFT]) {
		player_shoot();
	}
	if (keys_pressed[GLFW_MOUSE_BUTTON_RIGHT]) {
		player_sword_slash();
	}
	if (keys_pressed[GLFW_KEY_LEFT_SHIFT]) {
		player_dash();
	}
}

void WorldSystem::control_direction() {
	if (registry.deathTimers.has(player)) {
		return;
	}

	// Set player angle
	assert(registry.transforms.has(player));
	Transform& playertransform = registry.transforms.get(player);
	Transform& cursortransform = registry.transforms.get(cursor);

	int present = glfwJoystickPresent(GLFW_JOYSTICK_1);
	if (present && controller_mode) {

		//float controller_angle = playertransform.angle;
		if (!(axes_state[5] <= 0.4 && axes_state[5] >= -0.4) || !(axes_state[2] <= 0.4 && axes_state[2] >= -0.4)) {
			float controller_angle = atan2f(-axes_state[5] * abs(axes_state[5]), axes_state[2] * abs(axes_state[2]));
			playertransform.angle = controller_angle + playertransform.angle_offset;

			// Set cursor position
			const float offset_pixels = 40.f; // Same as the gun
			const float dist_to_cursor = 200.f;
			Transformation t;
			t.rotate(controller_angle);
			t.translate({ dist_to_cursor, -offset_pixels });
			cursortransform.position = t.mat[2];
		}

	}
	else {
		float mouse_angle = atan2f(mouse.y, mouse.x);
		playertransform.angle = mouse_angle + playertransform.angle_offset;

		// Set cursor position to be in front of the gun
		const float offset_pixels = 40.f; // Same as the gun
		Transformation t;
		t.rotate(mouse_angle);
		t.translate({ length(mouse), -offset_pixels });
		cursortransform.position = t.mat[2];


	}
	// hide cursor if on top of player
	if (length(cursortransform.position) < 70) {
		registry.colors.get(cursor).a = 0.f;
	}
	else {
		registry.colors.get(cursor).a = 1.f;
	}
}

void WorldSystem::player_shoot() {
	Gun& playerGun = registry.guns.get(player);
	if (playerGun.attack_timer <= 0) {
		createBullet(player, playerGun.bullet_size, playerGun.bullet_color);
		playerGun.attack_timer = playerGun.attack_delay;
	}
	else if (playerGun.attack_timer > PLAYER_ATTACK_DELAY * 2 &&
		!Mix_Playing(chunkToChannel["player_shoot_1"])) {
		// for attack delay effect or long cooldown attacks
		Mix_PlayChannel(chunkToChannel["player_shoot_1"], soundChunks["no_ammo"], 0);
	}
}

void WorldSystem::player_sword_slash() {
	if (registry.melees.has(player)) {
		Melee& player_sword = registry.melees.get(player);
		if (player_sword.attack_timer <= 0) {
			// Do the slashing
			player_sword.attack_timer = player_sword.attack_delay;
			player_sword.animation_timer = player_sword.attack_delay / 2.f;
			Entity sword_entity = player_sword.melee_entity;
			assert(registry.attachments.has(sword_entity) && registry.motions.has(sword_entity));
			Attachment& att = registry.attachments.get(sword_entity);
			Motion& sword_motion = registry.motions.get(sword_entity);
			sword_motion.angular_velocity = sword_motion.max_angular_velocity;
			Mix_PlayChannel(chunkToChannel["sword_unlock"], soundChunks["sword_unlock"], 0);

		}
	}
}

void WorldSystem::player_dash() {
	// Check if the player's dash ability is unlocked
	if (!registry.dashes.has(player)) return;

	Dash& playerDash = registry.dashes.get(player);
	if (playerDash.delay_timer_ms > 0) { //make sure player dash cooldown is 0 if not don't allow them to dash
		return;
	}

	Motion& playerMovement = registry.motions.get(player);
	vec2 dashDirection;

	if (keys_pressed[GLFW_KEY_W] || keys_pressed[GLFW_KEY_A] || keys_pressed[GLFW_KEY_S] || keys_pressed[GLFW_KEY_D]) {
		// prioritize direction of key presses (players intended direction)
		dashDirection.x = static_cast<float> (keys_pressed[GLFW_KEY_D] - keys_pressed[GLFW_KEY_A]);
		dashDirection.y = static_cast<float> (keys_pressed[GLFW_KEY_W] - keys_pressed[GLFW_KEY_S]);


		// handle conflicting key-presses
		bool leftAndRight = keys_pressed[GLFW_KEY_D] && keys_pressed[GLFW_KEY_A];
		bool upAndDown = keys_pressed[GLFW_KEY_W] && keys_pressed[GLFW_KEY_S];
		if ((leftAndRight != upAndDown) || (leftAndRight && upAndDown)) {
			dashDirection = normalize(playerMovement.velocity);
		}

		dashDirection = normalize(dashDirection);
	}
	else if (length(playerMovement.velocity) < 10.f) {
		// If player is barely moving, dash in mouse direction
		Transform transform = registry.transforms.get(player);
		float playerAngle = transform.angle - transform.angle_offset + M_PI;

		dashDirection = normalize(vec2(cos(playerAngle), sin(playerAngle)));
	}
	else {
		// otherwise dash in direction of player movement
		dashDirection = normalize(playerMovement.velocity);
	}

	playerMovement.velocity += playerDash.max_dash_velocity * dashDirection;

	playerDash.delay_timer_ms = playerDash.delay_duration_ms;
	playerDash.active_timer_ms = playerDash.active_duration_ms;

	Mix_PlayChannel(chunkToChannel["player_dash"], soundChunks["player_dash"], 0);
}

void WorldSystem::handle_shooting_sound_effect() {
	Gun& playerWeapon = registry.guns.get(player);

	auto play_or_queue_sound = [&]() {
		if (!Mix_Playing(chunkToChannel["player_shoot_1"])) {
			// duck volume if another sound is playing
			int volume = Mix_Playing(-1) ? 20 : 60;
			Mix_VolumeChunk(soundChunks["player_shoot_1"], volume);
			Mix_PlayChannel(chunkToChannel["player_shoot_1"], soundChunks["player_shoot_1"], 0);
		}
		else {
			// Queue sound if it's already playing
			isShootingSoundQueued = true;
		}
		};

	if (playerWeapon.attack_timer == playerWeapon.attack_delay) {
		// We know player just attacked because attack_timer reset
		play_or_queue_sound();
	}
	else if (isShootingSoundQueued && !Mix_Playing(chunkToChannel["player_shoot_1"])) {
		// If attack_timer ticked down too much it's probably too late to play a sound
		if (playerWeapon.attack_timer >= playerWeapon.attack_delay / 3.0) {
			play_or_queue_sound();
		}
		isShootingSoundQueued = false;
	}
}

void WorldSystem::setGameMode(GAME_MODE_ID id) {
	GameMode gm;
	if (id == GAME_MODE_ID::EASY_MODE) {
		gm = easyMode;
	}
	else {
		gm = regularMode;
	}
	registry.gameMode.components.back() = gm;
	maxEnemies[ENEMY_ID::RED] = gm.max_red;
	maxEnemies[ENEMY_ID::GREEN] = gm.max_green;
	maxEnemies[ENEMY_ID::YELLOW] = gm.max_yellow;
	for (int i = 0; i < registry.enemies.components.size(); i++) {
		auto& enemyCom = registry.enemies.components[i];
		auto& enemyEn = registry.enemies.entities[i];
		registry.healthValues.get(enemyEn).health = gm.enemy_health_map[enemyCom.type];
	}
}

void WorldSystem::step_menu() {
	if (state == GAME_STATE::START_MENU) {
		MENU_OPTION option = menu_system->poll_start_menu();
		if (option == MENU_OPTION::START_GAME) {
			restart_game(true);
		}
		else if (option == MENU_OPTION::LOAD_GAME) {
			load_game();
		}
		else if (option == MENU_OPTION::EXIT_GAME) {
			state = GAME_STATE::ENDED;
		}
		else if (option == MENU_OPTION::EASY_GAME_MODE) {
			setGameMode(GAME_MODE_ID::EASY_MODE);
		}
		else if (option == MENU_OPTION::REGULAR_GAME_MODE) {
			setGameMode(GAME_MODE_ID::REGULAR_MODE);
		}
	}
	else if (state == GAME_STATE::PAUSE_MENU) {
		MENU_OPTION option = menu_system->poll_pause_menu();
		if (option == MENU_OPTION::RESUME_GAME) {
			state = GAME_STATE::RUNNING;
		}
		else if (option == MENU_OPTION::SAVE_GAME) {
			save_game();
		}
		else if (option == MENU_OPTION::MUTE_SOUND) {
			Mix_Volume(-1, 0);
			Mix_VolumeMusic(0);
		}
		else if (option == MENU_OPTION::UNMUTE_SOUND) {
			Mix_Volume(-1, 128);
			Mix_VolumeMusic(45);
		}
		else if (option == MENU_OPTION::EXIT_CURR_PLAY) {
			state = GAME_STATE::START_MENU;
			Mix_FadeInMusic(backgroundMusic["menu"], -1, 5000);
			button_select = BUTTON_SELECT::NONE;
			step_menu();
		}
	}
}

// Returns true if new boss fight is detected and false otherwise
void WorldSystem::detect_bossfight() {
	if (registry.bosses.size() > 0) {
		// Assume only 1 boss exists at any time
		Entity current_boss = registry.bosses.entities[0];
		if (!registry.bosses.get(current_boss).activated) {
			Transform& boss_transform = registry.transforms.get(current_boss);
			Transform& player_transform = registry.transforms.get(player);
			vec2 player_pos = player_transform.position;
			vec2 boss_pos = boss_transform.position;
			vec2 distance = abs(player_pos - boss_pos);	// As soon as half of the boss is visible
			// Enter boss fight when player is close enough to the boss
			if (distance.x < CONTENT_WIDTH_PX / 2.f && distance.y < CONTENT_HEIGHT_PX / 2.f) {
				registry.bosses.get(current_boss).activated = true;
				registry.collidePlayers.emplace(current_boss);	// set here to avoid hitting boss when inactive
				// start boss music
				Mix_FadeInMusic(backgroundMusic["boss"], -1, 5000);

				// rotate player to face the boss
				vec2 delta_pos = boss_pos - player_pos;
				player_transform.angle = atan2f(delta_pos.y, delta_pos.x) + player_transform.angle_offset;
				for (uint i = 0; i < registry.attachments.size(); i++) {
					Entity att_entity = registry.attachments.entities[i];
					Attachment& att_comp = registry.attachments.components[i];
					if (att_comp.parent == player) {
						PhysicsSystem::update_attachment_orientation(att_entity, 1000.f);
					}
				}

				// Kill all small enemies when boss fight starts
				for (uint i = 0; i < registry.enemies.size(); i++) {
					Entity enemy_entity = registry.enemies.entities[i];
					Enemy& enemy_comp = registry.enemies.components[i];
					if (enemy_comp.type != ENEMY_ID::BOSS
						&& enemy_comp.type != ENEMY_ID::BOSS_ARM
						&& enemy_comp.type != ENEMY_ID::FRIENDBOSS
						&& enemy_comp.type != ENEMY_ID::FRIENDBOSSCLONE
						&& !registry.deathTimers.has(enemy_entity)) {
						startEntityDeath(enemy_entity);
					}
				}
				// Remove all bullets when boss fight starts
				while (registry.projectiles.entities.size() > 0) {
					registry.remove_all_components_of(registry.projectiles.entities.back());
				}
				if (registry.enemies.get(current_boss).type == ENEMY_ID::BOSS) {
					registry.motions.get(current_boss).max_velocity = BOSS_MAX_VELOCITY;
					for (uint i = 0; i < registry.attachments.size(); i++) {
						Attachment& att = registry.attachments.components[i];
						if (att.type == ATTACHMENT_ID::BACTERIOPHAGE_ARM) {
							registry.collidePlayers.emplace(registry.attachments.entities[i]);
						}
					}

					dialog_system->add_dialog(TEXTURE_ASSET_ID::PRE_BOSS_DIALOG1, 1000.f);
					dialog_system->add_camera_movement(player_pos, boss_pos, 1000.f);
					dialog_system->add_dialog(TEXTURE_ASSET_ID::PRE_BOSS_DIALOG2);
					dialog_system->add_dialog(TEXTURE_ASSET_ID::PRE_BOSS_DIALOG3);
					dialog_system->add_dialog(TEXTURE_ASSET_ID::PRE_BOSS_DIALOG4);
					dialog_system->add_camera_movement(boss_pos, player_pos, 1000.f);
				}
				else if (registry.enemies.get(current_boss).type == ENEMY_ID::FRIENDBOSS) {
					registry.motions.get(current_boss).max_velocity = FRIEND_BOSS_MAX_VELOCITY;

					dialog_system->add_dialog(TEXTURE_ASSET_ID::PRE_FRIEND_BOSS_DIALOG1, 1000.f);
					dialog_system->add_camera_movement(player_pos, boss_pos, 1000.f);
					dialog_system->add_dialog(TEXTURE_ASSET_ID::PRE_FRIEND_BOSS_DIALOG2);
					dialog_system->add_dialog(TEXTURE_ASSET_ID::PRE_FRIEND_BOSS_DIALOG3);
					dialog_system->add_dialog(TEXTURE_ASSET_ID::PRE_FRIEND_BOSS_DIALOG4);
					dialog_system->add_dialog(TEXTURE_ASSET_ID::PRE_FRIEND_BOSS_DIALOG5);
					dialog_system->add_camera_movement(boss_pos, player_pos, 1000.f);
				}
			}
		}
	}
}

// When chest is on-screen, spawn lots of enemies at once then dont spawn for x seconds
void WorldSystem::step_chests() {
	for (auto c : registry.chests.entities) {
		Chest& chest = registry.chests.get(c);
		if (!chest.waveActivated) {
			if (length(chest.position - registry.transforms.get(player).position) < SCREEN_RADIUS) {
				printf("activating\n");
				chest.waveActivated = true;

				individual_spawn_interval = 200.f;

				// first timer: spawn lots of enemies for a couple seconds
				TimedEvent& timer = registry.timedEvents.emplace(Entity());
				timer.timer_ms = 2700.f;
				timer.callback = [c, this]() {
					// first callback: this code will run when first timer expires
					individual_spawn_interval = 1000.f; // revert spawn change
					enemy_spawn_cooldown = 25000.f; // don't spawn for x seconds
					printf("set cooldown\n");

					// second timer: don't activate again for x seconds
					TimedEvent& timer = registry.timedEvents.emplace(Entity());
					timer.timer_ms = 25000.f;
					timer.callback = [c, this]() {
						// second callback: this code will run when second timer expires
						if (registry.chests.has(c)) {
							Chest& chest = registry.chests.get(c);
							printf("deactivating\n");

							chest.waveActivated = false;
						}
						};
					};
			}
		}
	}
}

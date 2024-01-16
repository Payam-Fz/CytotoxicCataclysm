#pragma once

// stlib
#include <fstream> // stdout, stderr..
#include <string>
#include <tuple>
#include <vector>

// glfw (OpenGL)
#define NOMINMAX
#include <gl3w.h>
#include <GLFW/glfw3.h>

// The glm library provides vector and matrix operations as in GLSL
#include <glm/vec2.hpp>				// vec2
#include <glm/ext/vector_int2.hpp>  // ivec2
#include <glm/vec3.hpp>             // vec3
#include <glm/mat3x3.hpp>           // mat3
using namespace glm;

#include "tiny_ecs.hpp"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

// Simple utility functions to avoid mistyping directory name
// audio_path("audio.ogg") -> data/audio/audio.ogg
// Get defintion of PROJECT_SOURCE_DIR from:
#include "../ext/project_path.hpp"
inline std::string data_path() { return std::string(PROJECT_SOURCE_DIR) + "data"; };
inline std::string shader_path(const std::string& name) {return std::string(PROJECT_SOURCE_DIR) + "/shaders/" + name;};
inline std::string textures_path(const std::string& name) {return data_path() + "/textures/" + std::string(name);};
inline std::string audio_path(const std::string& name) {return data_path() + "/audio/" + std::string(name);};
inline std::string mesh_path(const std::string& name) {return data_path() + "/meshes/" + std::string(name);};


#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// The 'Transform' component handles transformations passed to the Vertex shader
// (similar to the gl Immediate mode equivalent, e.g., glTranslate()...)
// We recommend making all components non-copyable by derving from ComponentNonCopyable
struct Transformation {
	mat3 mat = { { 1.f, 0.f, 0.f }, { 0.f, 1.f, 0.f}, { 0.f, 0.f, 1.f} }; // start with the identity
	void scale(vec2 scale);
	void rotate(float radians);
	void translate(vec2 offset);
};

bool gl_has_errors();

// Game configuration
const bool DEBUG_MODE = false;
const bool USE_FULLSCREEN = DEBUG_MODE ? false : true;
const bool SHOW_DIALOGS = DEBUG_MODE ? false : true;
const bool MOVE_ENEMIES = DEBUG_MODE ? false : true;

const int TARGET_REFRESH_RATE = 60;
// This is the "in-game" screen
const int CONTENT_WIDTH_PX = 1920;
const int CONTENT_HEIGHT_PX = 1080;
const float SCREEN_RADIUS = (float) hypot(CONTENT_WIDTH_PX, CONTENT_HEIGHT_PX) / 2; // Half of screen diagonal
const size_t NUM_REGIONS = 6;
const float MAP_RADIUS = 10000.f;
const float SPAWN_REGION_RADIUS = CONTENT_HEIGHT_PX;
const float EDGE_FADING_THICKNESS = 0.1;	// Radians
const float HEALTH_BAR_UPDATE_TIME_SLAP = 200.f;
const float DEATH_EFFECT_DURATION = 3000.f;
const float DEATH_EFFECT_DURATION_ENEMY = 250.f;
const float PLAYER_ATTACK_DELAY = 300.f;
const float PLAYER_SWORD_ATTACK_DELAY = 500.f;
const float PLAYER_DASH_DELAY = 800.f;
const float BOSS_MAX_VELOCITY = 250.f;
const float FRIEND_BOSS_MAX_VELOCITY = 350.f;
const float FRIEND_BOSS_DIFFICULTY = 1.0f;	// 1 : normal | <1 : easy | >1 : hard
const float play_animation_threshold = 40.0f;
const vec4 dashing_default_color = vec4(1, 100, 10, 1.f);
const vec4 no_color = vec4(0, 0, 0, 0);
const float ANGLE_PRECISION = 0.01f;

const int max_green_easyMode = 3;
const int max_red_easyMode = 5;
const int max_yellow_easyMode = 2;
const float enemy_health_reductionMultipler = 0.5f;
const float FRIEND_BOSS_DIFFICULTY_easy = 0.5f;	// 1 : normal | <1 : easy | >1 : hard


const int max_green_regularMode = 4;
const int max_red_regularMode = 8;
const int max_yellow_regularMode = 4;

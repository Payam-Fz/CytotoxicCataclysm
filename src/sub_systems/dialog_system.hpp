#pragma once

// internal
#include "common.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"

// stlib
#include <queue>

const float ACTION_DELAY = 3000.f;	// Time during which user is trying the action
const float SKIP_DELAY = 300.f;	// Time during which user is trying the action

enum class DIALOG_STATUS {
	DISPLAY = 0,
	AWAIT_ACTION = DISPLAY + 1,
	ACTION_TIMER = AWAIT_ACTION + 1,
	MOVING_CAMERA = ACTION_TIMER + 1
};


struct Stage {
	TEXTURE_ASSET_ID instruction_asset;
	float skip_delay_duration = SKIP_DELAY;
	vec2 instruction_position{ 0, 0 };
	// a lambda function which checks if required actions is performed
	// default action is to do left click
	std::function<bool()> is_action_performed = []() {
		throw std::runtime_error("Tutorial action not implemented.");
		return true;
		};
	bool camera_movement;
	vec2 start_pos;
	vec2 end_pos;
	float camera_duration;
	float camera_timer;
};

class DialogSystem {
public:
	DialogSystem(std::unordered_map<int, int>& keys_pressed, const vec2& mouse, const unsigned char*& controller_buttons);
	~DialogSystem();
	bool has_pending();
	bool step(float elapsed_ms);
	void add_dialog(TEXTURE_ASSET_ID asset, float skip_delay_duration = SKIP_DELAY);
	void add_camera_movement(vec2 start_pos, vec2 end_pos, float duration);
	void clear_pending_dialogs();

private:
	std::queue<Stage> dialogs;
	std::unordered_map<int, int>& keys_pressed;
	const vec2& mouse;
	const unsigned char*& controller_buttons;
	vec2 previous_mouse = { 0,0 };
	DIALOG_STATUS current_status;
	Entity rendered_entity;
	float skip_timer = SKIP_DELAY;
	bool prev_limit_fov = false;
};
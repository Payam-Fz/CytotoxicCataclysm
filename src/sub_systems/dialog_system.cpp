// internal
#include "dialog_system.hpp"
#include "world_init.hpp"

DialogSystem::DialogSystem(std::unordered_map<int, int>& keys_pressed, const vec2& mouse, const unsigned char*& controller_buttons)
	: keys_pressed(keys_pressed), mouse(mouse), controller_buttons(controller_buttons){
	rendered_entity = Entity();
	current_status = DIALOG_STATUS::DISPLAY;
}

// NOTE: A dialog mid-game should be given a higher delay to avoid accidental skips.
void DialogSystem::add_dialog(TEXTURE_ASSET_ID asset, float skip_delay_duration) {
	dialogs.push({
		asset, skip_delay_duration, {0,0},
		[this]() {
			if (this->skip_timer > 0) return false;
			auto keys = (std::unordered_map<int, int>&) this->keys_pressed;
			for (auto keyVal = keys.begin(); keyVal != keys.end(); ++keyVal) {
				if (keyVal->second) return true;
			}
			if (this->controller_buttons != nullptr) {
				for (uint i = 0; i < 19; i++) {
					if (this->controller_buttons[i]) return true;
				}
			}
			return false;
		},
		false});
}

void DialogSystem::add_camera_movement(vec2 start_pos, vec2 end_pos, float duration) {
	Stage new_cam_movement;
	new_cam_movement.camera_movement = true;
	new_cam_movement.start_pos = start_pos;
	new_cam_movement.end_pos = end_pos;
	new_cam_movement.camera_duration = duration;
	new_cam_movement.camera_timer = duration;
	dialogs.push(new_cam_movement);
}

DialogSystem::~DialogSystem() {
	registry.remove_all_components_of(rendered_entity);
};

bool DialogSystem::has_pending() {
	return !dialogs.empty();
}

void DialogSystem::clear_pending_dialogs() {
	registry.remove_all_components_of(rendered_entity);
	current_status = DIALOG_STATUS::DISPLAY;
	while(!dialogs.empty()) {
		dialogs.pop();
	}
}

// Returns true if the game should continue running and false if it should be paused
bool DialogSystem::step(float elapsed_ms) {
	if (has_pending() && SHOW_DIALOGS) {
		if (registry.screenStates.components[0].limit_fov) {
			prev_limit_fov = true;
			registry.screenStates.components[0].limit_fov = false;
		}
		Stage& current_stage = dialogs.front();
		if (current_stage.camera_movement) {
			current_status = DIALOG_STATUS::MOVING_CAMERA;
		}
		switch (current_status) {
		case (DIALOG_STATUS::DISPLAY): {
			Transform& transform = registry.transforms.emplace(rendered_entity);
			transform.position = current_stage.instruction_position;
			transform.scale = { DIALOG_TEXTURE_SIZE.x, DIALOG_TEXTURE_SIZE.y };
			transform.is_screen_coord = true;

			registry.renderRequests.insert(
				rendered_entity,
				{ dialogs.front().instruction_asset,
					EFFECT_ASSET_ID::TEXTURED,
					GEOMETRY_BUFFER_ID::SPRITE,
					RENDER_ORDER::DIALOG });

			keys_pressed.clear();
			skip_timer = current_stage.skip_delay_duration;
			current_status = DIALOG_STATUS::AWAIT_ACTION;
			return false;
			break;
		}
		case DIALOG_STATUS::AWAIT_ACTION:
			skip_timer = fmax(skip_timer - elapsed_ms, 0.f);
			if (current_stage.is_action_performed()) {
				registry.remove_all_components_of(rendered_entity);
				// UPDATED: Removed action timer based on feedback from users
				current_status = DIALOG_STATUS::DISPLAY;
				dialogs.pop();
				return !has_pending();
			}
			else {
				return false;
			}
			break;
		//case DIALOG_STATUS::ACTION_TIMER:
		//	current_stage.skip_delay_duration -= elapsed_ms;
		//	if (current_stage.skip_delay_duration <= 0) {
		//		current_status = DIALOG_STATUS::DISPLAY;
		//		dialogs.pop();
		//	}
		//	break;
		case DIALOG_STATUS::MOVING_CAMERA:
			current_stage.camera_timer -= elapsed_ms;
			vec2 current_pos = current_stage.start_pos * (max(0.f, current_stage.camera_timer) / current_stage.camera_duration) + 
							   current_stage.end_pos * (1.f - (max(0.f, current_stage.camera_timer) / current_stage.camera_duration));
			registry.camera.components[0].position = current_pos;
			if (current_stage.camera_timer < 0.f) {
				current_status = DIALOG_STATUS::DISPLAY;
				dialogs.pop();
				return !has_pending();
			} else {
				return false;
			}
			break;
		default:
			break;
		}
		previous_mouse = mouse;
	}
	if (prev_limit_fov) {
		prev_limit_fov = false;
		registry.screenStates.components[0].limit_fov = true;
	}
	return true;
}

#pragma once

#include <array>
#include <utility>

#include "common.hpp"
#include "components.hpp"
#include "tiny_ecs.hpp"

// System responsible for setting up OpenGL and for rendering all the
// visual entities in the game
class RenderSystem {
	/**
	 * The following arrays store the assets the game will use. They are loaded
	 * at initialization and are assumed to not be modified by the render loop.
	 *
	 * Whenever possible, add to these lists instead of creating dynamic state
	 * it is easier to debug and faster to execute for the computer.
	 */
	std::array<GLuint, texture_count> texture_gl_handles;
	std::array<ivec2, texture_count> texture_dimensions;

	// Make sure these paths remain in sync with the associated enumerators.
	// Associated id with .obj path
	const std::vector < std::pair<GEOMETRY_BUFFER_ID, std::string>> mesh_paths =
	{
		  std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::BACTERIOPHAGE, mesh_path("bacteriophage.obj")),
		  std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::BACTERIOPHAGE_ARM, mesh_path("bacteriophage_arm.obj"))
	};

	const std::vector < std::pair<GEOMETRY_BUFFER_ID, std::string>> mesh_paths_color_vector =
	{
		  std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::SWORD, mesh_path("sword.obj"))
	};

	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, texture_count> texture_paths = {
		
		// Game Objects
		textures_path("empty.png"),
		textures_path("immunity.png"),
		textures_path("red_enemy.png"),
		textures_path("green_enemy.png"),
		textures_path("yellow_enemy.png"),
		textures_path("healthbarframe.png"),
		textures_path("healthbar_boss.png"),
		textures_path("healthbar_boost.png"),
		textures_path("chest.png"),
		textures_path("cure.PNG"),
		textures_path("gun.png"),
		textures_path("cyst.png"),
		textures_path("immunity_moving.png"),
		textures_path("immunity_dying.png"),
		textures_path("green_enemy_moving.png"),
		textures_path("green_enemy_dying.png"),
		textures_path("immunity_blink.png"),
		textures_path("cyst_animation.png"),
		textures_path("bacteriophage.png"),
		textures_path("bacteriophage_arm.png"),
		textures_path("dashing.png"),
		textures_path("friend.png"),

		// Background
		textures_path("background/nervous_bg.png"),
		textures_path("background/respiratory_bg.png"),
		textures_path("background/urinary_bg.png"),
		textures_path("background/muscular_bg.png"),
		textures_path("background/skeletal_bg.png"),
		textures_path("background/cutaneous_bg.png"),

		// Dialog
		textures_path("dialog/dialog1_this-is.png"),
		textures_path("dialog/dialog2_until-one-day.png"),
		textures_path("dialog/dialog3_im-gonna.png"),
		textures_path("dialog/pre_boss_dialog1.png"),
		textures_path("dialog/pre_boss_dialog2.png"),
		textures_path("dialog/pre_boss_dialog3.png"),
		textures_path("dialog/pre_boss_dialog4.png"),
		textures_path("dialog/post_boss_dialog.png"),
		textures_path("dialog/pre_friend_boss_dialog1.png"),
		textures_path("dialog/pre_friend_boss_dialog2.png"),
		textures_path("dialog/pre_friend_boss_dialog3.png"),
		textures_path("dialog/pre_friend_boss_dialog4.png"),
		textures_path("dialog/pre_friend_boss_dialog5.png"),
		textures_path("dialog/post_friend_boss_dialog.png"),

		// Tutorial
		textures_path("tutorial/controls_keyboard.png"),
		textures_path("tutorial/controls_controller.png"),
		textures_path("tutorial/waypoints.png"),
		textures_path("tutorial/cysts-burst.png"),
		textures_path("tutorial/unlock-bullet-boost.png"),
		textures_path("tutorial/unlock-health-boost.png"),
		textures_path("tutorial/unlock-dashing.png"),
		textures_path("tutorial/unlock-dashing_keyboard.png"),
		textures_path("tutorial/unlock-dashing_controller.png"),
		textures_path("tutorial/unlock-sword.png"),
		textures_path("tutorial/unlock-melee_mouse.png"),
		textures_path("tutorial/unlock-melee_controller.png"),
		textures_path("tutorial/start.png"),
		textures_path("ui/death_screen_1.png"),
		textures_path("ui/death_screen_2.png"),

		// UI
		textures_path("ui/icon_damage.png"),
		textures_path("ui/icon_slow.png"),
		textures_path("ui/icon_fov.png"),
		textures_path("ui/icon_ammo.png"),
		textures_path("ui/start_menu_title.png"),
		textures_path("ui/pause_menu_title.png"),
		textures_path("ui/start_button.png"),
		textures_path("ui/load_button.png"),
		textures_path("ui/exit_button.png"),
		textures_path("ui/resume_button.png"),
		textures_path("ui/save_button.png"),
		textures_path("ui/mute_button.png"),
		textures_path("ui/unmute_button.png"),
		textures_path("ui/crosshair.png"),
		textures_path("ui/skull.png"),
		textures_path("ui/question.png"),
		textures_path("ui/easyMode_selected.png"),
		textures_path("ui/regularMode_selected.png"),
		textures_path("ui/easyMode_unselected.png"),
		textures_path("ui/regularMode_unselected.png"),
		textures_path("ui/credits.png"),
		textures_path("ui/hold-space.png"),
		textures_path("ui/hold-o.png")
	};

	std::array<GLuint, effect_count> effects;
	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, effect_count> effect_paths = {
		shader_path("coloured"),
		shader_path("textured"),
		shader_path("screen"),
		shader_path("region"),
	};

	std::array<GLuint, geometry_count> vertex_buffers;
	std::array<GLuint, geometry_count> index_buffers;
	std::array<Mesh, geometry_count> meshes;

public:
	// Initialize the window
	bool init(GLFWwindow* window);

	template <class T>
	void bindVBOandIBO(GEOMETRY_BUFFER_ID gid, std::vector<T> vertices, std::vector<uint16_t> indices);

	void initializeGlTextures();

	void initializeGlEffects();

	void initializeGlMeshes();
	Mesh& getMesh(GEOMETRY_BUFFER_ID id) { return meshes[(int)id]; };

	void initializeGlGeometryBuffers();
	// Initialize the screen texture used as intermediate render target
	// The draw loop first renders to this texture, then it is used for the screen
	// shader
	bool initScreenTexture();

	// Destroy resources associated to one or all entities created by the system
	~RenderSystem();

	// Draw all entities
	void draw();

	mat3 createProjectionMatrix();

	mat3 createViewMatrix();
	vec2 offset;

	bool is_outside_screen(vec2 entityPos);


	//animation system
	void initAnimation(GEOMETRY_BUFFER_ID gid, ANIMATION_FRAME_COUNT fcount);
	void animationSys_step(float elapsed_ms);
	void animationSys_init();
	static void animationSys_switchAnimation(Entity& entity, 
		ANIMATION_FRAME_COUNT animationType, 
		int update_period_ms);
	void initAnimation_dashing();
	
private:
	Entity player; // Keep reference to player entity

	// Internal drawing functions for each entity type
	void drawEntity(
		Entity entity,
		const RenderRequest& render_request,
		const mat3& transform,
		const mat3& viewProjection
	);
	// void drawBackground(const mat3& viewProjection);
	void drawToScreen();
	void setUniformShaderVars(
		Entity entity,
		const mat3& transform,
		const mat3& viewProjection
	);
	void setTexturedShaderVars(Entity entity);
	void setColouredShaderVars(Entity entity);
	void setRegionShaderVars(Entity entity);

	// Window handle
	GLFWwindow* window;

	// Screen texture handles
	GLuint frame_buffer;
	GLuint off_screen_render_buffer_color;
	GLuint off_screen_render_buffer_depth;

	Entity screen_state_entity;
	
};


bool loadEffectFromFile(
	const std::string& vs_path, const std::string& fs_path, GLuint& out_program);

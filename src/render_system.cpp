#include <algorithm>    // std::sort

// internal
#include "render_system.hpp"
#include "common.hpp"
#include <SDL.h>
#include "tiny_ecs_registry.hpp"

void RenderSystem::setUniformShaderVars(
	Entity entity,
	const mat3& transform,
	const mat3& viewProjection
) {
	GLint program;
	glGetIntegerv(GL_CURRENT_PROGRAM, &program);

	// Getting variable locations
	GLuint transform_loc = glGetUniformLocation(program, "transform");
	GLuint viewProjection_loc = glGetUniformLocation(program, "viewProjection");
	GLint color_uloc = glGetUniformLocation(program, "fcolor");
	gl_has_errors();

	// Setting values to the currently bound program
	const vec4 color = registry.colors.has(entity) ? registry.colors.get(entity) : vec4(1);
	glUniform4fv(color_uloc, 1, (float*)&color);
	glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float*)&transform);
	glUniformMatrix3fv(viewProjection_loc, 1, GL_FALSE, (float*)&viewProjection);
	gl_has_errors();
}

void RenderSystem::setTexturedShaderVars(Entity entity) {
	GLint program;
	glGetIntegerv(GL_CURRENT_PROGRAM, &program);

	// Getting variable locations
	GLint in_position_loc = glGetAttribLocation(program, "in_position");
	GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
	gl_has_errors();
	assert(in_texcoord_loc >= 0);

	// Setting values to the currently bound program
	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
		sizeof(TexturedVertex), (void*)0);
	glEnableVertexAttribArray(in_texcoord_loc);
	glVertexAttribPointer(
		in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
		(void*)sizeof(vec3)); // note the stride to skip the preceeding vertex position
	gl_has_errors();

	// Enabling and binding texture to slot 0
	glActiveTexture(GL_TEXTURE0);
	gl_has_errors();

	assert(registry.renderRequests.has(entity));
	GLuint texture_id =
		texture_gl_handles[(GLuint)registry.renderRequests.get(entity).used_texture];
	glBindTexture(GL_TEXTURE_2D, texture_id);
	GLint isAnimation_loc = glGetUniformLocation(program, "isAnimation");
	if (registry.animations.has(entity)) {
		glUniform1f(isAnimation_loc, 1.f);
		GLint curr_frame_loc = glGetUniformLocation(program, "currFrame");
		glUniform1f(curr_frame_loc, 1.f * registry.animations.get(entity).curr_frame);
		GLint total_frame_loc = glGetUniformLocation(program, "totalFrames");
		glUniform1f(total_frame_loc, 1.f * registry.animations.get(entity).total_frame);
	}
	else {
		glUniform1f(isAnimation_loc, -1.f);
	}

	GLint highlight_loc = glGetUniformLocation(program, "highlight");
	if (registry.menuButtons.has(entity) && registry.menuButtons.get(entity).highlight) {
		glUniform1i(highlight_loc, 1);
	} else {
		glUniform1i(highlight_loc, 0);
	}

	gl_has_errors();
}

void RenderSystem::setColouredShaderVars(Entity entity) {
	(void)entity; // silence unused warning for now
	GLint program;
	glGetIntegerv(GL_CURRENT_PROGRAM, &program);

	// Getting variable locations
	GLint in_position_loc = glGetAttribLocation(program, "in_position");
	GLint in_color_loc = glGetAttribLocation(program, "in_color");
	gl_has_errors();

	// Setting values to the currently bound program
	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
		sizeof(ColoredVertex), (void*)0);
	glEnableVertexAttribArray(in_color_loc);
	glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE,
		sizeof(ColoredVertex), (void*)sizeof(vec3));
	gl_has_errors();
}

void RenderSystem::setRegionShaderVars(Entity entity) {
	(void)entity; // silence unused warning for now
	GLint program;
	glGetIntegerv(GL_CURRENT_PROGRAM, &program);

	// Getting variable locations
	GLuint mapRadius_loc = glGetUniformLocation(program, "mapRadius");
	GLuint spawnRadius_loc = glGetUniformLocation(program, "spawnRadius");
	GLuint edgeThickness_loc = glGetUniformLocation(program, "edgeThickness");
	GLuint regionAngle_loc = glGetUniformLocation(program, "regionAngle");
	gl_has_errors();

	// Setting values to the currently bound program
	glUniform1f(mapRadius_loc, MAP_RADIUS);
	glUniform1f(spawnRadius_loc, SPAWN_REGION_RADIUS);
	glUniform1f(edgeThickness_loc, EDGE_FADING_THICKNESS);
	glUniform1f(regionAngle_loc, 2 * M_PI / NUM_REGIONS);
	gl_has_errors();
}

void RenderSystem::drawEntity(
	Entity entity,
	const RenderRequest& render_request,
	const mat3& transform,
	const mat3& viewProjection
) {
	// Setting shader program
	const GLuint used_effect_enum = (GLuint)render_request.used_effect;
	assert(used_effect_enum != (GLuint)EFFECT_ASSET_ID::EFFECT_COUNT);
	const GLuint program = (GLuint)effects[used_effect_enum];
	glUseProgram(program);
	gl_has_errors();

	// Setting vertex and index buffers
	assert(render_request.used_geometry != GEOMETRY_BUFFER_ID::GEOMETRY_COUNT);
	const GLuint vbo = vertex_buffers[(GLuint)render_request.used_geometry];
	const GLuint ibo = index_buffers[(GLuint)render_request.used_geometry];
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();

	// Set shader input data
	setUniformShaderVars(entity, transform, viewProjection);

	switch (render_request.used_effect) {
	case EFFECT_ASSET_ID::TEXTURED:
		setTexturedShaderVars(entity);
		break;
	case EFFECT_ASSET_ID::REGION:
		setTexturedShaderVars(entity);
		setRegionShaderVars(entity);
		break;
	case EFFECT_ASSET_ID::COLOURED:
		setColouredShaderVars(entity);
		break;	// Variables already set in setUniformShaderVars
	default:
		assert(false && "Type of render request not supported");
	}

	// Drawing of num_indices/3 triangles specified in the index buffer
	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();
	GLsizei num_indices = size / sizeof(uint16_t);
	// GLsizei num_triangles = num_indices / 3;
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
	gl_has_errors();
}

// draw the intermediate texture to the screen
void RenderSystem::drawToScreen()
{
	// Setting shaders
	// get the screen texture, sprite mesh, and program
	glUseProgram(effects[(GLuint)EFFECT_ASSET_ID::SCREEN]);
	gl_has_errors();
	// Clearing backbuffer
	int w, h;
	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, w, h);
	glDepthRange(0, 10);
	glClearColor(0, 0, 0, 1);	// black default background
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl_has_errors();
	// Enabling alpha channel for textures
	glDisable(GL_BLEND);
	// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	// Draw the screen texture on the quad geometry
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]);
	glBindBuffer(
		GL_ELEMENT_ARRAY_BUFFER,
		index_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]); // Note, GL_ELEMENT_ARRAY_BUFFER associates
	// indices to the bound GL_ARRAY_BUFFER
	gl_has_errors();
	const GLuint screen_program = effects[(GLuint)EFFECT_ASSET_ID::SCREEN];
	// Set clock
	GLuint time_uloc = glGetUniformLocation(screen_program, "time");
	GLuint dead_timer_uloc = glGetUniformLocation(screen_program, "screen_darken_factor");
	glUniform1f(time_uloc, (float)(glfwGetTime() * 10.0f));
	ScreenState& screen = registry.screenStates.get(screen_state_entity);
	glUniform1f(dead_timer_uloc, screen.screen_darken_factor);
	// set fov
	GLint limit_fov = glGetUniformLocation(screen_program, "is_fov_limited");
	glUniform1i(limit_fov, screen.limit_fov);
	gl_has_errors();
	// Set the vertex position and vertex texture coordinates (both stored in the
	// same VBO)
	GLint in_position_loc = glGetAttribLocation(screen_program, "in_position");
	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
	gl_has_errors();

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, off_screen_render_buffer_color);
	gl_has_errors();
	// Draw
	glDrawElements(
		GL_TRIANGLES, 3, GL_UNSIGNED_SHORT,
		nullptr); // one triangle = 3 vertices; nullptr indicates that there is
	// no offset from the bound index buffer
	gl_has_errors();

}

// helper function for view frustum culling
bool RenderSystem::is_outside_screen(vec2 entityPos) {
	assert(registry.camera.size() == 1);
	vec2 camPos = registry.camera.components[0].position;
	// check a radius a bit bigger than screen so we don't cull big objects too soon
	return length(entityPos - camPos) > SCREEN_RADIUS * 1.2;
}

// Render our game world
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
void RenderSystem::draw()
{
	// Getting size of window
	int w, h;
	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays

	// First render to the custom framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	gl_has_errors();
	// Clearing backbuffer
	glViewport(0, 0, w, h);
	glDepthRange(0.00001, 10);
	glClearColor(0, 0, 0, 1);	// black default background
	glClearDepth(10.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST); // native OpenGL does not work with a depth buffer
	// and alpha blending, one would have to sort
	// sprites back to front
	gl_has_errors();
	mat3 projection_2D = createProjectionMatrix();
	mat3 view = createViewMatrix();
	mat3 viewProjection = projection_2D * view;

	// update player entity (it might change on death)
	auto players = registry.players.entities;
	if (!players.empty()) {
		player = players.back();
	}

	// Draw entities based on their renderRequest order
	/*
	 * Payam: This code is O(4N) instead of original O(N) but uses good cache locality.
	 * I could not think of a more efficient way to render entities in order.
	 */
	for (uint order = 0; order < render_order_count; order++) {
		for (uint i = 0; i < registry.renderRequests.components.size(); i++) {
			if ((uint)registry.renderRequests.components[i].order == order) {
				Entity entity = registry.renderRequests.entities[i];
				RenderRequest& render_request = registry.renderRequests.components[i];
				if (registry.transforms.has(entity)) {
					Transform& transform = registry.transforms.get(entity);

					// View frustum culling; ie. cull entities before vertex shader
					// exclude on-screen entities, regions, and UI elements from culling
					if (!registry.regions.has(entity) && !transform.is_screen_coord && is_outside_screen(transform.position)) {
						continue;
					}

					// Transformation
					Transformation transformation;
					transformation.translate(transform.position);
					transformation.rotate(transform.angle);
					transformation.scale(transform.scale);
					// Note, its not very efficient to access elements indirectly via the entity
					// albeit iterating through all Sprites in sequence. A good point to optimize

					if (transform.is_screen_coord) {
						drawEntity(entity, render_request, transformation.mat, projection_2D);
					}
					else {
						drawEntity(entity, render_request, transformation.mat, viewProjection);
					}
				}
			}
		}
	}

	// Truely render to the screen
	drawToScreen();

	// flicker-free display with a double buffer
	glfwSwapBuffers(window);
	gl_has_errors();
}

// Orthographic Projection
// basically converts Viewing Coordinate System -> Device Coordinate System
mat3 RenderSystem::createProjectionMatrix()
{
	// Fake projection matrix, scales with respect to window coordinates
	gl_has_errors();
	Transformation projection;
	projection.scale({ 2.f / CONTENT_WIDTH_PX, 2.f / CONTENT_HEIGHT_PX });
	return projection.mat;
}

mat3 RenderSystem::createViewMatrix()
{
	offset = vec2(0.f, 0.f);
	
	assert(registry.camera.size() == 1);
	vec2 cameraPos = registry.camera.components[0].position;
	Transformation view;
	view.translate(-cameraPos);
	return view.mat;
}

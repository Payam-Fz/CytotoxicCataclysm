
#define GL3W_IMPLEMENTATION
#include <gl3w.h>

// stlib
#include <chrono>

// internal
#include "physics_system.hpp"
#include "render_system.hpp"
#include "world_system.hpp"
#include "ai_system.hpp"

using Clock = std::chrono::high_resolution_clock;

void reset_forces() {
	for (Motion& m : registry.motions.components) {
		m.force = { 0.f, 0.f };
	}
}

// Entry point
int main()
{
	// Global systems
	WorldSystem world_system;
	RenderSystem render_system;
	PhysicsSystem physics_system;
	AISystem ai_system;

	// Initializing window
	GLFWwindow* window = world_system.create_window();
	if (!window) {
		// Time to read the error message
		printf("Press any key to exit");
		getchar();
		return EXIT_FAILURE;
	}
	glfwSetWindowTitle(window, "Cytotoxic Cataclysm");

	// initialize the main systems
	render_system.init(window);
	world_system.init(&render_system);
	render_system.animationSys_init();

	// variable timestep loop
	auto t = Clock::now();
	while (!world_system.is_over()) {
		// Processes system messages, if this wasn't present the window would become unresponsive
		glfwPollEvents();

		// Calculating elapsed times in milliseconds from the previous iteration
		auto now = Clock::now();
		float elapsed_ms =
			(float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;
		t = now;

		reset_forces();
		bool isRunning = world_system.step(elapsed_ms);
		if (isRunning) {
			ai_system.step(elapsed_ms);
			physics_system.step(elapsed_ms);
			world_system.resolve_collisions();
			render_system.animationSys_step(elapsed_ms);
			world_system.update_camera(elapsed_ms);
		}
		
		render_system.draw();
	}

	// Debugging for memory/component leaks
	printf("\n=========================\n|\tEnding\t\t|\n=========================\n");
	registry.list_all_components();

	return EXIT_SUCCESS;
}

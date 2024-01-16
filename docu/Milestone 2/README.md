# Cytotoxic Cataclysm
**Video Game Programming Term Project**

## Team 3
- Keigo Lee
- Nick Zhang
- Nitahi Escolar Bach
- Payam Forouzandeh
- Ritik Keswani
- Shirley Du

## Implemented Features
Note: By using the ticket number from the [<u>Task
Board</u>](https://github.students.cs.ubc.ca/orgs/CPSC427-2023W-T1/projects/3/views/1),
the linked branch of the task can be retrieved.
<table>
	<colgroup>
		<col style="width: 15%" />
		<col style="width: 25%" />
		<col style="width: 59%" />
	</colgroup>
	<thead>
		<tr class="header">
			<th><strong>Category</strong></th>
			<th><strong>Task</strong></th>
			<th><strong>Details</strong></th>
		</tr>
		<tr>
			<th rowspan="5">Improved Gameplay</th>
			<th>
			<p>Game logic response to user input</p>
			</th>
			<th><ul>
			<li>
			<p>Ticket #41, #33</p>
			</li>
			<li>
			<p>Enemies chase player (AISystem::move_enemies) and deals damage to player when they collide (WorldSystem::resolve_collisions)</p>
			<li>
			<p>Enemies are spawned offscreen with a frequency based on the player’s proximity to point of interest (point with game objectives). Handled by WorldSystem::step_enemySpawn</p>
			</li>
			<li>
			<p>Player can shoot bullets to enemies and dash (WorldSystem::control_action()). </p>
			</li></li>
			<li>
			<p>When player shoots enemies enough they die</p>
			</th>
		</tr>
		<tr>
			<th>
			<p>Sprite sheet animation </p>
			</th>
			<th><ul>
			<li>
			<p>Ticket #32</p>
			</li>
			<li>
			<p>Created a generalized interface for playing sprite sheet animations (functions in animation_system.cpp)</p>
			</li>
			<li>
			<p>Created assets for several animations: player/enemy moving, player blinking, and player/enemy death. Textures for these are located in data/textures</p>
			</li>
			<li>
			<p>Integrated the above animations into the game (handled in PhysicsSystem::step_movement and WorldSystem::step_health) </p>
			</li>
			</th>
		</tr>
		<tr>
			<th>
			<p>New integrated assets</p>
			</th>
			<th><ul>
			<li>
			<p>Ticket #34</p>
			</li>
			<li>
			<p>Added background music to the game</p>
			</li>
			<li>
			<p>Handled in WorldSystem::create_window()
</p>
			</li>
			<li>
			<p>Ticket #46</p>
			</li>
			<li>
			<p>Added boss bacteriophage mesh created in Blender. Located in ‘data/meshes/
</p>
			</li>
			</ul>
			</th>
		</tr>
		<tr>
			<th>
			<p>Mesh-based collision</p>
			</th>
			<th><ul>
			<li>
			<p>Ticket #46
</p>
			</li>
			<li>
			<p>Mesh::loadFromObjFile() loads a mesh with uv coordinates stored in obj file format into the game.</p>
			</li>
			<li>
			<p>createBoss() in world_init.cpp creates a boss entity with the mesh and corresponding texture. The mesh and uv coordinates are passed to textured shaders to draw the boss</p>
			</li>
			<li>
			<p>In physics_system.cpp, collides_with_mesh() function checks if a mesh collides with circles.</p>
			</li>
			</ul>
			</th>
		</tr>
		<tr>
			<th>
			<p>Basic tutorial/help</p>
			</th>
			<th><ul>
			<li>
			<p>Ticket #38</p>
			</li>
			<li>
			<p>Added a dialog sub-system to handle any dialogs or interactive tutorials. Class definition in sub_systems/dialog_system.hpp</p>
			</li>
			<li>
			<p>Added cutscenes at the beginning of the game to tell the story
</p>
			</li>
			<li>
			<p>Added interactive instructions after the cutscenes to guide the player</p>
			</li>
			<li>
			<p>Created assets for the cutscenes, visual guidance and texts</p>
			</li>
			</ul>
			</th>
		</tr>
		<tr>
			<th rowspan="1">Playability</th>
			<th>
			<p>2 minutes of non-repetitive gameplay</p>
			</th>
			<th><ul>
			<li>
			<p>This is a collaborative result from completing all other features
</p>
			</li>
			<li>
			<p>The player will encounter two different enemy types over four regions before finding and fighting the boss</p>
			</li>
			</ul>
			</th>
		</tr>
		<tr>
			<th rowspan="3">Stability</th>
			<th>
			<p>Minimal lag</p>
			</th>
			<th><ul>
			<li>
			<p>Runtimes of each step of the subsystems are measured to identify the performance bottleneck</p>
			</li>
			<li>
			<p>Potential performance issues are optimized to avoid frame loss</p>
			</li>
			</ul>
			</th>
		</tr>
		<tr class="odd">
			<th>
			<p>Consistent game resolution</p>
			</th>
			<th><ul>
			<li>
			<p>Ticket #40</p>
			</li>
			<li>
			<p>Made the game run in fullscreen mode with 1080p resolution and 60Hz frame rate by default to make sure that the game resolution looks consistent across different machines.</p>
			</li>
			<li>
			<p>16:9 aspect ratio is enforced with letterboxing on different screen dimensions while maintaining a consistent FOV and scale</p>
			</li>
			<li>
			<p>Handled in WorldSystem::create_window()</p>
			</li>
			</ul>
			</th>
		</tr>
		<tr class="odd">
			<th>
			<p>No crashes, glitches, unpredictable behaviour</p>
			</th>
			<th><ul>
			<li>
			<p>Done by reviewing each others pull requests</p>
			</li>
			</ul>
			</th>
		</tr>
		<tr>
			<th rowspan="2">Creative</th>
			<th>[Creative][basic][Basic integrated assets][24]:</th>
			<th><ul>
			<li>
			<p>Ticket #5, #46, #38, #81 etc.</p>
			</li>
			<li>
			<p>Created background textures, cutscenes, player, enemy and boss textures, and fully integrated these textures into the game</p>
			</li>
			<li>
			<p>All textures are stored under data/textures folder. RenderSystem::initializeGlTextures() loads these textures and they are mapped to different geometries in the game to be used for different purposes</p>
			</li>
			</ul>
			</th>
		</tr>
		<tr>
			<th>[Creative][basic][23][Audio feedback]</th>
			<th><ul>
			<li>
			<p>Ticket #65</p>
			</li>
			<li>
			<p>Added sound effects for various interactions in the game (e.g. player shoot or enemies get hit) to provide audio feedback</p>
			</li>
			<li>
			<p>Handled by WorldSystem::create_window() and WorldSystem::handle_shooting_sound_effect()</p>
			</li>
			</ul>
			</th>
		</tr>
	</thead>
</table>

## Alignment with development plan
We had the following discrepancies in Milestone 2 compared to the development plan in the original proposal:
* “Complex geometry” (advanced creative feature) is removed because it now overlaps with the new mandatory mesh-based collision requirement. After confirming with TA, this feature will not count.
* “Audio feedback” (basic creative feature) is pulled from Milestone 4 into Milestone 2 to compensate for complex geometry.
* “Basic integrated assets” (basic creative feature) is revisited and completed in Milestone 2 also to compensate for complex geometry
* “Start and pause menus” is pushed to Milestone 3 as it is not required for Milestone 2.

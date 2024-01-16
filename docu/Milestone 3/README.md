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
			<th rowspan="3">Creative</th>
			<th>
			<p>Reloadability</p>
			</th>
			<th><ul>
			<li>
			<p>Ticket #91 & #43</p>
			</li>
			<li>
			<p>Save and load necessary game states and components, including map regions, player state, enemies and cysts (handled by WorldSystem::save_game() and WorldSystem::load_game(), etc.)</p>
			<li>
			<p>Implement start, save, load, exit, mute/unmute sound in the menus (handled by WorldSystem::step_menu() and functions in menu_system.cpp)</p>
			</li>
			</th>
		</tr>
		<tr>
			<th>
			<p>[1] Simple rendering effects </p>
			</th>
			<th><ul>
			<li>
			<p>Ticket #48, #43, #131, #19</p>
			</li>
			<li>
			<p>Player flashes when damaged (handled in WorldSystem::step_invincibility())</p>
			</li>
			<li>
			<p>Highlight menu buttons when mouse cursor hovers on them  (shaders/textured.fs.glsl)</p>
			</li>
			<li>
			<p>Vignette effect for limit fov cyst effect  (shaders/screen.fs.glsl)</p>
			</li>
			<li>
			<p>Region borders (shaders/region.fs.glsl)</p>
			</li>
			</th>
		</tr>
		<tr>
			<th>
			<p>[10] Precise collisions</p>
			</th>
			<th><ul>
			<li>
			<p>Ticket #93 & #94</p>
			</li>
			<li>
			<p>Implement complex geometry of the sword (data/meshes/sword.obj)</p>
			</li>
			<li>
			<p>The Sword collision with the boss (mesh-mesh collision, handled by PhysicsSystem::collides_mesh_with_mesh())</p>
			</li>
			<li>
			<p>Optimization: retrieve bounding boxes for every moving object. Only proceed with the circle-mesh or mesh-mesh collision if their bounding boxes collide</p>
			</li>
			<li>
			<p>Handle sword collision with other enemies and cysts as well</p>
			</li>
			<li>
			<p>References: https://freesvg.org/tachi; https://developer.mozilla.org/en-US/docs/Games/Techniques/3D_collision_detection; https://stackoverflow.com/questions/3838329/how-can-i-check-if-two-segments-intersect.</p>
			</li>
			</ul>
			</th>
		</tr>
		<tr>
			<th rowspan="1">Playability</th>
			<th>
			<p>5 minutes of non-repetitive gameplay</p>
			</th>
			<th><ul>
			<li>
			<p>This is a collaborative result from completing all other features</p>
			</li>
			<li>
			<p>Linked Pull Request #133: second boss</p>
			</li>
			<li>
			<p>Linked Pull Request #131: Cyst Effects</p>
			</li>
			<li>
			<p>Linked Pull Request #114: Cysts WIP and View Frustum Culling</p>
			</li>
			<li>
			<p>Linked Pull Request #113: yellow enemy shooting</p>
			</li>
			</ul>
			</th>
		</tr>
		<tr>
			<th rowspan="3">Stability</th>
			<th>
			<p>Prior missed milestone features & bug fixes</p>
			</th>
			<th><ul>
			<li>
			<p>Linked Pull Request #124: Player collision with boundary causes game to crash</p>
			</li>
			<li>
			<p>Linked Pull Request # 129:  sprite order and flash; yellow enemy crash</p>
			</li>
			<li>
			<p>Linked Pull Request #107: fix all warnings</p>
			</li>
			<li>
			<p>Linked Pull Request #137: Fix enemy bullet collide with cyst</p>
			</li>
			<li>
			<p>Linked Pull Request #128: player collision boundary crash fix</p>
			</li>
			<li>
			<p>Linked Pull Request #143: consistent game resolution</p>
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
			<p>Linked Pull Request #143: consistent game resolution</p>
			</li>
		</tr>
		<tr class="odd">
			<th>
			<p>No crashes, glitches, unpredictable behaviour</p>
			</th>
			<th><ul>
			<li>
			<p>This is a team effort from completing all tasks carefully and reviewing each other's pull requests</p>
			</li>
			</ul>
			</th>
		</tr>
		<tr>
			<th rowspan="3">Robustness</th>
			<th>Memory management</th>
			<th><ul>
			<li>
			<p>Ticket #100</p>
			</li>
			<li>
			<p>In the Debug window , the six types of components(8 Transform components for the UI elements, 8 RenderRequest for the UI elements, 1 ScreenState of the game, 1 colour component, 6 Regions, 1 Camera) that are printed for debugging/memory leak checking are expected.</p>
			</li>
			</ul>
			</th>
		</tr>
		<tr>
			<th>Handle all user input</th>
			<th><ul>
			<li>
			<p>Simultanoues user inputs from keyboard and mouse are all handled properly</p>
			</li>
			</ul>
			</th>
		</tr>
		<tr>
			<th>Real-time gameplay</th>
			<th><ul>
			<li>
			<p>Objects out of the screen are factored out from collision detection (Pull request #112)</p>
			</li>
			<li>
			<p>View frustum culling (Pull request #114)</p>
			</li>
			</ul>
			</th>
		</tr>
	</thead>
</table>

## Alignment with development plan
We had the following discrepancies in Milestone 3 compared to the development plan in the original proposal:
* [Creative][advanced][13][Physics-based animation]: We changed the task from “Physics-based animation for cyst breaking” to “Articulated Motion for boss arms” as it introduced more interesting gameplay. However, since it was not implemented with Inverse Kinematics, we decided to move it to Milestone 4 to possibly re-implement it with Inverse Kinematics.
* [Creative][advanced/basic][27][Story elements]: Since all pieces of the game come together in MS4, we decided to use this feature once we have all the dialogs.
* [creative][advanced][1][Simple rendering effects]: More rendering effects have been added in this milestone in addition to what has been implemented in previous milestones, so this feature is included in this milestone for grading
* (Not for marks)[Section interest points]: A few of the power-ups were implemented but the textures were not ready. So we decided to move to Milestone 4.

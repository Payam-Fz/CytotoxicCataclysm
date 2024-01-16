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
			<th><strong>Details (optional)</strong></th>
		</tr>
		<tr>
			<th rowspan="3">Rendering</th>
			<th>
			<p>Textured geometry</p>
			</th>
			<th><ul>
			<li>
			<p>Ticket #7</p>
			</li>
			<li>
			<p>Art part:</p>
			<ul>
			<li>
			<p>Come up with a few prototypes of a regular enemy and take feedback
			from everyone.</p>
			</li>
			<li>
			<p>Draw them in Krita while keeping the resolution and size consistent
			with the main character. (use separate layer in the All.kra file)</p>
			</li>
			</ul></li>
			<li>
			<p>Code part:</p>
			<ul>
			<li>
			<p>Load the sprite in the game.</p>
			</li>
			<li>
			<p>Add an enemy entity.</p>
			</li>
			<li>
			<p>Register enemy's basic required components (including the sprite) in
			the ECS system.</p>
			</li>
			<li>
			<p>Add one enemy to the main scene.</p>
			</li>
			</ul></li>
			</ul>
			</th>
		</tr>
		<tr>
			<th>
			<p>Basic 2D transformations</p>
			</th>
			<th><ul>
			<li>
			<p>Rotation is handled by <a
			href="https://github.students.cs.ubc.ca/CPSC427-2023W-T1/Team03CytotoxicCataclysm/blob/9bdbff0afb2642b8617121bd22d0e65af641abb6/src/world_system.cpp#L354-L363"><u>this</u></a></p>
			</li>
			<li>
			<p>Translation is handled by <a
			href="https://github.students.cs.ubc.ca/CPSC427-2023W-T1/Team03CytotoxicCataclysm/blob/9bdbff0afb2642b8617121bd22d0e65af641abb6/src/world_system.cpp#L309-L345"><u>this</u></a>
			and <a
			href="https://github.students.cs.ubc.ca/CPSC427-2023W-T1/Team03CytotoxicCataclysm/blob/9bdbff0afb2642b8617121bd22d0e65af641abb6/src/physics_system.cpp#L93-L99"><u>this</u></a></p>
			</li>
			</ul>
			</th>
		</tr>
		<tr>
			<th>
			<p>Key-frame/state interpolation</p>
			</th>
			<th><ul>
			<li>
			<p>Ticket #28</p>
			</li>
			<li>
			<p>Health component</p>
			</li>
			<li>
			<p>Links to the player entity.</p>
			</li>
			<li>
			<p>Health value will be checked at each frame.</p>
			</li>
			<li>
			<p>If the health value is changed, start the timer to render the health
			bar to the target value.</p>
			</li>
			<li>
			<p>The render effects use function C(t) = x_0<em>t +
			x_1</em>(1-t/300).</p>
			</li>
			</ul>
			</th>
		</tr>
		<tr>
			<th rowspan="4">Gameplay</th>
			<th>
			<p>Keyboard/mouse control</p>
			</th>
			<th><ul>
			<li>
			<p>Ticket #2</p>
			</li>
			<li>
			<p>Functions:</p>
			</li>
			<li>
			<p>[Keyboard/mouse control]: Player character movement and
			orientation</p>
			</li>
			<li>
			<p>Keyboard handled by <a
			href="https://github.students.cs.ubc.ca/CPSC427-2023W-T1/Team03CytotoxicCataclysm/blob/9bdbff0afb2642b8617121bd22d0e65af641abb6/src/world_system.cpp#L271-L307"><u>this</u></a></p>
			</li>
			<li>
			<p>Mouse handled by <a
			href="https://github.students.cs.ubc.ca/CPSC427-2023W-T1/Team03CytotoxicCataclysm/blob/9bdbff0afb2642b8617121bd22d0e65af641abb6/src/world_system.cpp#L347-L352"><u>this</u></a></p>
			</li>
			</ul>
			</th>
		</tr>
		<tr class="odd">
			<th>
			<p>Random/coded action</p>
			</th>
			<th><ul>
			<li>
			<p>Ticket #6</p>
			</li>
			<li>
			<p>Four sections randomly located in the map</p>
			</li>
			</ul>
			</th>
		</tr>
		<tr>
			<th>
			<p>Well-defined game-space boundaries</p>
			</th>
			<th><ul>
			<li>
			<p>Common::MAP_RADIUS</p>
			</li>
			<li>
			<p>Both Render_system and Physics_system responds to this value of the
			boundary</p>
			</li>
			</ul>
			</th>
		</tr>
		<tr>
			<th>Correct collision processing</th>
			<th><ul>
			<li>
			<p>Ticket #8</p>
			</li>
			<li>
			<p>Functions:</p>
			<ul>
			<li>
			<p>get_collision_circles()</p>
			</li>
			<li>
			<p>collides()</p>
			</li>
			<li>
			<p>collides_with_boundary()</p>
			</li>
			<li>
			<p>(all in physics_system.cpp)</p>
			</li>
			</ul></li>
			<li>
			<p>Detects if: two moving entities collide with each other; a moving
			entity collides with the border</p>
			</li>
			</ul>
			</th>
		</tr>
		<tr>
			<th rowspan="2">Stability</th>
			<th>
			<p>Minimal lag</p>
			</th>
			<th><ul>
			<li>
			<p>Not an issue</p>
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
			<th>[21][basic][Camera controls]:</th>
			<th><ul>
			<li>
			<p>Ticket #4</p>
			</li>
			<li>
			<p>Functions: Render_system::createProjectionMatrix();
			Render_system::createViewMatrix(); <a
			href="https://github.students.cs.ubc.ca/CPSC427-2023W-T1/Team03CytotoxicCataclysm/commit/8869999e7b5a55e54ea7dca3d79a03addfe99d8c?diff=unified#diff-a0a82052c3d34154647fcfa7520d884895e8ad1a51a67f277d0f73b656d711a0">textured.vs.glsl</a></p>
			</li>
			<li>
			<p>Implement a camera following the player.</p>
			</li>
			<li>
			<p>Handled <a
			href="https://github.students.cs.ubc.ca/CPSC427-2023W-T1/Team03CytotoxicCataclysm/blob/9bdbff0afb2642b8617121bd22d0e65af641abb6/src/render_system.cpp#L415-L432"><u>here</u></a></p>
			</li>
			</ul>
			</th>
		</tr>
		<tr>
			<th>[8][basic][Basic physics]</th>
			<th><ul>
			<li>
			<p>Ticket #2 &amp; #8</p>
			</li>
			<li>
			<p>Functions: WorldSystem::handle_collisions(),
			WorldSystem::movement()</p>
			</li>
			<li>
			<p>Add inertia to character movement</p>
			</li>
			<li>
			<p>Elastic collision when moving entities collide with the map
			boundary</p>
			</li>
			</ul>
			</th>
		</tr>
	</thead>
</table>

## Alignment with development plan
All the features that we have planned for MS1 have been completed. In
addition, the camera control feature has been pulled from MS2 into MS1
because it was needed in the early stage for us to test everything else.
The updated version of the proposal has been included in the submission.

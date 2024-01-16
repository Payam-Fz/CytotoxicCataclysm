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
			<th rowspan="2">Stability</th>
			<th>
			<p>Prior missed milestone features & bug fixes</p>
			</th>
			<th><ul>
			<li>
			<p>No missing features or bugs from the prior milestone</p>
			</li>
			</th>
		</tr>
		<tr>
			<th>
			<p>No crashes, glitches, unpredictable behaviour </p>
			</th>
			<th><ul>
			<li>
			<p>This is a collaborative result from code reviews</p>
			</li>
			<li>
			<p>Bug Ticket #159</p>
			</li>
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
			</th>
		</tr>
		<tr>
			<th rowspan="2">User Experience</th>
			<th>
			<p>Comprehensive tutorial</p>
			</th>
			<th><ul>
			<li>
			<p>Ticket #150 (PR #177)</p>
			</li>
			<li>
			<p>Tutorials are displayed at the beginning and when the player unlocks an ability</p>
			</li>
			<li>
			<p>Different tutorials for keyboard and mouse/controller</p>
			</li>
			<li>
			<p>Textures in data/textures/tutorial</p>
			</li>
			</ul>
			</th>
		</tr>
		<tr class="odd">
			<th>
			<p>Optimize user interaction and REPORT it</p>
			</th>
			<th><ul>
			<li>
			<p>See docu/Milestone 4/User Testing Report.pdf</p>
			</li>
			<li>
			<p>More details are included in the document</p>
			</li>
		</tr>
		<tr>
			<th rowspan="7">Creative</th>
			<th>[25][basic][Game balance]</th>
			<th><ul>
			<li>
			<p>See docu/Milestone 4/Game Balance.pdf</p>
			</li>
			<li>
			<p>More details are included in the document</p>
			</li>
			</ul>
			</th>
		</tr>
		<tr>
			<th>[advanced][articulated motion][12]</th>
			<th><ul>
			<li>
			<p>Ticket #92</p>
			</li>
			<li>
			<p>Boss have legs that moves to point towards the player</p>
			</li>
			<li>
			<p>Handled in AISystem::move_articulated_part</p>
			</li>
			</ul>
			</th>
		</tr>
		<tr>
			<th>[Creative][Swarm behavior][16] </th>
			<th><ul>
			<li>
			<p>Pull Request #169</p>
			</li>
			<li>
			<p>Nearby enemies repelling each other (Except bosses and yellow enemies)</p>
			</li>
			<li>
			<p>Handled in ai_system.cpp, line 64 - 84</p>
			</li>
			</ul>
			</th>
		</tr>
		<tr>
			<th>[advanced/basic][story elements][27]</th>
			<th><ul>
			<li>
			<p>Ticket #97 (PR #181)
</p>
			</li>
			<li>
			<p>Added Dialogs at the beginning and before and after a boss fight to tell the story
</p>
			</li>
			<li>
			<p>sub_systems/dialog_system.cpp handles the display and interaction logic of all these dialogs</p>
			</li>
			</ul>
			</th>
		</tr>
		<tr>
			<th>Start and pause menus</th>
			<th><ul>
			<li>
			<p>Ticket #43 (PR #135)</p>
			</li>
			<li>
			<p>Add start and pause menu to the game. Supports start, save/load, resume, mute/unmute, exit, and difficulty selection.
</p>
			</li>
			<li>
			<p>Can be navigated by mouse or controller
</p>
			</li>
			<li>
			<p>Handled by sub_systems/menu_system.cpp
</p>
			</li>
			</ul>
			</th>
		</tr>
		<tr>
			<th>PS5 Controller support</th>
			<th><ul>
			<li>
			<p>PR #166 & PR #185
</p>
			</li>
			<li>
			<p>Add support for using PS5 controller to play the game</p>
			</li>
			<li>
			<p>Supports all the interactions as keyboard and mouse, including regular gameplay, menu navigation, and skipping tutorial/dialogs
</p>
			</li>
			<li>
			<p>Handled by WorldSystem::step_controller, WorldSystem::menu_controller, WorldSystem::control_direction, etc.
</p>
			</li>
			</ul>
			</th>
		</tr>
		<tr>
			<th>[advanced][26][Numerou sophisticated integrated assets]</th>
			<th><ul>
			<li>
			<p>All the assets in the game are hand made for our game except for the chest and the sword texture. Including enemies, backgrounds, bosses, player</p>
			</li>
			<li>
			<p>All the textures under data/textures are in the same art style</p>
			</li>
			<li>
			<p>The music and sound effects are credited in our credits. They also go with the feel of the game.</p>
			</li>
			<li>
			<p>PR #189 Music changes depending on scenarios (menu, main game, boss)</p>
			</li>
			</ul>
			</th>
		</tr>
	</thead>
</table>

## Alignment with development plan
We had the following discrepancies in Milestone 4 compared to the development plan in the original proposal:
* [Creative][advanced][7][2D dynamic shadows] We did not do this because it does not fit in our game

Everything else follows the proposal

## Music
"Voxel Revolution" Kevin MacLeod (incompetech.com)  
Licensed under Creative Commons: By Attribution 4.0 License  
http://creativecommons.org/licenses/by/4.0/  

## Additional Art 
Sound Effects: The Essential Retro Video Game Sound Effects Collection by Juhani Junkala  
https://opengameart.org/content/512-sound-effects-8-bit-style  
Keyboard Sprite by: xPheRe  
https://xphere.itch.io/pixel-keyboard-layout  
Joystick Sprite by: Juliano (lightUP)  
https://julianoferreiradelima.itch.io/gamepads-assets-pixelart  
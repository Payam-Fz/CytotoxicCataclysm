// internal
#include "physics_system.hpp"
#include "world_init.hpp"

// Returns the local bounding coordinates scaled by the current size of the entity
vec2 get_bounding_box(const Transform& transform)
{
	// abs is to avoid negative scale due to the facing direction.
	return { abs(transform.scale.x), abs(transform.scale.y) };
}

struct CollisionCircle {
	vec2 position;
	float radius;
	CollisionCircle(vec2 position, float radius) : 
		position(position), 
		radius(radius) 
	{}
};

// A brief explanation of the collision detection algorithm used in functions below:
// For a textured moving object, try to fit some number of circles within its bounding
// box. These circles together represent the collision region of the entity. if any
// collision circle of one entity collides with any collision circle of another entity,
// then these two entities collide. This is a more accurate approximation when the
// entity's bounding box is not square
std::vector<CollisionCircle> get_collision_circles(const Transform& transform)
{
	std::vector<CollisionCircle> res;
	vec2 bounding_box = get_bounding_box(transform);
	// If the bounding box is square, use a single circle
	if (abs(bounding_box.x - bounding_box.y) < 0.0001f) {
		res.push_back(CollisionCircle(transform.position, bounding_box.x / 2));
	} else {
		// Otherwise partition the rectangle into multiple circles
		float shorter_edge = min(bounding_box.x, bounding_box.y); 
		float longer_edge = max(bounding_box.x, bounding_box.y);
		Transformation transformation;
		transformation.rotate((bounding_box.x < bounding_box.y) ? (M_PI / 2 + transform.angle) : transform.angle);
		// Start from one side of the rectangle and go along the longer edge
		float circle_pos = -(longer_edge / 2.f - shorter_edge / 2.f);
		vec2 pos_offset;
		while (circle_pos < longer_edge / 2.f - shorter_edge / 2.f) {
			pos_offset = vec2(circle_pos, 0);
			pos_offset = transformation.mat * vec3(pos_offset, 1.f);
			res.push_back(CollisionCircle(
				transform.position + vec2(pos_offset.x, pos_offset.y),
				shorter_edge / 2.f
			));
			circle_pos += shorter_edge / 4.f;
		}
		pos_offset = vec2(longer_edge / 2.f - shorter_edge / 2.f, 0);
		pos_offset = transformation.mat * vec3(pos_offset, 1.f);
		res.push_back(CollisionCircle(
			transform.position + vec2(pos_offset.x, pos_offset.y),
			shorter_edge / 2.f
		));
	}
	return res;
}

bool collides(const Transform& transform1, const Transform& transform2)
{
	std::vector<CollisionCircle> circles1 = get_collision_circles(transform1);
	std::vector<CollisionCircle> circles2 = get_collision_circles(transform2);
	for (CollisionCircle circle1 : circles1) {
		for (CollisionCircle circle2: circles2) {
			float distance = length(circle1.position - circle2.position);
			if (distance < circle1.radius + circle2.radius) {
				return true;
			}
		}
	}
	return false;
}

//AABB-AABB collision is used
//reference: https://developer.mozilla.org/en-US/docs/Games/Techniques/3D_collision_detection
bool collides_bounding_box(const Transform& transform1, const Transform& transform2)
{
	//diagnal
	float transform1_scale_diagnal_abs = abs(length(transform1.scale));
	float transform2_scale_diagnal_abs = abs(length(transform2.scale));

	vec2 top_right_1 = { transform1.position.x + transform1_scale_diagnal_abs / 2.f, transform1.position.y + transform1_scale_diagnal_abs / 2.f };
	vec2 bottom_left_1 = { transform1.position.x - transform1_scale_diagnal_abs / 2.f, transform1.position.y - transform1_scale_diagnal_abs / 2.f };

	vec2 top_right_2 = { transform2.position.x + transform2_scale_diagnal_abs / 2.f, transform2.position.y + transform2_scale_diagnal_abs / 2.f };
	vec2 bottom_left_2 = { transform2.position.x - transform2_scale_diagnal_abs / 2.f, transform2.position.y - transform2_scale_diagnal_abs / 2.f };

	return bottom_left_1.x <= top_right_2.x 
		&& top_right_1.x >= bottom_left_2.x
		&& bottom_left_1.y <= top_right_2.y
		&& top_right_1.y >= bottom_left_2.y;
}

bool collides_with_boundary(const Transform& transform)
{
	std::vector<CollisionCircle> collision_circles = get_collision_circles(transform);
	for (CollisionCircle circle : collision_circles) {
		if (length(circle.position) > MAP_RADIUS - circle.radius) {
			return true;
		}
	}
	return false;
}

// Check if a line segment intersects with a circle
// point_1 and point_2 are the two ends of the line segment
// Reference: https://math.stackexchange.com/questions/275529/check-if-line-intersects-with-circles-perimeter
bool line_interesect_with_circle(vec2 point_1, vec2 point_2, CollisionCircle circle) {
	point_1 -= circle.position;
	point_2 -= circle.position;
	float a = pow(point_2.x - point_1.x, 2.f) + pow(point_2.y - point_1.y, 2.f);
	float b = 2 * (point_1.x * (point_2.x - point_1.x) + point_1.y * (point_2.y - point_1.y));
	float c = point_1.x * point_1.x + point_1.y * point_1.y - circle.radius * circle.radius;
	float d = b * b - 4 * a * c;
	if (d > 0) {
		float t1 = (-b + sqrt(d)) / (2 * a);
		float t2 = (-b - sqrt(d)) / (2 * a);
		if ((t1 > 0 && t1 < 1) || (t2 > 0 && t2 < 1)) {
			return true;
		}
	}
	return false;
}

// Returns the knockback direction if collides. Otherwise returns {0, 0}
vec2 collides_with_region_boundary(const Transform& transform, const Motion& motion) {
	float target_angle = atan2f(transform.position.y, transform.position.x);
	float min_region_angle = 0.f, max_region_angle = 0.f;
	float region_spread = M_PI * 2 / NUM_REGIONS;
	for (uint i = 0; i < registry.regions.size(); i++) {
		min_region_angle = registry.transforms.get(registry.regions.entities[i]).angle;
		if (min_region_angle > M_PI) min_region_angle -= 2 * M_PI;	// Convert from [0, 2PI] to [-PI, +PI]
		max_region_angle = min_region_angle + region_spread;
		if (target_angle > min_region_angle && target_angle < max_region_angle) {
			break;	// Found the region
		}
	}
	std::vector<CollisionCircle> collision_circles = get_collision_circles(transform);
	vec2 knockback_dir = {0.f, 0.f};
	for (CollisionCircle circle : collision_circles) {
		if (line_interesect_with_circle(vec2(0.f, 0.f), vec2(cosf(min_region_angle) * MAP_RADIUS, 
										sin(min_region_angle) * MAP_RADIUS), circle)) {
			vec2 normal_vec = normalize(vec2(cosf(min_region_angle + M_PI / 2), sinf(min_region_angle + M_PI / 2)));
			if (dot(motion.velocity, normal_vec) < 0) {
				knockback_dir = motion.velocity - 2 * dot(motion.velocity, normal_vec) * normal_vec;
				break;
			}
		}
		if (line_interesect_with_circle(vec2(0.f, 0.f), vec2(cos(max_region_angle) * MAP_RADIUS, 
										sin(max_region_angle) * MAP_RADIUS), circle)) {
			vec2 normal_vec = normalize(vec2(cos(max_region_angle - M_PI / 2), sin(max_region_angle - M_PI / 2)));
			if (dot(motion.velocity, normal_vec) < 0) {
				knockback_dir = motion.velocity - 2 * dot(motion.velocity, normal_vec) * normal_vec;
				break;
			}
		}
	}
	return knockback_dir;
}

// Check which side of line point target is on. The line goes from point_1 to point_2
// Return true if on one side of line and false if on the other
// Reference: https://stackoverflow.com/questions/1560492/how-to-tell-whether-a-point-is-to-the-right-or-left-side-of-a-line
bool get_side_of_line(vec2 point_1, vec2 point_2, vec2 target) {
	return (point_2.x - point_1.x) * (target.y - point_1.y) - 
		(point_2.y - point_1.y) * (target.x - point_1.x) > 0;
}

//Reference: https://stackoverflow.com/questions/3838329/how-can-i-check-if-two-segments-intersect
bool line_line_intersection_t_u(vec2 A, vec2 B, vec2 C, vec2 D) {
	return get_side_of_line(A, C, D) != get_side_of_line(B,C,D) && get_side_of_line(A,B,C) != get_side_of_line(A,B,D);
}

//check if any kine segment on mesh p1p2p3 is intersecting any line segment of the mesh abc
bool line_line_intersect(vec2 a, vec2 b, vec2 c, vec2 p1, vec2 p2, vec2 p3) {
	//ab p1p2
	if (line_line_intersection_t_u(b, a, p1, p2)) return true;

	//ab p1p3
	if( line_line_intersection_t_u(b, a, p1, p3)) return true;

	//ab p2p3
	if( line_line_intersection_t_u(b, a, p2, p3)) return true;

	//ac p1p2
	if( line_line_intersection_t_u(c, a, p1, p2)) return true;

	//ac p2p3
	if( line_line_intersection_t_u(c, a, p2, p3)) return true;

	//ac p1p3
	if( line_line_intersection_t_u(c, a, p1, p3)) return true;

	//bc p1p2
	if( line_line_intersection_t_u(b, c, p1, p2)) return true;

	//bc p1p3
	if( line_line_intersection_t_u(b, c, p1, p3))return true;

	//bc p2p3
	if( line_line_intersection_t_u(b, c, p2, p3)) return true;

	return false;	
}

bool collides_mesh_with_mesh(Mesh* mesh1, Transform transform_1, Mesh* mesh2, Transform transform_2) {
	Transformation t_matrix1;
	t_matrix1.translate(transform_1.position);
	t_matrix1.rotate(transform_1.angle);
	t_matrix1.scale(transform_1.scale);
	std::vector<vec2> vertex_pos1;

	Transformation t_matrix2;
	t_matrix2.translate(transform_2.position);
	t_matrix2.rotate(transform_2.angle);
	t_matrix2.scale(transform_2.scale);
	std::vector<vec2> vertex_pos2;
	
	// Convert all vertex position to world coordinate
	for (TexturedVertex v1 : mesh1->texture_vertices) {
		vec3 world_pos1 = t_matrix1.mat * vec3(v1.position.x, v1.position.y, 1.f);
		vertex_pos1.push_back(vec2(world_pos1.x, world_pos1.y));
	}

	for (TexturedVertex v2 : mesh2->texture_vertices) {
		vec3 world_pos2 = t_matrix2.mat * vec3(v2.position.x, v2.position.y, 1.f);
		vertex_pos2.push_back(vec2(world_pos2.x, world_pos2.y));
	}

	for (int i = 0; i < mesh1->vertex_indices.size(); i += 3) {
		vec2 point1_1 = vertex_pos1[mesh1->vertex_indices[i]];
		vec2 point1_2 = vertex_pos1[mesh1->vertex_indices[i + 1]];
		vec2 point1_3 = vertex_pos1[mesh1->vertex_indices[i + 2]];
		for (int j = 0; j < mesh2->vertex_indices.size(); j += 3) {
			vec2 point2_1 = vertex_pos2[mesh2->vertex_indices[j]];
			vec2 point2_2 = vertex_pos2[mesh2->vertex_indices[j + 1]];
			vec2 point2_3 = vertex_pos2[mesh2->vertex_indices[j + 2]];
			// line intersection
			if(line_line_intersect(point1_1, point1_2, point1_3, point2_1, point2_2, point2_3)) return true;
		}
	}
	return false;
}

// Check if mesh collides with circles. Mesh is associated with transform_1
bool collides_with_mesh(Mesh *mesh, Transform transform_1, Transform transform_2) {
	std::vector<CollisionCircle> circles = get_collision_circles(transform_2);
	Transformation t_matrix;
	t_matrix.translate(transform_1.position);
	t_matrix.rotate(transform_1.angle);
	t_matrix.scale(transform_1.scale);
	std::vector<vec2> vertex_pos;
	// Convert all vertex position to world coordinate
	for (TexturedVertex v : mesh->texture_vertices) {
		vec3 world_pos = t_matrix.mat * vec3(v.position.x, v.position.y, 1.f);
		vertex_pos.push_back(vec2(world_pos.x, world_pos.y));
	}
	// For each triangle, check if any of the three edges collides with any of the circles
	for (int i = 0; i < mesh->vertex_indices.size(); i += 3) {
		for (CollisionCircle circle : circles) {
			vec2 point_1 = vertex_pos[mesh->vertex_indices[i]];
			vec2 point_2 = vertex_pos[mesh->vertex_indices[i+1]];
			vec2 point_3 = vertex_pos[mesh->vertex_indices[i+2]];
			if (line_interesect_with_circle(point_1, point_2, circle) ||
				line_interesect_with_circle(point_2, point_3, circle) || 
				line_interesect_with_circle(point_3, point_1, circle)) 
			{
				return true;
			} else {
				// The circle might be completely inside the triangle
				bool side_1 = get_side_of_line(point_1, point_2, circle.position);
				bool side_2 = get_side_of_line(point_2, point_3, circle.position);
				bool side_3 = get_side_of_line(point_3, point_1, circle.position);
				if (side_1 == side_2 && side_2 == side_3) {
					return true;
				}
			}
		}
	}
	return false;
}

// Adds collision events to be handled in world_system's resolve_collisions()
void collisionhelper(Entity entity_1, Entity entity_2) {
	// Bullet Collisions
	if (registry.projectiles.has(entity_1)) {
		if (registry.projectiles.has(entity_2)) {
			registry.collisions.emplace_with_duplicates(entity_1, COLLISION_TYPE::BULLET_WITH_BULLET, entity_2);
		}
		else if (registry.players.has(entity_2) && registry.collidePlayers.has(entity_1)) {
			registry.collisions.emplace_with_duplicates(entity_1, COLLISION_TYPE::BULLET_WITH_PLAYER, entity_2);
		}
		else if (registry.enemies.has(entity_2) && registry.collideEnemies.has(entity_1) && registry.collidePlayers.has(entity_2)) {
			registry.collisions.emplace_with_duplicates(entity_1, COLLISION_TYPE::BULLET_WITH_ENEMY, entity_2);
		}
		else if (registry.cysts.has(entity_2) && registry.collideEnemies.has(entity_1)) {
			registry.collisions.emplace_with_duplicates(entity_1, COLLISION_TYPE::BULLET_WITH_CYST, entity_2);
		}
	// Player Collisions
	} else if (registry.players.has(entity_1) && registry.collidePlayers.has(entity_2)) {
		if (registry.enemies.has(entity_2)) {
			registry.collisions.emplace_with_duplicates(entity_1, COLLISION_TYPE::PLAYER_WITH_ENEMY, entity_2);
		} else if (registry.cysts.has(entity_2)) {
			registry.collisions.emplace_with_duplicates(entity_1, COLLISION_TYPE::PLAYER_WITH_CYST, entity_2);
		} else if (registry.chests.has(entity_2)) {
			registry.collisions.emplace_with_duplicates(entity_1, COLLISION_TYPE::PLAYER_WITH_CHEST, entity_2);
		} else if (registry.cure.has(entity_2)) {
			registry.collisions.emplace_with_duplicates(entity_1, COLLISION_TYPE::PLAYER_WITH_CURE, entity_2);
		}
	// Enemy Collisions
	} else if (registry.enemies.has(entity_1)) {
		if (registry.enemies.has(entity_2)) {
			registry.collisions.emplace_with_duplicates(entity_1, COLLISION_TYPE::ENEMY_WITH_ENEMY, entity_2);
		}
	// Sword collisions
	} else if (registry.attachments.has(entity_1) && registry.attachments.get(entity_1).type == ATTACHMENT_ID::SWORD) {
		if (registry.enemies.has(entity_2) && registry.collideEnemies.has(entity_1) && registry.collidePlayers.has(entity_2)) {
			registry.collisions.emplace_with_duplicates(entity_1, COLLISION_TYPE::SWORD_WITH_ENEMY, entity_2);
		} else if (registry.cysts.has(entity_2) && registry.collideEnemies.has(entity_1) && registry.collidePlayers.has(entity_2)) {
			registry.collisions.emplace_with_duplicates(entity_1, COLLISION_TYPE::SWORD_WITH_CYST, entity_2);
		}
	}
}

// Calculates angle of the entity based on result of all the forces acting on it
float get_angle_velocity(Transform& transform, Motion& motion, float elapsed_seconds) {
	float target_angle = atan2f(motion.force.y, motion.force.x);
	float angle_remaining = target_angle - transform.angle + transform.angle_offset;
	if (fabs(angle_remaining) < ANGLE_PRECISION) {
		return 0.f;
	} else if (fabs(angle_remaining) > M_PI) {
		// Handle the case where angle is wrapping
		angle_remaining = (2 * M_PI - fabs(angle_remaining)) * -sign(angle_remaining);
	}
	float required_velocity = fabs(angle_remaining) / elapsed_seconds;
	float velocity_magnitude = min(required_velocity, motion.max_angular_velocity);
	return velocity_magnitude * sign(angle_remaining);
}

// Step movement for all entities with Motion component
void step_movement(float elapsed_ms) {
	// Move NPC based on how much time has passed, this is to (partially) avoid
	// having entities move at different speed based on the machine.
	float elapsed_seconds = elapsed_ms / 1000.f;	// Since velocities are in units per second
	auto& motion_container = registry.motions;
	for (uint i = 0; i < motion_container.size(); i++)
	{
		Entity entity = motion_container.entities[i];
		assert(registry.transforms.has(entity));
		Transform& transform = registry.transforms.get(entity);
		Motion& motion = motion_container.components[i];
		if (registry.attachments.has(entity)) {
			// will deal with these separately
			continue;
		} else {
			// Update velocity based on forces
			if (length(motion.force) > 0.f) {
				motion.velocity += motion.force * elapsed_ms * motion.acceleration_unit;

				// Normalize velocity if not dashing
				if (!registry.dashes.has(entity) || registry.dashes.get(entity).active_timer_ms <= 0) {
					float magnitude = length(motion.velocity);
					if (magnitude > motion.max_velocity) {
						motion.velocity *= (motion.max_velocity / magnitude);
					}
				}
				motion.angular_velocity = get_angle_velocity(transform, motion, elapsed_seconds);
			}

			// Update transform based on velocities
			transform.position += motion.velocity * elapsed_seconds;
			transform.angle += motion.angular_velocity * elapsed_seconds;
			transform.angle = fmod(transform.angle, 2 * M_PI);
		}

		if (registry.animations.has(entity) && registry.players.has(entity)) {
			bool speed_above_threshold = (abs(length(motion.velocity)) - play_animation_threshold) > 0.0f;
			Animation& animation = registry.animations.get(entity);
			if (!speed_above_threshold && animation.total_frame != (int)ANIMATION_FRAME_COUNT::IMMUNITY_BLINKING) {
				RenderSystem::animationSys_switchAnimation(entity, ANIMATION_FRAME_COUNT::IMMUNITY_BLINKING, 120);
			}
			else if (speed_above_threshold && animation.total_frame != (int)ANIMATION_FRAME_COUNT::IMMUNITY_MOVING && animation.total_frame != (int)ANIMATION_FRAME_COUNT::IMMUNITY_DYING) {
				RenderSystem::animationSys_switchAnimation(entity, ANIMATION_FRAME_COUNT::IMMUNITY_MOVING, 30);
			}
		}		
	}
}

void PhysicsSystem::update_attachment_orientation(Entity entity, float elapsed_ms) {
	float elapsed_seconds = elapsed_ms / 1000.f;	// Since velocities are in units per second
	if (registry.transforms.has(entity) && registry.motions.has(entity)) {
		Transform& transform = registry.transforms.get(entity);
		Motion& motion = registry.motions.get(entity);
		Attachment& attachment = registry.attachments.get(entity);
		Entity parent = attachment.parent;
		assert(registry.transforms.has(parent));
		Transform& parent_transform = registry.transforms.get(parent);

		// 1st part of relative transformation
		Transformation pos_calculator;
		pos_calculator.translate(parent_transform.position);
		pos_calculator.rotate(parent_transform.angle);
		pos_calculator.mat = pos_calculator.mat * attachment.relative_transform_1.mat;

		float new_moved_angle = attachment.moved_angle;
		if (length(motion.force) > 0.f) {
			// Update angle based on force (used to control boss arms)
			new_moved_angle += get_angle_velocity(transform, motion, elapsed_seconds) * elapsed_seconds;
		}
		else if (abs(motion.angular_velocity) > 0.f) {
			// Update angle based on constant angle velocity (used for sword)
			new_moved_angle += motion.angular_velocity * elapsed_seconds;
		}
		if (fabs(new_moved_angle - attachment.angle_offset) > ANGLE_PRECISION) {
			float rotate_direction = sign(new_moved_angle);
			attachment.moved_angle = rotate_direction * min(attachment.angle_freedom, fabs(new_moved_angle));
		}
		else {
			attachment.moved_angle = attachment.angle_offset;
		}
		pos_calculator.rotate(attachment.moved_angle);

		// 2nd part of relative transformation
		pos_calculator.mat = pos_calculator.mat * attachment.relative_transform_2.mat;

		// Decompose the result matrix to position + angle + scale
		bool flipped = glm::determinant(glm::mat2(pos_calculator.mat)) < 0;
		if (flipped) {
			pos_calculator.mat[0][0] = -pos_calculator.mat[0][0];
			pos_calculator.mat[1][1] = -pos_calculator.mat[1][1];
		}
		float angle = atan2f(pos_calculator.mat[0][1], pos_calculator.mat[0][0]);
		vec2 scale = { 1.f, 1.f };
		scale.x = length(pos_calculator.mat[0]);
		scale.y = length(pos_calculator.mat[1]);
		if (flipped) {
			// The only possible flipping at the moment is horizontal
			scale.x = -scale.x;
			angle = -angle;
		}
		// Special case for dashing effect attachment
		if (attachment.type == ATTACHMENT_ID::DASHING) {
			vec2 parent_velocity = registry.motions.get(parent).velocity;
			angle = atan2f(parent_velocity.y, parent_velocity.x);
		}

		// Update transform
		transform.position = { pos_calculator.mat[2] };
		transform.angle = angle;
		transform.scale = scale;
	}
}

void step_attachment_movement(float elapsed_ms) {
	auto& attachment_container = registry.attachments;
	for (uint i = 0; i < attachment_container.size(); i++)
	{
		Entity entity = attachment_container.entities[i];
		PhysicsSystem::update_attachment_orientation(entity, elapsed_ms);
	}
}


// duplicate of the one in render_system.cpp
bool is_outside_screen(vec2 entityPos) {
	assert(registry.camera.size() == 1);
	vec2 camPos = registry.camera.components[0].position;

	return length(entityPos - camPos) > SCREEN_RADIUS * 1;
}

// Check collision for all entities with Motion component
void check_collision() {
	auto& motion_container = registry.motions;
	// Check for collisions between all moving entities
	for (uint i = 0; i < motion_container.components.size(); i++)
	{
		Entity entity_i = motion_container.entities[i];
		assert(registry.transforms.has(entity_i));
		Transform transform_i = registry.transforms.get(entity_i);

		// Check for collisions with the map boundary
		if (!registry.cysts.has(entity_i) && collides_with_boundary(transform_i)) {
			if (registry.projectiles.has(entity_i)) {
				registry.collisions.emplace_with_duplicates(entity_i, COLLISION_TYPE::BULLET_WITH_BOUNDARY);
			}
			else {
				registry.collisions.emplace_with_duplicates(entity_i, COLLISION_TYPE::WITH_BOUNDARY);
			}
		}

		// Check for collisions with the region boundary in boss fight
		if (registry.players.has(entity_i) && registry.bosses.size() > 0 && registry.bosses.components.front().activated) {
			vec2 knockback_dir = collides_with_region_boundary(transform_i, motion_container.components[i]);
			if (knockback_dir.x != 0.f && knockback_dir.y != 0.f) {
				registry.collisions.emplace_with_duplicates(entity_i, COLLISION_TYPE::PLAYER_WITH_REGION_BOUNDARY, knockback_dir);
			} 
		}

		// skip if outside screen after checking boundary
		if (is_outside_screen(transform_i.position)) {
			continue;
		}

		// note starting j at i+1 to compare all (i,j) pairs only once (and to not compare with itself)
		for (uint j = i + 1; j < motion_container.components.size(); j++)
		{
			Entity entity_j = motion_container.entities[j];
			assert(registry.transforms.has(entity_j));
			Transform transform_j = registry.transforms.get(entity_j);

			// skip if outside screen
			if (is_outside_screen(transform_j.position)) {
				continue;
			}

			//skip if bounding box is not colliding
			if (!collides_bounding_box(transform_i, transform_j)) {
				continue;
			}

			// ignore collision between an attachment (ie. dashing, sword) and its owner
			if ((registry.attachments.has(entity_i) && registry.attachments.get(entity_i).parent == entity_j)
				|| (registry.attachments.has(entity_j) && registry.attachments.get(entity_j).parent == entity_i)) {
				continue;
			}


			if (registry.meshPtrs.has(entity_i) && registry.meshPtrs.has(entity_j)) {//mesh-mesh collision
				if ( collides_mesh_with_mesh(registry.meshPtrs.get(entity_i), transform_i, registry.meshPtrs.get(entity_j), transform_j) ) {
					collisionhelper(entity_i, entity_j);
					collisionhelper(entity_j, entity_i);
				}
			} else if (registry.meshPtrs.has(entity_i)) {
				if (collides_with_mesh(registry.meshPtrs.get(entity_i), transform_i, transform_j)) {
					collisionhelper(entity_i, entity_j);
					collisionhelper(entity_j, entity_i);
				}
			} else if (registry.meshPtrs.has(entity_j)) {
				if (collides_with_mesh(registry.meshPtrs.get(entity_j), transform_j, transform_i)) {
					collisionhelper(entity_i, entity_j);
					collisionhelper(entity_j, entity_i);
				}
			} else {
				if (collides(transform_i, transform_j))
				{
					// Create a collisions event
					// We are abusing the ECS system a bit in that we potentially insert muliple collisions for the same entity
					// If collision between player and enemy, always add the collision component under player entity
					collisionhelper(entity_i, entity_j);
					collisionhelper(entity_j, entity_i);
				}
			}
		}
	}
}

void PhysicsSystem::step(float elapsed_ms)
{
	step_movement(elapsed_ms);
	step_attachment_movement(elapsed_ms);	// Should handle these after setting all the positions
	check_collision();
}

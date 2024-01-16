#version 330

// Input attributes
in vec3 in_position;
in vec2 in_texcoord;

// Passed to fragment shader
out vec2 texcoord;
out vec2 vertcoord;
out vec2 worldcoord;

// Application data
uniform mat3 transform;
uniform mat3 viewProjection;
uniform float mapRadius;

void main()
{
	vec3 world_pos = transform * vec3(in_position.xy, 1.0);	
	vec3 pos = viewProjection * world_pos;
	texcoord = in_texcoord;
	vertcoord = in_position.xy * mapRadius;
	worldcoord = world_pos.xy;
	gl_Position = vec4(pos.xy, in_position.z, 1.0);
}
#version 330

// Input attributes
in vec3 in_position;
in vec2 in_texcoord;

// Passed to fragment shader
out vec2 texcoord;

// Application data
uniform mat3 transform;
uniform mat3 viewProjection;

uniform float isAnimation;
uniform float totalFrames;
uniform float currFrame;

void main()
{
	texcoord = in_texcoord;
	vec3 pos = viewProjection * transform * vec3(in_position.xy, 1.0);
	gl_Position = vec4(pos.xy, in_position.z, 1.0);

	if(isAnimation > 0.0){
  		texcoord.x = texcoord.x + (1.0/totalFrames)*currFrame;
  	}
}
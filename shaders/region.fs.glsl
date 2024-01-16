#version 330

// From vertex shader
in vec2 texcoord;
in vec2 vertcoord;
in vec2 worldcoord;

// Application data
uniform sampler2D sampler0;
uniform float spawnRadius;
uniform float mapRadius;
uniform float regionAngle;
uniform float edgeThickness;

// Output color
layout(location = 0) out vec4 color;

void main()
{
	float brightness = 0.8f;
	float saturation = 0.9f;
	float origin_dist = length(worldcoord);
	if (origin_dist > mapRadius) {
		discard;
	}
	float angle = atan(vertcoord.y / vertcoord.x);
	float angleToEdge = min(angle, regionAngle - angle);

	if (angleToEdge < edgeThickness) {
		brightness = min(brightness, pow(angleToEdge / edgeThickness, 0.4));
	}
	if (origin_dist < spawnRadius) {
		brightness = min(brightness, pow(origin_dist / spawnRadius, 3));
	}
	if (brightness < 0.01f) {
		discard;
	}
	vec4 temp_color = texture(sampler0, vec2(texcoord.x, texcoord.y));
	temp_color *= brightness;
	float gray = (temp_color.r + temp_color.g + temp_color.b) / 3;
	color = (saturation * temp_color + (1 - saturation) * gray); 
}

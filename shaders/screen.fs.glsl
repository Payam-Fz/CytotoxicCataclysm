#version 330

uniform sampler2D screen_texture;
uniform float time;
uniform float screen_darken_factor;
uniform int is_fov_limited;

in vec2 texcoord;

layout(location = 0) out vec4 color;

vec4 color_shift(vec4 in_color) 
{
	return in_color;
}

vec4 fade_color(vec4 in_color) 
{
	if (screen_darken_factor > 0)
		in_color -= screen_darken_factor * vec4(0.8, 0.8, 0.8, 0);
	return in_color;
}

vec4 limit_fov(vec4 in_color)
{
	if (is_fov_limited == 1) {
        // Apply vignette effect
		float strength = 1.5;
		float smoothness = 0.4;
        vec2 fromCenter = texcoord - vec2(0.5, 0.5);
		float distance = length(fromCenter);
		float vignette = smoothstep(smoothness, 1.0, 1.0 - distance * strength);

        // Combine with other effects
        in_color.rgb *= vignette;
	}
	return in_color;
}

void main()
{
    vec4 in_color = texture(screen_texture, texcoord);
    color = color_shift(in_color);
    color = fade_color(color);
	color = limit_fov(color);
}
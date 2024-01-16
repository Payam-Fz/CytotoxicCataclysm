#version 330

// From vertex shader
in vec2 texcoord;

// Application data
uniform sampler2D sampler0;
uniform vec4 fcolor;
uniform int highlight;

// Output color
layout(location = 0) out  vec4 color;

void main()
{
	color = fcolor * texture(sampler0, vec2(texcoord.x, texcoord.y));
	if (highlight > 0) {
		color = color * 1.2;
	}
}

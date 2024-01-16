#version 330

// From Vertex Shader
in vec3 vcolor;

// Application data
uniform vec4 fcolor;

// Output color
layout(location = 0) out vec4 out_color;

void main()
{
	out_color = vec4(vcolor, 1.0) * fcolor;
}

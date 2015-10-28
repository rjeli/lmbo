#version 330 core

in vec2 texcoord;

uniform sampler2D tex;
uniform vec4 color;

layout(location = 0) out vec4 outColor;

void main()
{
	outColor = vec4(1, 1, 1, texture(tex, texcoord).r) * color;
}

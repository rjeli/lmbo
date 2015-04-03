#version 330 core

in vec2 Texcoord;

uniform sampler2D texFramebuffer;

out vec4 outColor;

void main()
{
	outColor = texture(texFramebuffer, Texcoord);
}

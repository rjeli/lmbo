#version 330 core

in vec3 position;
in vec2 texcoord;

uniform float time;
uniform vec4 mvp;

out vec2 Texcoord;

void main()
{
	Texcoord = texcoord;
	gl_Position = vec4(position.x, position.y + 0.1 * sin(time + position.x), 0.0, 1.0);
}

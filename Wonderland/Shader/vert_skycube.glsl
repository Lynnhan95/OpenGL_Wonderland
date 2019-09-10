#version 430

uniform mat4 PVM;

in vec3 pos_attrib; // In object space

out vec3 pos;


void main(void)
{
	// Compute clip-space vertex position
	gl_Position = PVM * vec4(pos_attrib, 1.0);
	pos = pos_attrib;
}
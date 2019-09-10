#version 430

//uniform mat4 PVM;
//uniform float stroke_width;
uniform vec3 BbMax;

in vec3 pos_attrib;


void main(void)
{
	gl_Position = vec4(BbMax, 1.0);
}
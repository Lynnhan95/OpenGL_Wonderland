#version 430

uniform mat4 lightSpaceMatrix;
uniform mat4 M;

in vec3 pos_attrib;
in vec2 tex_coord_attrib;

out vec2 tex_coord;


void main(void)
{
	gl_Position = lightSpaceMatrix * M * vec4(pos_attrib, 1.0);
	tex_coord = tex_coord_attrib;
}
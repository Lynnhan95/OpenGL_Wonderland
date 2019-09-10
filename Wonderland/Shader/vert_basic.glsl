#version 430

uniform mat4 PVM;

in vec3 pos_attrib;
in vec2 tex_coord_attrib;
in vec3 normal_attrib;

out vec2 tex_coord;
out vec3 normal;


void main(void)
{
	tex_coord = tex_coord_attrib;
	normal = normal_attrib;

	gl_Position = PVM * vec4(pos_attrib, 1.0);
}
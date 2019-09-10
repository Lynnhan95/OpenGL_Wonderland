#version 430

in vec3 pos_attrib;
in vec2 tex_coord_attrib;
// Instanced render info
in float transp_attrib;
in mat4  PVM_attrib;

out vec2  tex_coord;
out float transparency;


void main(void)
{
	tex_coord = tex_coord_attrib;
	transparency = transp_attrib;

	gl_Position = PVM_attrib * vec4(pos_attrib, 1.0);
}
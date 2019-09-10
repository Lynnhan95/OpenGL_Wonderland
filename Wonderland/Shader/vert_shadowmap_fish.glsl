#version 430

uniform mat4 lightSpaceMatrix;
uniform mat4 M;
uniform float time;

in vec3 pos_attrib;
in vec2 tex_coord_attrib;

out vec2 tex_coord;


void main(void)
{
	// Fish vertex animation
	vec3 disp = vec3(0.0, 0.0, 0.05f*sin(5.0f*pos_attrib.x + 3.5f*time));

	gl_Position = lightSpaceMatrix * M * vec4(pos_attrib + disp, 1.0);
	tex_coord = tex_coord_attrib;
}
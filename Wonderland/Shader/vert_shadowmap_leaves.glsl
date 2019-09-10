#version 430

uniform mat4 lightSpaceMatrix;
uniform mat4 M;
uniform float time;
uniform vec3  world_pos;
uniform vec3  wind_dir;
uniform float wind_strength;

in vec3 pos_attrib;
in vec2 tex_coord_attrib;

out vec2 tex_coord;


void main(void)
{
	// Leaves vertex animation
	// TODO: Use less sin function
	vec3 wind_dire = normalize(wind_dir);
	float disp_x = 0.05f*sin(0.5f*pos_attrib.x + 3.5f*wind_strength*time + world_pos.x) * wind_dire.x * wind_strength;
	float disp_y = 0.05f*sin(0.5f*pos_attrib.y + 3.5f*wind_strength*time + world_pos.y) * wind_dire.y * wind_strength;
	float disp_z = 0.05f*sin(0.5f*pos_attrib.z + 3.5f*wind_strength*time + world_pos.z) * wind_dire.z * wind_strength;
	vec3 disp = vec3(disp_x, disp_y, disp_z);

	gl_Position = lightSpaceMatrix * M * vec4(pos_attrib + disp, 1.0);
	tex_coord = tex_coord_attrib;
}
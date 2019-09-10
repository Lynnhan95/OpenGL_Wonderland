#version 430       

uniform mat4 PVM;
uniform mat4 M;
uniform mat3 Normal_M;
uniform mat4 lightSpaceMatrix;
//uniform float time;

in vec3 pos_attrib;
in vec2 tex_coord_attrib;
in vec3 normal_attrib;

out vec3 frag_pos;
out vec2 tex_coord;  
out vec3 normal_w;
out vec4 fragPos_lightSpace;


void main(void)
{
	frag_pos = (M * vec4(pos_attrib, 1.0)).xyz;
	tex_coord = tex_coord_attrib;
	normal_w = normalize(Normal_M*normal_attrib);
	
	fragPos_lightSpace = lightSpaceMatrix * vec4(frag_pos, 1.0);

	gl_Position = PVM * vec4(pos_attrib, 1.0);	
}
#version 430       

uniform mat4 PVM;
uniform mat4 M;
uniform mat3 Normal_M;
uniform mat4 lightSpaceMatrix;
uniform float time;
uniform float wind_strength;

in vec3 pos_attrib;
in vec2 tex_coord_attrib;
in vec3 normal_attrib;

out vec3 frag_pos;
out vec2 tex_coord;  
out vec3 normal_w;
out vec4 fragPos_lightSpace;


void main(void)
{
	// Water wave	
	float timerCon = 0.05 + wind_strength * 0.01;
	float val = timerCon * time;
	vec3 disp = vec3(0.0f, (0.5+0.05*wind_strength)*sin(5 * (5 * val + timerCon * 5 * pos_attrib.y) + 5 * (5 * val + timerCon * 3 * pos_attrib.x) + 5 * (5 * val + timerCon * 1 * pos_attrib.z)), 0.0f);
	
	frag_pos = (M * vec4(pos_attrib + disp, 1.0)).xyz;
	tex_coord = tex_coord_attrib;
	normal_w = normalize(Normal_M*normal_attrib);
	
	fragPos_lightSpace = lightSpaceMatrix * vec4(frag_pos, 1.0);

	gl_Position = PVM * vec4(pos_attrib + disp, 1.0);	
}
#version 430       

uniform mat4 PVM;
uniform mat4 VM;
uniform float time;
uniform vec3 light;

in vec3 pos_attrib;
in vec2 tex_coord_attrib;
in vec3 normal_attrib;

out vec2 tex_coord;  
out vec3 normal;
out float Kd;

void main(void)
{
	vec3 disp = vec3(0.0, 0.0, 0.05f*sin(5.0f*pos_attrib.x + 3.5f*time));
	
	// Transform vertex  position into eye coordinates
	vec3 pos = (VM * vec4(pos_attrib, 1.0) ).xyz + disp;
	vec3 N = normalize( VM*vec4(normal_attrib, 0.0) ).xyz;
	vec3 L = normalize( (VM*vec4(light, 0.0)).xyz - pos );
	Kd = max( dot(L, N), 0.3 );
	
	gl_Position = PVM*vec4(pos_attrib + disp, 1.0);

	tex_coord = tex_coord_attrib;
	normal = normal_attrib;
}
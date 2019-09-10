#version 430
#extension GL_NV_shadow_samplers_cube : enable // Enable textureCube

uniform samplerCube cubemap; // Cubemap source: http://www.custommapmakers.org/skyboxes.php
uniform vec3 tintColor;

in vec3 pos;

out vec4 fragcolor;


void main(void)
{   
	// Use position as sample coordinate
	vec3 npos = normalize(pos);
	fragcolor = textureCube(cubemap, pos) * vec4(tintColor, 1.0);
}





















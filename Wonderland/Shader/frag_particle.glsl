#version 430

uniform sampler2D albedo;
uniform vec3 color;

in vec2  tex_coord;
in float transparency;

out vec4 fragcolor;


void main(void)
{   	
	fragcolor = texture2D(albedo, tex_coord);

	if (fragcolor.a <= 0)
	{
		discard;
	}

	fragcolor.xyz *= color;
	fragcolor.a *= transparency;
}
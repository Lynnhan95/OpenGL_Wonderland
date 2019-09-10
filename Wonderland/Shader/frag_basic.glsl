#version 430

uniform sampler2D albedo;
uniform float transparency;
uniform vec3 color;

in vec2 tex_coord;
in vec3 normal;

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

	//fragcolor.rgb = normal.xyz;
}
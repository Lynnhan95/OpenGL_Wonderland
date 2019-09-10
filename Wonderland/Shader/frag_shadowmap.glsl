#version 430

uniform sampler2D albedo;

in vec2 tex_coord;


void main(void)
{
	gl_FragDepth = gl_FragCoord.z;

	vec4 fragcolor = texture2D(albedo, tex_coord);
	if (fragcolor.a <= 0)
	{
		gl_FragDepth = 1;
	}
}
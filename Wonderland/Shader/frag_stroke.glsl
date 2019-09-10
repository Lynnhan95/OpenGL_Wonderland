#version 430

uniform vec3 stroke_color;
//uniform int can_be_blocked;

out vec4 fragcolor;


void main(void)
{
	//gl_FragDepth = can_be_blocked == 0 ? 0 : gl_FragDepth - 0.01f;
	gl_FragDepth = 0;
	fragcolor = vec4(stroke_color, 1.0f);
}
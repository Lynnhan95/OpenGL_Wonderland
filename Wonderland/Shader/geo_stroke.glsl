#version 430

layout (points) in;
layout (line_strip, max_vertices = 16) out;

uniform mat4 PVM;
uniform vec3 BbMin;


// Build the bounding box
void build_box(vec4 position)
{
	float ox = position.x - BbMin.x;
	float oy = position.y - BbMin.y;
	float oz = position.z - BbMin.z;
	
	gl_Position = PVM * position;
	EmitVertex();
	gl_Position = PVM * (position - vec4(0,0,oz,0));
	EmitVertex();
	gl_Position = PVM * (position - vec4(ox,0,oz,0));
	EmitVertex();
	gl_Position = PVM * (position - vec4(ox,0,0,0));
	EmitVertex();
	gl_Position = PVM * position;
	EmitVertex();
	gl_Position = PVM * (position - vec4(0,oy,0,0));
	EmitVertex();
	gl_Position = PVM * (position - vec4(0,oy,oz,0));
	EmitVertex();
	gl_Position = PVM * (position - vec4(ox,oy,oz,0));
	EmitVertex();
	gl_Position = PVM * (position - vec4(ox,oy,0,0));
	EmitVertex();
	gl_Position = PVM * (position - vec4(0,oy,0,0));
	EmitVertex();
	gl_Position = PVM * (position - vec4(0,oy,oz,0));
	EmitVertex();
	gl_Position = PVM * (position - vec4(0,0,oz,0));
	EmitVertex();
	gl_Position = PVM * (position - vec4(ox,0,oz,0));
	EmitVertex();
	gl_Position = PVM * (position - vec4(ox,oy,oz,0));
	EmitVertex();
	gl_Position = PVM * (position - vec4(ox,oy,0,0));
	EmitVertex();
	gl_Position = PVM * (position - vec4(ox,0,0,0));
	EmitVertex();
	EndPrimitive();
}

void main(void)
{
	build_box(gl_in[0].gl_Position);
}
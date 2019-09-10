#include "Skycube.h"

void Skycube::Init()
{
	Create_Cube_VAO();
	//cube_mesh = LoadMesh(SKYCUBE_MESH_NAME);
}

void Skycube::Create_Cube_VAO()
{
	// generate vao id to hold the mapping from attrib variables in shader to memory locations in vbo
	glGenVertexArrays(1, &m_vao);

	// binding vao means that bindbuffer, enablevertexattribarray and vertexattribpointer 
	// state will be remembered by vao
	glBindVertexArray(m_vao);

	m_vbo = Create_Cube_VBO();

	int pos_loc = 0; // Set in InitShader.cpp
	glBindAttribLocation(cube_shader, pos_loc, "pos_attrib");
	glEnableVertexAttribArray(pos_loc);

	glVertexAttribPointer(pos_loc, 3, GL_FLOAT, false, 3 * sizeof(float), 0);
	glBindVertexArray(0);
}

GLuint Skycube::Create_Cube_VBO()
{
	// Hard coded cube
	const float cube_verts[] = { -1.0f,-1.0f,-1.0f,
							   -1.0f,-1.0f, 1.0f,
							   -1.0f, 1.0f, 1.0f,
							   1.0f, 1.0f,-1.0f,
							   -1.0f,-1.0f,-1.0f,
							   -1.0f, 1.0f,-1.0f,
							   1.0f,-1.0f, 1.0f,
							   -1.0f,-1.0f,-1.0f,
							   1.0f,-1.0f,-1.0f,
							   1.0f, 1.0f,-1.0f,
							   1.0f,-1.0f,-1.0f,
							   -1.0f,-1.0f,-1.0f,
							   -1.0f,-1.0f,-1.0f,
							   -1.0f, 1.0f, 1.0f,
							   -1.0f, 1.0f,-1.0f,
							   1.0f,-1.0f, 1.0f,
							   -1.0f,-1.0f, 1.0f,
							   -1.0f,-1.0f,-1.0f,
							   -1.0f, 1.0f, 1.0f,
							   -1.0f,-1.0f, 1.0f,
							   1.0f,-1.0f, 1.0f,
							   1.0f, 1.0f, 1.0f,
							   1.0f,-1.0f,-1.0f,
							   1.0f, 1.0f,-1.0f,
							   1.0f,-1.0f,-1.0f,
							   1.0f, 1.0f, 1.0f,
							   1.0f,-1.0f, 1.0f,
							   1.0f, 1.0f, 1.0f,
							   1.0f, 1.0f,-1.0f,
							   -1.0f, 1.0f,-1.0f,
							   1.0f, 1.0f, 1.0f,
							   -1.0f, 1.0f,-1.0f,
							   -1.0f, 1.0f, 1.0f,
							   1.0f, 1.0f, 1.0f,
							   -1.0f, 1.0f, 1.0f,
							   1.0f,-1.0f, 1.0f };

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_verts), &cube_verts[0], GL_STATIC_DRAW);

	return vbo;
}

void Skycube::Draw(Camera* camera, glm::vec3 tintcolor)
{
	glBindVertexArray(m_vao);
	//glBindVertexArray(cube_mesh.mVao);

	glUseProgram(cube_shader);

	glm::mat4 Msky = glm::scale(glm::vec3(20.0f));
	glm::mat4 PVM = camera->PV_matrix * Msky;
	PVM[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	RuntimeShader::instance().SetUniform(cube_shader, "PVM", UniformType::mat4, glm::value_ptr(PVM));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
	float map_loc = 0;
	RuntimeShader::instance().SetUniform(cube_shader, "cubemap", UniformType::int1, &map_loc);

	RuntimeShader::instance().SetUniform(cube_shader, "tintColor", UniformType::vec3,glm::value_ptr(tintcolor));

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);  // Draw only inside

	glDepthMask(GL_FALSE); // Draw behind everything

	glDrawArrays(GL_TRIANGLES, 0, 36);
	//cube_mesh.DrawMesh();

	glDepthMask(GL_TRUE);

	glBindVertexArray(0);
	glDisable(GL_CULL_FACE);
}
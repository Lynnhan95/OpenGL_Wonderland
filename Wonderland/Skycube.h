#pragma once

#include <windows.h>
#include "Camera.h"
#include "RuntimeShader.h"
#include "Loader/LoadMesh.h"
//#include <glm/gtx/transform.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>


class Skycube
{
private:
	GLuint m_vao = -1;
	GLuint m_vbo = -1;

	void   Create_Cube_VAO();
	GLuint Create_Cube_VBO();
	//MeshData cube_mesh;

public:
	GLuint cube_shader = -1;
	GLuint cubemap = -1;

	void Init();
	void Draw(Camera* camera, glm::vec3 tintcolor = glm::vec3(1));
};
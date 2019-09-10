#pragma once
#include "SimParameters.h"
#include "LightSource.h"
#include "RuntimeShader.h"
#include "Prefab.h"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>

enum LightMap_Type
{
	LT_SHADOW_MAP = 0
};

// Bounding box of shadow mapping area
struct ShadowBox
{
	glm::vec3 vertA, vertB; // Vertice of box
	glm::vec3 center;

	void Update(Camera* cam);
};

struct LightMap
{
	LightMap_Type type = LightMap_Type::LT_SHADOW_MAP;
	
	GLuint texture_id = -1;
	GLuint lightMapFBO = -1;

	ShadowBox* shadowBox;

	unsigned int resolution_x;
	unsigned int resolution_y;

	glm::mat4 lightSpaceMatrix;
	glm::mat4 lightProjection;

	void InitializeLightMap(unsigned int res_x, unsigned int res_y);
	void GenerateLightMap(Camera* cam, LightSource const light, std::vector<ModelInstance>& const mesh, SimParams* simParams = nullptr);
	void ApplyLightMap(GLuint shader, GLenum unit, float unit_in_float);
	void Clear();
};
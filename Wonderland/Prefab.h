#pragma once

#include <vector>
#include <string>
#include <map>
#include "Loader/LoadMesh.h"
#include "SimParameters.h"
#include "RuntimeShader.h"
#include "Interact.h"
#include "Camera.h"
#include "LightSource.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>

extern struct LightMap; // Resolve mutual inclusion

// Material properties for a single submesh
struct MaterialProperties
{
	GLuint albedo_id = -1;
	GLuint normal_id = -1;
	GLuint spec_id = -1;
	GLuint shader_program = -1;
	GLuint special_shader_program = -1; // For rendering depth map with vertex animation

	glm::vec3 ambt_color = glm::vec3(1, 1, 1);
	glm::vec3 main_color = glm::vec3(1, 1, 1);
	glm::vec3 spec_color = glm::vec3(1, 1, 1);
	float shiness = 10.0f;
	bool  useTextureColor = true;

	bool  isTransparent = false;
	float transparency = 1;

	bool draw_back = false; // Draw both sides of face
	bool has_special_shader = false;
};

struct Prefab
{
	MeshData mesh_data;
	// Material properties
	std::vector<MaterialProperties> materials;

	bool cast_shadow = true;

	bool has_particle_vbo = false;
	GLuint ex_vbo_1 = -1, ex_vbo_2 = -1; // Extra VBOs for particle

	int submeshCount()
	{
		return mesh_data.mSubmesh.size();
	}
};

// Simulation parameters
struct SimParams
{
	glm::vec3 wind_direction = glm::vec3(1.0f, 0.01f, 1.0f);
	float wind_strength = 0;
};

// Instance of prefab
class ModelInstance : public IInteractable
{
private:
	std::vector<Handle> m_i_handles;

public:
	Prefab*	  base;
	glm::vec3 pos;
	glm::vec3 rot;
	glm::vec3 scale;

	std::string name;
	int handleIndex;

	void Draw(Camera* const cam, std::vector<LightSource>& lights, SimParams* simParams = nullptr, LightMap* shadowMap = nullptr, GLuint shader_override = -1);
	void DrawDepth(GLuint shader_override, float* matrix_ptr, SimParams* simParams = nullptr);
	void DrawStroke(Camera* const cam);

	std::string GetIObjectName();
	std::vector<Handle> GetIHandle();
	void ClearHandles();
};

// Resource paths
struct ResourcePath
{
	std::vector<ShaderGroup> groups;
	std::vector<ShaderGroup> special_groups;
	std::string				 model_path;
	std::vector<std::string> albedo_paths;
	std::vector<std::string> normal_paths;
	std::vector<std::string> spec_paths;
	std::vector<bool> draw_back;
	std::vector<bool> has_sp_shader;
	bool cast_shadow;
};

// Hard coded resource initializer
void Init_Resource_Paths(std::map<std::string, ResourcePath>& gResource_paths);
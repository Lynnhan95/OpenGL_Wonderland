#pragma once
#include "RuntimeShader.h"
#include "Interact.h"
#include "Camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

enum LightSource_Type
{
	LS_POINT = 0,
	DIRECTIONAL
};

struct LightProfile
{
	LightSource_Type type;

	glm::vec3 color;

	float intensity_ambt;
	float intensity_diff;
	float intensity_spec;

	// Point
	float radius;
	float attenuation;
	bool  soft_edge;
	float soft_ratio;

	// Directional
	glm::vec3 direction;

	LightProfile()
	{
		type = LightSource_Type::LS_POINT;
		intensity_ambt = 0.1f;
		intensity_diff = 1;
		intensity_spec = 1;
		color = glm::vec3(1, 1, 1);
		radius = 1;
		attenuation = 1;
		soft_edge = true;
		soft_ratio = 0.1f;
		direction = glm::vec3(0, -1, 0);
	}
};

class LightSource : public IInteractable
{
private:
	static int m_point_light_index;
	static int m_directional_light_index;
	int m_light_index = -1;
	std::vector<Handle> m_i_handles;

	LightSource() {};

public:
	LightProfile profile;
	glm::vec3 position;

	std::string name;
	int handleIndex;

	LightSource(LightProfile profile);

	void Apply(GLuint const shader); // Apply shader uniforms
	void DrawStroke(Camera* const camera, GLuint dummyVAO); // Require a 'dummy' VAO to activate shader
	void RefreshIndex(); // Use this to access new index for duplicated light source

	std::string GetIObjectName();
	std::vector<Handle> GetIHandle();
	void ClearHandles();
};

void Init_Light_Profiles(std::map<std::string, LightProfile>& gLightSource_profiles);
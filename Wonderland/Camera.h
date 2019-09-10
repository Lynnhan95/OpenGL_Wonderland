#pragma once
// GLM
#include <glm/glm.hpp> // This header defines basic glm types (vec3, mat4, etc)
// These headers define matrix transformations
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp> // This header defines helper functions (e.g. value_ptr(...)) for passing vectors and matrices to shaders
#include "SimParameters.h"

class Camera
{
private:
	glm::vec3 m_position;
	glm::vec3 m_rotation; // Stored as: (pitch, yaw, 0),
						  // we don't want the camera to roll
	glm::vec3 m_forward, m_right;

public:
	glm::mat4 V_matrix;
	glm::mat4 P_matrix;
	glm::mat4 PV_matrix;

	float fov = FOV;
	float near_clip = NEAR_CLIP;
	float far_clip = FAR_CLIP;

	Camera();

	void Translate(glm::vec3 translation);
	void Rotate(glm::vec3 rotation);
	void Update();
	void Reset();
	glm::vec3 forward();
	glm::vec3 right();
	glm::vec3 up();
	glm::vec3 position();
	glm::vec3 rotation();
};
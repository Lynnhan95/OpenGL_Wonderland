#include "Camera.h"

Camera::Camera()
{
	m_position = INIT_POS;
	m_rotation = INIT_ROT;
	m_forward = INIT_FWD;
	m_right = INIT_RGT;
}

void Camera::Update()
{
	V_matrix = glm::lookAt(m_position, m_position + m_forward, UP);
	P_matrix = glm::perspective(fov, window_aspect, near_clip, far_clip);
	PV_matrix = P_matrix * V_matrix;
}

void Camera::Translate(glm::vec3 translation)
{
	m_position += translation;
}

void Camera::Rotate(glm::vec3 rotation)
{
	m_rotation.x -= rotation.x; // pitch
	m_rotation.y += rotation.y; // yaw

	// Clamp pitch value to prevent screen getting flipped
	m_rotation.x = fmin(89 * D2R, fmax(m_rotation.x, -89 * D2R));
	
	m_forward.x = cos(m_rotation.y) * cos(m_rotation.x);
	m_forward.y = sin(m_rotation.x);
	m_forward.z = sin(m_rotation.y) * cos(m_rotation.x);
	m_right = glm::cross(m_forward, UP);

	// Normalize
	m_forward = glm::normalize(m_forward);
	m_right = glm::normalize(m_right);
}

void Camera::Reset()
{
	m_rotation = INIT_ROT;
	m_position = INIT_POS;
	m_forward = INIT_FWD;
	m_right = INIT_RGT;
}

glm::vec3 Camera::forward()
{
	return m_forward;
}

glm::vec3 Camera::right()
{
	return m_right;
}

glm::vec3 Camera::up()
{
	return glm::cross(m_forward, m_right);
}

glm::vec3 Camera::position()
{
	return m_position;
}

glm::vec3 Camera::rotation()
{
	return m_rotation;
}
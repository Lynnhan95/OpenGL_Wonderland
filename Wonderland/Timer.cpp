#include "Timer.h"

double Timer::m_last_frame_time = glfwGetTime();
double Timer::m_delta_time = 0;
float  Timer::currentTime = STARTING_TIME;
float  Timer::prevTime = STARTING_TIME;

double Timer::deltaTime()
{
	return m_delta_time;
}

void Timer::Update()
{
	m_delta_time = glfwGetTime() - m_last_frame_time;
	m_last_frame_time = glfwGetTime();
}

void Timer::TimeElapse(Scene* scene)
{
	currentTime = (m_last_frame_time * TIME_ELAPSE_SPEED + STARTING_TIME - floor((int)floor(m_last_frame_time * TIME_ELAPSE_SPEED + STARTING_TIME) / 120) * 120.0f);

	glm::vec3 noon_color(1, 1, 1 - MINIMUM_LUMINANCE);
	glm::vec3 night_color(MINIMUM_LUMINANCE, MINIMUM_LUMINANCE, 2 * MINIMUM_LUMINANCE);

	glm::vec3 tintColor = (1.0f - abs(currentTime - 60.0f) / 60.0f) * noon_color + (abs(currentTime - 60.0f) / 60.0f) * night_color;

	if (scene->GetSun() != nullptr)
	{
		scene->GetSun()->profile.color = tintColor;

		float dt = currentTime > prevTime ? currentTime - prevTime : 120 - prevTime + currentTime;
		scene->GetSun()->profile.direction = glm::normalize(
			glm::rotate(dt * 3.0f * D2R, glm::vec3(0, 0, 1)) * 
			glm::vec4(glm::normalize(scene->GetSun()->profile.direction), 0));

		// Respawn direction
		if (scene->GetSun()->profile.direction.x >= 0.999f)
		{
			scene->GetSun()->profile.direction.x = -0.999f;
			scene->GetSun()->profile.direction.y = -0.001f;
			scene->GetSun()->profile.direction.z = 0;
		}
	}

	scene->sunTint = tintColor;

	prevTime = currentTime;
}
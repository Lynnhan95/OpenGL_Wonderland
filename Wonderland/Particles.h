#pragma once
#include "Prefab.h"
#include "Camera.h"
#include "SimParameters.h"
#include "Interact.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct ParticleInstance
{
	Prefab* base;

	bool  isActive;
	float lifetime;
	float passed_time;

	glm::vec3 pos;
	glm::vec3 rot;
	glm::vec3 scale;
	
	glm::vec3 init_speed;
	glm::vec3 init_scale;
	glm::vec3 init_rot_speed;
	glm::vec3 translate_speed;
	glm::vec3 rotate_speed;

	float alpha;
	float camera_distance; // For distance sorting

	ParticleInstance()
	{
		base = nullptr;
		isActive = true;
		lifetime = 0;
		passed_time = 0;
		pos = glm::vec3(0);
		rot = glm::vec3(0);
		scale = glm::vec3(1); 
		init_speed = glm::vec3(0);
		init_scale = glm::vec3(0);
		init_rot_speed = glm::vec3(0);
		translate_speed = glm::vec3(0);
		rotate_speed = glm::vec3(0);
		alpha = 1;
		camera_distance = 0;
	};
};

enum ParticleSystemShape
{
	CONE = 0,
	SPHERE,
	PLANE
};

struct ParticleSystemProperties
{
	bool play_on_awake;
	bool billboard;

	ParticleSystemShape shape;

	// Take random between two values
	std::pair<float, float> initial_scale;
	std::pair<float, float> initial_speed;

	std::pair<glm::vec3, glm::vec3> initial_rotation;
	std::pair<glm::vec3, glm::vec3> initial_rotation_speed;

	glm::vec3 color;

	bool  isTransparent;
	float initial_alpha;

	// Final state
	float scale_over_lifetime;
	float speed_over_lifetime;
	float rot_speed_over_lifetime;
	float alpha_over_lifetime;

	float lifetime;
	float particle_lifetime;

	int   emission_rate;
	int   max_count;
	bool  repeat;

	float cone_angle;
	float plane_width;
	float plane_height;

	bool  use_gravity;
	bool  affected_by_wind;
	float wind_coef;

	ParticleSystemProperties()
	{
		play_on_awake = true;
		billboard = false;
		shape = ParticleSystemShape::CONE;
		initial_scale = std::pair<float, float>(1, 1);
		initial_speed = std::pair<float, float>(0, 0);
		initial_rotation = std::pair<glm::vec3, glm::vec3>(glm::vec3(0), glm::vec3(0));
		initial_rotation_speed = std::pair<glm::vec3, glm::vec3>(glm::vec3(0), glm::vec3(0));
		color = glm::vec3(1);
		isTransparent = false;
		initial_alpha = 1;
		scale_over_lifetime = 1;
		speed_over_lifetime = 1;
		alpha_over_lifetime = 1;
		rot_speed_over_lifetime = 1;
		lifetime = 0;
		particle_lifetime = 0;
		emission_rate = 0;
		max_count = 0;
		repeat = false;
		cone_angle = 0;
		plane_width = 0;
		plane_height = 0;
		use_gravity = false;
		affected_by_wind = false;
		wind_coef = 0;
	};
};

class ParticleSystem : public IInteractable
{
private:
	std::vector<ParticleInstance> m_particles;

	GLuint shader_program;
	GLuint m_transpVBO = -1, m_mvpVBO = -1;

	bool  m_is_playing;
	float m_prev_time, m_passed_time, m_since_last_emit;
	int   m_active_count;
	bool  m_deprecated; // Mark this particle system to be removed, non-reversible

	// For cone and plane shape emitter
	glm::vec3 m_emit_dir;
	// For plane shape emitter
	glm::vec3 m_diag_1, m_diag_2;

	std::vector<Handle> m_i_handles;

	ParticleSystem() {};

public:
	ParticleSystemProperties properties;
	Prefab* prefab;

	glm::vec3 position;
	glm::vec3 rotation;

	std::string name;
	int handleIndex;

	ParticleSystem(ParticleSystemProperties properties, Prefab* prefab, glm::vec3 pos, glm::vec3 rot = glm::vec3(0));

	void Update(Camera* const camera, SimParams* simParams = nullptr);
	void Draw(Camera* const camera);
	void DrawStroke(Camera* const camera);
	void Play();
	void Stop();
	void Deprecate(); // Like an emergency brake, normally you won't call it from out side
	void Translate(glm::vec3 distance); // Use this for plane shaped emitter
	void Rotate(glm::vec3 angles); // Use this for cone and plane shaped emitter

	bool isDeprecated();
	void Release();

	std::string GetIObjectName();
	std::vector<Handle> GetIHandle();
	void ClearHandles();
};

void Init_ParticleSystem_Properties(std::map<std::string, ParticleSystemProperties>& gPartiSystem_profiles);
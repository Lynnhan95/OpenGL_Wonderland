#pragma once

#include <vector>
#include <map>

#include "Loader/LoadMesh.h"
#include "Camera.h"
#include "RuntimeShader.h"
#include "Prefab.h"
#include "Particles.h"
#include "LightSource.h"
#include "LightMap.h"
#include "Skycube.h"

#include <GLFW/glfw3.h>

// Scene generator
class Scene
{
private:
	// TODO: Use trees instead of vetors to store scene graph
	std::vector<ModelInstance>  m_models; // Keep mesh instance instead of duplicated mesh data
	std::vector<ParticleSystem> m_particleSystems;
	std::vector<LightSource>	m_lights;

	int m_pointlight_count = 0;
	int m_directionallight_count = 0;

	enum _ObjectGroup
	{
		MODEL = 0,
		PARTICLE,
		LIGHT
	};
	int m_currHandleIndex = 0;
	std::map<int, std::pair<_ObjectGroup, int>> m_objectHandleMap; // Reference to current scene objects

	int focusedModel = -1; // Model to stroke, -1 means none
	int focusedParticleSys = -1;
	int focusedLightSource = -1;

	int m_shadow_light_index = -1; // Index of directional light with shadow

	Skycube m_skybox;

public:
	// Global prefabs
	static std::map<std::string, Prefab> gPrefabs;

	// Modifiable scene parameters
	SimParams simParams;
	glm::vec3 sunTint = glm::vec3(1);

	// Light map of this scene
	LightMap  mainLightMap;

	void Add(ModelInstance* model);
	void Add(ParticleSystem* system);
	void Add(LightSource* light);

	void DrawScene(Camera* const cam);
	void Update(Camera* const cam);
	void UseSkymap(GLuint skybox_map);
	void Clear(); // Clear model instance
	void Reset(); // Clear both model prefab and instance

	int GetModelCount();
	int GetLightCount();
	int GetParticleSystemCount();

	std::vector<Handle> GetObjectHandleByIndex(int idx);
	std::string GetObjectNameByIndex(int idx);
	void DuplicateObject(int idx);

	// Access scene contents to be saved
	const std::vector<ModelInstance>*  ModelContent();
	const std::vector<ParticleSystem>* ParticleSysContent();
	const std::vector<LightSource>*	   LightContent();
	LightSource* GetSun();
};
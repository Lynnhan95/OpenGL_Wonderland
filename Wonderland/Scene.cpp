#include "Scene.h"

// Do the definition
std::map<std::string, Prefab> Scene::gPrefabs;

void Scene::Add(ModelInstance* model)
{
	model->handleIndex = m_currHandleIndex;
	m_models.push_back(*model);
	m_objectHandleMap[m_currHandleIndex] = 
		std::make_pair<_ObjectGroup, int>(_ObjectGroup::MODEL, m_models.size() - 1); // Not pretty solution, but will do for now
	m_currHandleIndex++;
}

void Scene::Add(ParticleSystem* system)
{
	system->handleIndex = m_currHandleIndex;
	m_particleSystems.push_back(*system);
	m_objectHandleMap[m_currHandleIndex] = 
		std::make_pair<_ObjectGroup, int>(_ObjectGroup::PARTICLE, m_particleSystems.size() - 1);
	m_currHandleIndex++;
}

void Scene::Add(LightSource* light)
{
	light->handleIndex = m_currHandleIndex;
	m_lights.push_back(*light);
	m_objectHandleMap[m_currHandleIndex] = 
		std::make_pair<_ObjectGroup, int>(_ObjectGroup::LIGHT, m_lights.size() - 1);
	m_currHandleIndex++;

	if (light->profile.type == LightSource_Type::LS_POINT)
	{
		++m_pointlight_count;
	}
	else if (light->profile.type == LightSource_Type::DIRECTIONAL)
	{
		m_shadow_light_index = m_lights.size() - 1;
		++m_directionallight_count;
	}
}

void Scene::DrawScene(Camera* const cam)
{
	// Generate depth map
	if (&mainLightMap != nullptr)
	{
		mainLightMap.GenerateLightMap(cam, m_lights[m_shadow_light_index], m_models, &simParams);
	}

	// Stroke selection
	if (focusedModel >= 0)
	{
		m_models[focusedModel].DrawStroke(cam);
	}
	else if (focusedParticleSys >= 0)
	{
		m_particleSystems[focusedParticleSys].DrawStroke(cam);
	}
	else if (focusedLightSource >= 0)
	{
		m_lights[focusedLightSource].DrawStroke(cam, m_models[0].base->mesh_data.mVao); // Use m_models[0]'s VAO as dummy
	}

	// Skycube
	// Draw before particles to allow particle blending
	if (&m_skybox != nullptr)
	{
		m_skybox.Draw(cam, sunTint);
	}

	// Draw model
	for (int i = 0; i < m_models.size(); i++)
	{
		m_models[i].Draw(cam, m_lights, &simParams, &mainLightMap);
	}

	// Draw particles
	for (int i = 0; i < m_particleSystems.size(); i++)
	{
		m_particleSystems[i].Draw(cam);
	}
}

void Scene::Update(Camera* const cam)
{
	for (int i = 0; i < m_particleSystems.size(); i++)
	{
		m_particleSystems[i].Update(cam, &simParams);
	}
	// Clear timeout particle system
	for (auto itr = m_particleSystems.begin(); itr != m_particleSystems.end(); itr++)
	{
		if (itr->isDeprecated())
		{
			itr->Release();
			m_particleSystems.erase(itr);
			itr = m_particleSystems.begin(); // Reset iterator
		}
	}
}

void Scene::UseSkymap(GLuint skybox_map)
{
	m_skybox = *new Skycube();
	m_skybox.cube_shader = RuntimeShader::instance().GetShader(ShaderGroup::SKYCUBE);
	m_skybox.cubemap = skybox_map;
	m_skybox.Init();
}

void Scene::Clear()
{
	m_models.clear();
	m_particleSystems.clear();
	m_lights.clear();
	m_objectHandleMap.clear();
	m_currHandleIndex = 0;
}

void Scene::Reset()
{
	m_models.clear();
	m_particleSystems.clear();
	m_lights.clear();
	m_objectHandleMap.clear();
	m_currHandleIndex = 0;

	gPrefabs.clear();
}

std::vector<Handle> Scene::GetObjectHandleByIndex(int idx)
{
	if (m_objectHandleMap.find(idx) == m_objectHandleMap.end())
	{
		std::cout << "Object index out of range." << std::endl;
		return std::vector<Handle>();		
	}

	focusedModel = -1;
	focusedParticleSys = -1;
	focusedLightSource = -1;

	switch (m_objectHandleMap[idx].first)
	{
	case _ObjectGroup::MODEL:
		focusedModel = m_objectHandleMap[idx].second;
		return m_models[focusedModel].GetIHandle();
		break;
	case _ObjectGroup::PARTICLE:
		focusedParticleSys = m_objectHandleMap[idx].second;
		return m_particleSystems[focusedParticleSys].GetIHandle();
		break;
	case _ObjectGroup::LIGHT:
		focusedLightSource = m_objectHandleMap[idx].second;
		return m_lights[focusedLightSource].GetIHandle();
		break;
	}

	return std::vector<Handle>();
}

std::string Scene::GetObjectNameByIndex(int idx)
{
	if (m_objectHandleMap.find(idx) == m_objectHandleMap.end())
	{
		std::cout << "Object index out of range." << std::endl;
		return "<error>";
	}
	
	switch (m_objectHandleMap[idx].first)
	{
	case _ObjectGroup::MODEL:
		return m_models[m_objectHandleMap[idx].second].GetIObjectName();
		break;
	case _ObjectGroup::PARTICLE:
		return m_particleSystems[m_objectHandleMap[idx].second].GetIObjectName();
		break;
	case _ObjectGroup::LIGHT:
		return m_lights[m_objectHandleMap[idx].second].GetIObjectName();
		break;
	}

	return "<error>";
}

void Scene::DuplicateObject(int idx)
{
	if (m_objectHandleMap.find(idx) == m_objectHandleMap.end())
	{
		std::cout << "Object index out of range." << std::endl;
		return;
	}

	if (m_objectHandleMap[idx].first == _ObjectGroup::MODEL)
	{
		ModelInstance newInstance = m_models[m_objectHandleMap[idx].second];
		newInstance.ClearHandles();
		Add(&newInstance);
	}
	else if (m_objectHandleMap[idx].first == _ObjectGroup::PARTICLE)
	{
		ParticleSystem newInstance = m_particleSystems[m_objectHandleMap[idx].second];
		newInstance.ClearHandles();
		Add(&newInstance);
	}
	else if (m_objectHandleMap[idx].first == _ObjectGroup::LIGHT)
	{
		LightSource newInstance = m_lights[m_objectHandleMap[idx].second];
		// Check if light source amount exceeds limit
		if (newInstance.profile.type == LightSource_Type::LS_POINT)
		{
			if (m_pointlight_count >= MAX_POINT_LIGHT)
			{
				return;
			}
			else
			{
				++m_pointlight_count;
			}
		}
		else if (newInstance.profile.type == LightSource_Type::DIRECTIONAL)
		{
			if (m_pointlight_count >= MAX_DIRECTIONAL_LIGHT)
			{
				return;
			}
			else
			{
				++m_directionallight_count;
			}
		}
		m_lights[m_objectHandleMap[idx].second].ClearHandles(); // Don't quite know why, but we also need to clear the original one here
		newInstance.ClearHandles();
		newInstance.RefreshIndex();
		Add(&newInstance);
	}
}

int Scene::GetModelCount()
{
	return m_models.size();
}

int Scene::GetLightCount()
{
	return m_lights.size();
}

int Scene::GetParticleSystemCount()
{
	return m_particleSystems.size();
}

const std::vector<ModelInstance>* Scene:: ModelContent()
{
	return &m_models;
}
const std::vector<ParticleSystem>* Scene::ParticleSysContent()
{
	return &m_particleSystems;
}

const std::vector<LightSource>* Scene::LightContent()
{
	return &m_lights;
}

LightSource* Scene::GetSun()
{
	return &m_lights[m_shadow_light_index];
}
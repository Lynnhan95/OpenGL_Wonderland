#pragma once
#include <vector>
#include "Scene.h"
#include "SimParameters.h"
#include "Loader/LoadTexture.h" // Functions for creating OpenGL textures from image files

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>

//#define _CRT_SECURE_NO_WARNINGS /* Disable warnings for fscanf() */

enum ElementRead
{
	NONE = 0,
	MODEL,
	PARTICLE_SYS,
	LIGHT
};

// Multipurpose loader function
void LoadModel(const char* name, std::map<std::string, ResourcePath> gResource_paths)
{
	Prefab* newPrefab = new Prefab();

	// Load mesh
	newPrefab->mesh_data = LoadMesh(gResource_paths[name].model_path);

	// Load material
	for (int i = 0; i < newPrefab->submeshCount(); i++)
	{
		newPrefab->materials.push_back(*new MaterialProperties());

		if (gResource_paths[name].albedo_paths.size() > 0)
			newPrefab->materials[i].albedo_id = LoadTexture(gResource_paths[name].albedo_paths[i]);

		if (gResource_paths[name].normal_paths.size() > 0)
			newPrefab->materials[i].normal_id = LoadTexture(gResource_paths[name].normal_paths[i]);

		if (gResource_paths[name].spec_paths.size() > 0)
			newPrefab->materials[i].spec_id = LoadTexture(gResource_paths[name].spec_paths[i]);

		if (gResource_paths[name].draw_back.size() > 0)
		{
			newPrefab->materials[i].draw_back = gResource_paths[name].draw_back[i];
		}

		if (gResource_paths[name].has_sp_shader.size() > 0)
		{
			newPrefab->materials[i].has_special_shader = gResource_paths[name].has_sp_shader[i];
			newPrefab->materials[i].special_shader_program = RuntimeShader::instance().GetShader(gResource_paths[name].special_groups[i]);
		}

		newPrefab->materials[i].shader_program = RuntimeShader::instance().GetShader(gResource_paths[name].groups[i]);
	}

	// Set per-prefab properties
	newPrefab->cast_shadow = gResource_paths[name].cast_shadow;

	Scene::gPrefabs.insert(std::pair<std::string, Prefab>(name, *newPrefab));
}

// Load map info from *.wmap file into the given scene
// TODO: Parse .xml format map file
void LoadMap(Scene* sceneToFill, const char* addrstr, std::map<std::string, ResourcePath> gResource_paths, std::map<std::string, ParticleSystemProperties>& gPartiSystem_profiles, 
	std::map<std::string, LightProfile>& gLightSource_profiles)
{
	std::ifstream fileReader;
	fileReader.open(addrstr);

	if (fileReader.fail())
	{
		std::cout << "Cannot open " << addrstr << std::endl;
		return;
	}

	int light_count = 0;

	std::string line;
	while (std::getline(fileReader, line))
	{
		std::istringstream steam(line); // don't want to use the name 'stream'

		// Read line header
		char header[128]; // Max length of 128
		if (!(steam >> header))
		{
			std::cout << "Map file element header is corrupted!" << std::endl;
			return;
		}
		if (strcmp(header, "#") == 0 || header[0] == '#') // This line is comment
		{
			continue;
		}
		ElementRead element = ElementRead::NONE;
		if (strcmp(header, "M") == 0)
		{
			element = ElementRead::MODEL;
		}
		else if (strcmp(header, "P") == 0)
		{
			element = ElementRead::PARTICLE_SYS;
		}
		else if (strcmp(header, "L") == 0)
		{
			element = ElementRead::LIGHT;
		}

		// Load element
		if (element != ElementRead::NONE)
		{
			// Get element name
			char elementName[128]; // Max name length is 128
			if (!(steam >> elementName))
			{
				std::cout << "Map file element name is corrupted!" << std::endl;
				return;
			}

			// Load a new model
			if (element != ElementRead::LIGHT && Scene::gPrefabs.find(elementName) == Scene::gPrefabs.end())
			{
				LoadModel(elementName, gResource_paths);
			}

			line.clear();
			while (std::getline(fileReader, line))
			{
				std::istringstream psn(line);

				char subHeader[128];
				if (!(psn >> subHeader))
				{
					std::cout << "Map file subheader is corrupted!" << std::endl;
					return;
				}

				// End of an element
				if (strcmp(subHeader, "M") == 0 || strcmp(subHeader, "P") == 0 || strcmp(subHeader, "L") == 0)
				{
					break;
				}
				bool EOE = false; // End of element

				// Add model instance
				if (element == ElementRead::MODEL)
				{
					// Geometry info
					glm::vec3 pos, rot, scale;

					if (!(psn >> pos.x >> pos.y >> pos.z
						 >> rot.x >> rot.y >> rot.z
						 >> scale.x >> scale.y >> scale.z))
					{
						std::cout << "Map file model data is corrupted!" << std::endl;
						return;
					}

					ModelInstance* instance = new ModelInstance();
					instance->base = &Scene::gPrefabs[elementName];
					instance->name = elementName;
					instance->pos = pos;
					instance->rot = rot;
					instance->scale = scale;

					// Material info
					line.clear();
					int mat_index = 0;
					while (std::getline(fileReader, line)) // Could have multiple lines
					{
						std::istringstream eshop(line);

						char subsubHeader[128];
						if (!(eshop >> subsubHeader))
						{
							std::cout << "Map file property header is corrupted!" << std::endl;
							return;
						}
						if (strcmp(subsubHeader, "M") == 0)
						{
							EOE = true;
							break;
						}

						glm::vec3 mainCol, specCol, ambtCol;
						float shiness, transp;
						bool useTex, isTransp;

						if (!(eshop >> mainCol.x >> mainCol.y >> mainCol.z
							 >> specCol.x >> specCol.y >> specCol.z
							 >> ambtCol.x >> ambtCol.y >> ambtCol.z
							 >> shiness >> useTex
							 >> isTransp >> transp))
						{
							std::cout << "Map file material data is corrupted!" << std::endl;
							return;
						}

						instance->base->materials[mat_index].main_color = mainCol;
						instance->base->materials[mat_index].spec_color = specCol;
						instance->base->materials[mat_index].ambt_color = ambtCol;
						instance->base->materials[mat_index].shiness = shiness;
						instance->base->materials[mat_index].transparency = transp;
						instance->base->materials[mat_index].useTextureColor = useTex;
						instance->base->materials[mat_index].isTransparent = isTransp;

						++mat_index;
					}

					sceneToFill->Add(instance);
				}

				// Add particle system
				else if (element == ElementRead::PARTICLE_SYS)
				{				
					glm::vec3 pos, col;

					// Instance info
					if (!(psn >> pos.x >> pos.y >> pos.z))
					{
						std::cout << "Map file particle sys data is corrupted!" << std::endl;
						return;
					}

					// Property info
					line.clear();
					if (!std::getline(fileReader, line)) // Only one line
					{
						std::cout << "Map file particle sys property data is corrupted!" << std::endl;
						return;
					}
					std::istringstream eshop(line);
					char subsubHeader[128];
					if (!(eshop >> subsubHeader))
					{
						std::cout << "Map file particle sys property data is corrupted!" << std::endl;
						return;
					}
					if (!(eshop >> col.x >> col.y >> col.z))
					{
						std::cout << "Map file particle sys property data is corrupted!" << std::endl;
						return;
					}

					ParticleSystem* system = new ParticleSystem(gPartiSystem_profiles[elementName], &Scene::gPrefabs[elementName], pos);
					system->name = elementName;
					system->properties.color = col;
					sceneToFill->Add(system);
				}

				// Add light source
				else if (element == ElementRead::LIGHT)
				{
					if (light_count < MAX_POINT_LIGHT + MAX_DIRECTIONAL_LIGHT)
					{
						LightSource* light = new LightSource(gLightSource_profiles[elementName]);

						if (strcmp(subHeader, "LPP") == 0) // Point light properties
						{
							glm::vec3 pos, col;
							float radius, atten, it_diff, it_spec, it_ambt, softRatio;
							bool softEdge;

							if (!(psn >> pos.x >> pos.y >> pos.z
								>> col.x >> col.y >> col.z
								>> radius 
								>> atten >> it_diff >> it_spec >> it_ambt
								>> softEdge >> softRatio))
							{
								std::cout << "Map file point light data is corrupted!" << std::endl;
								return;
							}

							light->name = elementName;
							light->position = pos;		
							light->profile.color = col;
							light->profile.radius = radius;
							light->profile.attenuation = atten;
							light->profile.intensity_diff = it_diff;
							light->profile.intensity_spec = it_spec;
							light->profile.intensity_ambt = it_ambt;
							light->profile.soft_edge = softEdge;
							light->profile.soft_ratio = softRatio;
						}
						else if (strcmp(subHeader, "LPD") == 0) // Directional light properties
						{
							glm::vec3 dir, col;
							float it_diff, it_spec, it_ambt;

							if (!(psn >> dir.x >> dir.y >> dir.z
								>> col.x >> col.y >> col.z
								>> it_diff >> it_spec >> it_ambt))
							{
								std::cout << "Map file directional light data is corrupted!" << std::endl;
								return;
							}

							light->name = elementName;
							light->profile.direction = dir;
							light->profile.color = col;
							light->profile.intensity_diff = it_diff;
							light->profile.intensity_spec = it_spec;
							light->profile.intensity_ambt = it_ambt;
						}
						else
						{
							std::cout << "Unrecognized light source type found." << std::endl;
							return;
						}

						++light_count;
						sceneToFill->Add(light);
					}
				}

				if (EOE)
				{
					break;
				}
			}

		}

	}

	fileReader.close();

	std::cout << "Map ";
	std::cout << addrstr;
	std::cout << " read success." << std::endl;	
}
#pragma once
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include "Scene.h"

// Write back current scene contents to the map file
// TODO: Store map data in .xml file
bool WriteMap(Scene* sceneToSave, const char* addrstr)
{
	std::ofstream fileWriter;
	fileWriter.open(addrstr);

	if (fileWriter.fail())
	{
		std::cout << "Cannot access " << addrstr << std::endl;
		return false;
	}

	// Comments
	fileWriter << "# This map file is dedicated to Wonderland program written by N7RX\n";

	// Save models
	for (ModelInstance instance : *sceneToSave->ModelContent())
	{
		// Model instance
		fileWriter << "M " << instance.name << "\n"; // Name
		fileWriter << "I " << instance.pos.x << " " << instance.pos.y << " " << instance.pos.z << " "; // Position
		fileWriter << instance.rot.x << " " << instance.rot.y << " " << instance.rot.z << " "; // Rotation
		fileWriter << instance.scale.x << " " << instance.scale.y << " " << instance.scale.z << "\n"; // Scale
		// Material properties
		for (int i = 0; i < instance.base->materials.size(); i++)
		{
			fileWriter << "MP " << instance.base->materials[i].main_color.x << " " << instance.base->materials[i].main_color.y << " " << instance.base->materials[i].main_color.z << " "; // Main color
			fileWriter << instance.base->materials[i].spec_color.x << " " << instance.base->materials[i].spec_color.y << " " << instance.base->materials[i].spec_color.z << " "; // Spec color
			fileWriter << instance.base->materials[i].ambt_color.x << " " << instance.base->materials[i].ambt_color.y << " " << instance.base->materials[i].ambt_color.z << " "; // Ambient color
			fileWriter << instance.base->materials[i].shiness << " "; // Shiness
			fileWriter << (instance.base->materials[i].useTextureColor ? "1" : "0") << " "; // Shiness
			fileWriter << (instance.base->materials[i].isTransparent ? "1" : "0") << " "; // Is Transparent
			fileWriter << instance.base->materials[i].transparency << "\n"; // Transparency;
		}
		fileWriter << "M\n"; // End of instance
	}

	// Save particle systems
	for (ParticleSystem instance : *sceneToSave->ParticleSysContent())
	{
		// Particle system instance
		fileWriter << "P " << instance.name << "\n"; // Name
		fileWriter << "I " << instance.position.x << " " << instance.position.y << " " << instance.position.z << "\n"; // Position
		// Material properties
		fileWriter << "MP " << instance.properties.color.x << " " << instance.properties.color.y << " " << instance.properties.color.z << "\n"; // Color
		fileWriter << "P\n"; // End of instance
	}

	// Save light sources
	for (LightSource instance : *sceneToSave->LightContent())
	{
		// Light source instance
		fileWriter << "L " << instance.name << "\n"; // Name
		// Light properties
		switch (instance.profile.type)
		{
		case LightSource_Type::LS_POINT:
			fileWriter << "LPP " << instance.position.x << " " << instance.position.y << " " << instance.position.z << " "; // Position
			fileWriter << instance.profile.color.x << " " << instance.profile.color.y << " " << instance.profile.color.z << " "; // Color
			fileWriter << instance.profile.radius << " "; // Radius
			fileWriter << instance.profile.attenuation << " "; // Attenuation
			fileWriter << instance.profile.intensity_diff << " " << instance.profile.intensity_spec << " " << instance.profile.intensity_ambt << " "; // Intensities
			fileWriter << (instance.profile.soft_edge ? "1" : "0") << " " << instance.profile.soft_ratio << "\n"; // Soft edge
			break;
		case LightSource_Type::DIRECTIONAL:
			fileWriter << "LPD " << instance.profile.direction.x << " " << instance.profile.direction.y << " " << instance.profile.direction.z << " "; // Direction
			fileWriter << instance.profile.color.x << " " << instance.profile.color.y << " " << instance.profile.color.z << " "; // Color
			fileWriter << instance.profile.intensity_diff << " " << instance.profile.intensity_spec << " " << instance.profile.intensity_ambt << "\n"; // Intensities
			break;
		default:
			std::cout << "You forget to register a new type of light source!" << std::endl;
			return false;
		}
		fileWriter << "L\n"; // End of instance
	}

	fileWriter.close();

	std::cout << "Map ";
	std::cout << addrstr;
	std::cout << " save success." << std::endl;
	return true;
}
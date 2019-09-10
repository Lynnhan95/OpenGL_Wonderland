#pragma once
#include <map>
#include <iostream>
#include "Loader/InitShader.h"  // Functions for loading shaders from text files
#include "SimParameters.h"

// Predefined shader groups
enum ShaderGroup
{
	SG_NONE = -1,
	BASIC = 0,
	LIT,
	STROKE,
	FISH,
	WATER_STATIC,
	SHADOW_MAP,
	SKYCUBE,
	PARTICLE,
	FISH_SHADOW,
	LEAVES,
	LEAVES_SHADOW
};

enum UniformType
{
	int1 = 0,
	flt1,
	vec2,
	vec3,
	mat3,
	mat4
};

class RuntimeShader
{
private:
	// Global shaders
	static std::map<ShaderGroup, GLuint> m_shaders;
	static RuntimeShader m_instance;
	static std::map<std::pair<GLuint, std::string>, int> m_uniformLoc;

	RuntimeShader() {}
	RuntimeShader(const RuntimeShader &cpy) {}

public:
	static RuntimeShader instance();
	void Init_Shaders();
	void Clear_Shaders();
	void Reload_Shaders();
	GLuint GetShader(ShaderGroup group);
	void SetUniform(GLuint shader, std::string name, UniformType type, const GLfloat* value);
};
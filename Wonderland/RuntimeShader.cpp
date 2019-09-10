#include "RuntimeShader.h"

RuntimeShader RuntimeShader::m_instance;
std::map<ShaderGroup, GLuint> RuntimeShader::m_shaders;
std::map<std::pair<GLuint, std::string>, int> RuntimeShader::m_uniformLoc;

RuntimeShader RuntimeShader::instance()
{
	if (&m_instance == nullptr)
	{
		m_instance = *new RuntimeShader();
	}
	return m_instance;
}

void RuntimeShader::Init_Shaders()
{
	m_shaders[ShaderGroup::BASIC] = InitShader(VS_BASIC.c_str(), FS_BASIC.c_str());
	if (m_shaders[ShaderGroup::BASIC] == -1)
	{
		glClearColor(1.0f, 0.0f, 1.0f, 0.0f);
		std::cout << "Basic shader group failed to init." << std::endl;
	}

	m_shaders[ShaderGroup::LIT] = InitShader(VS_LIT.c_str(), FS_LIT.c_str());
	if (m_shaders[ShaderGroup::LIT] == -1)
	{
		glClearColor(1.0f, 0.0f, 1.0f, 0.0f);
		std::cout << "Lit shader group failed to init." << std::endl;
	}

	m_shaders[ShaderGroup::STROKE] = InitShader(VS_STROKE.c_str(), GS_STROKE.c_str(), FS_STROKE.c_str());
	if (m_shaders[ShaderGroup::STROKE] == -1)
	{
		glClearColor(1.0f, 0.0f, 1.0f, 0.0f);
		std::cout << "Stroke shader group failed to init." << std::endl;
	}

	m_shaders[ShaderGroup::FISH] = InitShader(VS_LIT_FISH.c_str(), FS_LIT.c_str());
	if (m_shaders[ShaderGroup::BASIC] == -1)
	{
		glClearColor(1.0f, 0.0f, 1.0f, 0.0f);
		std::cout << "Fish shader group failed to init." << std::endl;
	}

	m_shaders[ShaderGroup::WATER_STATIC] = InitShader(VS_LIT_WATER.c_str(), FS_LIT.c_str());
	if (m_shaders[ShaderGroup::WATER_STATIC] == -1)
	{
		glClearColor(1.0f, 0.0f, 1.0f, 0.0f);
		std::cout << "Static water shader group failed to init." << std::endl;
	}

	m_shaders[ShaderGroup::SHADOW_MAP] = InitShader(VS_SHADOWMAP.c_str(), FS_SHADOWMAP.c_str());
	if (m_shaders[ShaderGroup::SHADOW_MAP] == -1)
	{
		glClearColor(1.0f, 0.0f, 1.0f, 0.0f);
		std::cout << "Shadow map shader group failed to init." << std::endl;
	}

	m_shaders[ShaderGroup::SKYCUBE] = InitShader(VS_SKYCUBE.c_str(), FS_SKYCUBE.c_str());
	if (m_shaders[ShaderGroup::SKYCUBE] == -1)
	{
		glClearColor(1.0f, 0.0f, 1.0f, 0.0f);
		std::cout << "Skycube shader group failed to init." << std::endl;
	}

	m_shaders[ShaderGroup::PARTICLE] = InitShader(VS_PARTICLE_BASIC.c_str(), FS_PARTICLE_BASIC.c_str());
	if (m_shaders[ShaderGroup::PARTICLE] == -1)
	{
		glClearColor(1.0f, 0.0f, 1.0f, 0.0f);
		std::cout << "Particle shader group failed to init." << std::endl;
	}

	m_shaders[ShaderGroup::FISH_SHADOW] = InitShader(VS_SHADOWMAP_FISH.c_str(), FS_SHADOWMAP.c_str());
	if (m_shaders[ShaderGroup::FISH_SHADOW] == -1)
	{
		glClearColor(1.0f, 0.0f, 1.0f, 0.0f);
		std::cout << "Fish shadow shader group failed to init." << std::endl;
	}

	m_shaders[ShaderGroup::LEAVES] = InitShader(VS_LIT_LEAVES.c_str(), FS_LIT.c_str());
	if (m_shaders[ShaderGroup::LEAVES] == -1)
	{
		glClearColor(1.0f, 0.0f, 1.0f, 0.0f);
		std::cout << "Leaves shader group failed to init." << std::endl;
	}

	m_shaders[ShaderGroup::LEAVES_SHADOW] = InitShader(VS_SHADOWMAP_LEAVES.c_str(), FS_SHADOWMAP.c_str());
	if (m_shaders[ShaderGroup::LEAVES_SHADOW] == -1)
	{
		glClearColor(1.0f, 0.0f, 1.0f, 0.0f);
		std::cout << "Leaves shadow shader group failed to init." << std::endl;
	}
}

void RuntimeShader::Clear_Shaders()
{
	auto itr = m_shaders.begin();
	while (itr != m_shaders.end())
	{
		glDeleteShader(itr->second);
		itr++;
	}

	m_shaders.clear();
	m_uniformLoc.clear();
}

void RuntimeShader::Reload_Shaders()
{
	Clear_Shaders();
	Init_Shaders();
}

GLuint RuntimeShader::GetShader(ShaderGroup group)
{
	return m_shaders[group];
}

void RuntimeShader::SetUniform(GLuint shader, std::string name, UniformType type, const GLfloat* value)
{
	std::pair<GLuint, std::string> key = std::make_pair(shader, name);
	if (m_uniformLoc.find(key) == m_uniformLoc.end())
	{
		m_uniformLoc[key] = glGetUniformLocation(shader, name.c_str());
	}
	int loc = m_uniformLoc[key];
	if (loc != -1)
	{
		switch (type)
		{
		case UniformType::int1:
			glUniform1i(loc, (int)*value);
			break;
		case UniformType::flt1:
			glUniform1f(loc, *value);
			break;
		case UniformType::vec2:
			glUniform2fv(loc, 1, value);
			break;
		case UniformType::vec3:
			glUniform3fv(loc, 1, value);
			break;
		case UniformType::mat3:
			glUniformMatrix3fv(loc, 1, false, value);
			break;
		case UniformType::mat4:
			glUniformMatrix4fv(loc, 1, false, value);
			break;
		default:
			std::cout << "Wrong uniform type." << std::endl;
		}
	}
	//else
	//{
	//	std::cout << "Incorrect uniform name: " << name << std::endl;
	//}
}
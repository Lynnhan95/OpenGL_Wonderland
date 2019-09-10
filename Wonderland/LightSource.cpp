#include "LightSource.h"

int LightSource::m_point_light_index = 0;
int LightSource::m_directional_light_index = 0;

LightSource::LightSource(LightProfile profile)
{
	this->profile = profile;
	m_light_index = -1;
	position = glm::vec3(0, 0, 0);
	m_i_handles = *new std::vector<Handle>();
}

void LightSource::Apply(GLuint const shader)
{
	// Assign index
	if (m_light_index < 0)
	{
		if (profile.type == LightSource_Type::LS_POINT)
		{
			m_light_index = m_point_light_index;
			m_point_light_index++;
		}
		else if (profile.type == LightSource_Type::DIRECTIONAL)
		{
			m_light_index = m_directional_light_index;
			m_directional_light_index++;
		}
	}

	// Set uniform values
	if (profile.type == LightSource_Type::LS_POINT && m_light_index >= 0 && m_light_index < MAX_POINT_LIGHT)
	{
		RuntimeShader::instance().SetUniform(shader, "pointLights[" + std::to_string(m_light_index) + "].position", UniformType::vec3, glm::value_ptr(position));
		RuntimeShader::instance().SetUniform(shader, "pointLights[" + std::to_string(m_light_index) + "].attenuation", UniformType::flt1, &profile.attenuation);
		RuntimeShader::instance().SetUniform(shader, "pointLights[" + std::to_string(m_light_index) + "].color", UniformType::vec3, glm::value_ptr(profile.color));
		RuntimeShader::instance().SetUniform(shader, "pointLights[" + std::to_string(m_light_index) + "].radius", UniformType::flt1, &profile.radius);
		RuntimeShader::instance().SetUniform(shader, "pointLights[" + std::to_string(m_light_index) + "].intensity_ambt", UniformType::flt1, &profile.intensity_ambt);
		RuntimeShader::instance().SetUniform(shader, "pointLights[" + std::to_string(m_light_index) + "].intensity_diff", UniformType::flt1, &profile.intensity_diff);
		RuntimeShader::instance().SetUniform(shader, "pointLights[" + std::to_string(m_light_index) + "].intensity_spec", UniformType::flt1, &profile.intensity_spec);
		float softEdge = profile.soft_edge ? 1 : 0;
		RuntimeShader::instance().SetUniform(shader, "pointLights[" + std::to_string(m_light_index) + "].soft_edge", UniformType::int1, &softEdge);
		RuntimeShader::instance().SetUniform(shader, "pointLights[" + std::to_string(m_light_index) + "].soft_ratio", UniformType::flt1, &profile.soft_ratio);
	}
	else if (profile.type == LightSource_Type::DIRECTIONAL && m_light_index >= 0 && m_light_index < MAX_DIRECTIONAL_LIGHT)
	{
		RuntimeShader::instance().SetUniform(shader, "dir_light_color_" + std::to_string(m_light_index), UniformType::vec3, glm::value_ptr(profile.color));
		RuntimeShader::instance().SetUniform(shader, "dir_light_dir_" + std::to_string(m_light_index), UniformType::vec3, glm::value_ptr(-glm::normalize(profile.direction)));
		RuntimeShader::instance().SetUniform(shader, "dir_light_iambt_" + std::to_string(m_light_index), UniformType::flt1, &profile.intensity_ambt);
		RuntimeShader::instance().SetUniform(shader, "dir_light_idiff_" + std::to_string(m_light_index), UniformType::flt1, &profile.intensity_diff);
		RuntimeShader::instance().SetUniform(shader, "dir_light_ispec_" + std::to_string(m_light_index), UniformType::flt1, &profile.intensity_spec);
	}
}

void LightSource::RefreshIndex()
{
	if (profile.type == LightSource_Type::LS_POINT)
	{
		m_light_index = m_point_light_index;
		m_point_light_index++;
	}
	else if (profile.type == LightSource_Type::DIRECTIONAL)
	{
		m_light_index = m_directional_light_index;
		m_directional_light_index++;
	}
}

void LightSource::DrawStroke(Camera* const camera, GLuint dummyVAO)
{
	if (profile.type == LightSource_Type::LS_POINT)
	{
		glBindVertexArray(dummyVAO); // This is used to activate shader only

		//glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINTS);

		GLuint stroke_shader = RuntimeShader::instance().GetShader(ShaderGroup::STROKE);

		//for (int i = 0; i < base->submeshCount(); i++)
		//{
			glUseProgram(stroke_shader);

			// Model matrix
			glm::mat4 M = glm::translate(position);

			// Set space transform uniform variable
			glm::mat4 PVM = camera->PV_matrix * M;
			RuntimeShader::instance().SetUniform(stroke_shader,
				"PVM", UniformType::mat4, glm::value_ptr(PVM));

			float boundSize = profile.radius;
			glm::vec3 bBmax(boundSize, boundSize, boundSize);
			RuntimeShader::instance().SetUniform(stroke_shader,
				"BbMax", UniformType::vec3, glm::value_ptr(bBmax));

			glm::vec3 bBmin(-boundSize, -boundSize, -boundSize);
			RuntimeShader::instance().SetUniform(stroke_shader,
				"BbMin", UniformType::vec3, glm::value_ptr(bBmin));

			//RuntimeShader::instance().SetUniform(stroke_shader,
			//	"stroke_width", UniformType::flt1, &STROKE_WIDTH);
			glLineWidth(STROKE_WIDTH);

			//float canBeBlocked = 0;
			// Blocked Part
			//RuntimeShader::instance().SetUniform(stroke_shader,
			//	"stroke_color", UniformType::vec3, glm::value_ptr(STROKE_COLOR_BLOCKED));
			//RuntimeShader::instance().SetUniform(stroke_shader,
			//	"can_be_blocked", UniformType::int1, &canBeBlocked);
			//base->mesh_data.mSubmesh[i].DrawSubmesh();

			//canBeBlocked = 1;
			// Unblocked Part
			//RuntimeShader::instance().SetUniform(stroke_shader,
			//	"stroke_color", UniformType::vec3, glm::value_ptr(STROKE_COLOR));
			//RuntimeShader::instance().SetUniform(stroke_shader,
			//	"can_be_blocked", UniformType::int1, &canBeBlocked);
			//base->mesh_data.mSubmesh[i].DrawSubmesh();

			RuntimeShader::instance().SetUniform(stroke_shader,
				"stroke_color", UniformType::vec3, glm::value_ptr(STROKE_COLOR));

			//base->mesh_data.mSubmesh[i].DrawSubmesh();
			glDrawArrays(GL_POINTS, 0, 1); // We build the stroke box from a single point
		//}

		glBindVertexArray(0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

std::string LightSource::GetIObjectName()
{
	return name;
}

std::vector<Handle> LightSource::GetIHandle()
{
	if (m_i_handles.size() == 0)
	{
		Handle* h = new Handle();
		h->type = HandleType::COL3;
		h->name = "Color";
		h->v1 = glm::value_ptr(profile.color);
		m_i_handles.push_back(*h);

		if (profile.type == LightSource_Type::LS_POINT)
		{
			h = new Handle();
			h->type = HandleType::VEC3;
			h->opType = OperationType::OT_INPUT;
			h->name = "Position";
			h->v1 = glm::value_ptr(position);
			m_i_handles.push_back(*h);

			h = new Handle();
			h->type = HandleType::FLT1;
			h->opType = OperationType::OT_INPUT;
			h->name = "Radius";
			h->v1 = &profile.radius;
			m_i_handles.push_back(*h);

			h = new Handle();
			h->type = HandleType::FLT1;
			h->opType = OperationType::OT_INPUT;
			h->name = "Intensity";
			h->v1 = &profile.attenuation;
			m_i_handles.push_back(*h);

			h = new Handle();
			h->type = HandleType::HT_BOOL;
			h->name = "Soft Edge";
			h->b1 = &profile.soft_edge;
			m_i_handles.push_back(*h);

			h = new Handle();
			h->type = HandleType::FLT1;
			h->opType = OperationType::SLIDE;
			h->name = "Soft Ratio";
			h->v1 = &profile.soft_ratio;
			h->min = 0.0f;
			h->max = 1.0f;
			m_i_handles.push_back(*h);
		}
		else if (profile.type == LightSource_Type::DIRECTIONAL)
		{
			h = new Handle();
			h->type = HandleType::VEC3;
			h->opType = OperationType::SLIDE;
			h->max = 1;
			h->min = -1;
			h->name = "Direction";
			h->v1 = glm::value_ptr(profile.direction);
			m_i_handles.push_back(*h);
		}

		h = new Handle();
		h->type = HandleType::FLT1;
		h->opType = OperationType::SLIDE;
		h->name = "Ambient Intensity";
		h->v1 = &profile.intensity_ambt;
		h->min = 0;
		h->max = 1;
		m_i_handles.push_back(*h);

		h = new Handle();
		h->type = HandleType::FLT1;
		h->opType = OperationType::SLIDE;
		h->name = "Diffuse Intensity";
		h->v1 = &profile.intensity_diff;
		h->min = 0;
		h->max = 1;
		m_i_handles.push_back(*h);

		h = new Handle();
		h->type = HandleType::FLT1;
		h->opType = OperationType::SLIDE;
		h->name = "Specular Intensity";
		h->v1 = &profile.intensity_spec;
		h->min = 0;
		h->max = 1;
		m_i_handles.push_back(*h);
	}

	return m_i_handles;
}

void LightSource::ClearHandles()
{
	m_i_handles.clear();
}

// Hard coded profile initializer
void Init_Light_Profiles(std::map<std::string, LightProfile>& gLightSource_profiles)
{
	gLightSource_profiles.clear();

	LightProfile* profile = new LightProfile();
	profile->type = LightSource_Type::LS_POINT;
	gLightSource_profiles["PointLight1"] = *profile;

	profile = new LightProfile();
	profile->type = LightSource_Type::DIRECTIONAL;
	gLightSource_profiles["DirectionalLight1"] = *profile;
}
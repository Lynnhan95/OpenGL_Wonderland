#include "Prefab.h"
#include "LightMap.h"

// Once a default argument has been given in a declaration or definition, you cannot redefine that argument, 
// even to the same value. However, you can add default arguments not given in previous declarations.
// https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.3.0/com.ibm.zos.v2r3.cbclx01/cplr238.htm
void ModelInstance::Draw(Camera* const cam, std::vector<LightSource>& lights, SimParams* simParams, LightMap* shadowMap, GLuint shader_override)
{
	glBindVertexArray(base->mesh_data.mVao);

	glEnable(GL_DEPTH_TEST);

	for (int i = 0; i < base->submeshCount(); i++)
	{
		if (base->materials[i].draw_back)
		{
			glDisable(GL_CULL_FACE);
		}
		else
		{
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK); // Default culling
		}

		// Draw transparent object
		if (!base->materials[i].isTransparent)
		{
			glDisable(GL_BLEND);
		}
		else // Enable blending
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			//glBlendFunc(GL_ONE, GL_ONE);
		}

		if (shader_override == -1)
		{
			glUseProgram(base->materials[i].shader_program);
		}
		else
		{
			glUseProgram(shader_override);
		}

		// Light source must be set first
		for (int il = 0; il < lights.size(); il++)
		{
			lights[il].Apply(base->materials[i].shader_program);
		}

		float lightcount = lights.size() - 1; // 1 directional light
		RuntimeShader::instance().SetUniform(base->materials[i].shader_program,
			"active_pointlight_num", UniformType::int1, &lightcount);

		// Albedo
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, base->materials[i].albedo_id);
		float unit0 = 0;
		RuntimeShader::instance().SetUniform(base->materials[i].shader_program,
			"albedo", UniformType::int1, &unit0); // we bound our texture to texture unit 0

		// Normal
		//glActiveTexture(GL_TEXTURE1);
		//glBindTexture(GL_TEXTURE_2D, model.base->materials[i].normal_id[1]);

		// Specular
		//glActiveTexture(GL_TEXTURE2);
		//glBindTexture(GL_TEXTURE_2D, model.base->materials[i].spec_id[1]);
		
		if (shadowMap != nullptr)
		{
			float unit1 = 1;
			shadowMap->ApplyLightMap(shader_override == -1 ? base->materials[i].shader_program : shader_override, GL_TEXTURE1, unit1);
		}

		// Model matrix
		glm::mat4 T = glm::translate(pos);
		glm::mat4 S = glm::scale(scale*glm::vec3(base->mesh_data.mScaleFactor));
		glm::mat4 R = glm::rotate(rot.x * D2R, glm::vec3(1, 0, 0))*glm::rotate(rot.y * D2R, glm::vec3(0, 1, 0))*glm::rotate(rot.z * D2R, glm::vec3(0, 0, 1));
		glm::mat4 M = T * R * S;

		glm::mat3 M_normal = glm::mat3(R) * glm::inverse(glm::mat3(S));
		glm::mat4 PVM = cam->PV_matrix * M;

		// Set space transform uniform variables
		
		RuntimeShader::instance().SetUniform(base->materials[i].shader_program,
			"PVM", UniformType::mat4, glm::value_ptr(PVM));
		RuntimeShader::instance().SetUniform(base->materials[i].shader_program,
			"M", UniformType::mat4, glm::value_ptr(M));
		RuntimeShader::instance().SetUniform(base->materials[i].shader_program,
			"Normal_M", UniformType::mat3, glm::value_ptr(M_normal));
		RuntimeShader::instance().SetUniform(base->materials[i].shader_program,
			"viewPos", UniformType::vec3, glm::value_ptr(cam->position()));

		// Timer

		float glfw_time = glfwGetTime();
		RuntimeShader::instance().SetUniform(base->materials[i].shader_program,
			"time", UniformType::flt1, &glfw_time);

		// Animation related properties

		if (simParams != nullptr)
		{
			RuntimeShader::instance().SetUniform(base->materials[i].shader_program,
				"world_pos", UniformType::vec3, glm::value_ptr(pos));
			RuntimeShader::instance().SetUniform(base->materials[i].shader_program,
				"wind_dir", UniformType::vec3, glm::value_ptr(simParams->wind_direction));
			RuntimeShader::instance().SetUniform(base->materials[i].shader_program,
				"wind_strength", UniformType::flt1, &simParams->wind_strength);
		}

		// Set material properties

		RuntimeShader::instance().SetUniform(base->materials[i].shader_program,
			"transparency", UniformType::flt1, &base->materials[i].transparency);
		float useTexColor = (float)base->materials[i].useTextureColor;
		RuntimeShader::instance().SetUniform(base->materials[i].shader_program,
			"use_texture_color", UniformType::int1, &useTexColor);
		RuntimeShader::instance().SetUniform(base->materials[i].shader_program,
			"Ambt_Color", UniformType::vec3, glm::value_ptr(base->materials[i].ambt_color));
		RuntimeShader::instance().SetUniform(base->materials[i].shader_program,
			"Diff_Color", UniformType::vec3, glm::value_ptr(base->materials[i].main_color));
		RuntimeShader::instance().SetUniform(base->materials[i].shader_program,
			"Spec_Color", UniformType::vec3, glm::value_ptr(base->materials[i].spec_color));
		RuntimeShader::instance().SetUniform(base->materials[i].shader_program,
			"shiness", UniformType::flt1, &base->materials[i].shiness);

		// Draw mesh
		base->mesh_data.mSubmesh[i].DrawSubmesh();
	}

	glBindVertexArray(0);

	glDisable(GL_CULL_FACE);
}

void ModelInstance::DrawDepth(GLuint shader_override, float* matrix_ptr, SimParams* simParams)
{
	glBindVertexArray(base->mesh_data.mVao);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	//glCullFace(GL_FRONT); // Improve 'Peter panning'

	for (int i = 0; i < base->submeshCount(); i++)
	{
		GLuint shader = shader_override;
		// Apply vertex animation
		if (base->materials[i].has_special_shader)
		{
			shader = base->materials[i].special_shader_program;
		}	

		glUseProgram(shader);

		// Discard transparent frag from shadow map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, base->materials[i].albedo_id);
		float unit0 = 0;
		RuntimeShader::instance().SetUniform(shader, "albedo", UniformType::int1, &unit0);

		glm::mat4 T = glm::translate(pos);
		glm::mat4 S = glm::scale(scale*glm::vec3(base->mesh_data.mScaleFactor));
		glm::mat4 R = glm::rotate(rot.x * D2R, glm::vec3(1, 0, 0))*glm::rotate(rot.y * D2R, glm::vec3(0, 1, 0))*glm::rotate(rot.z * D2R, glm::vec3(0, 0, 1));
		glm::mat4 M = T * R * S;

		RuntimeShader::instance().SetUniform(shader, "M", UniformType::mat4, glm::value_ptr(M));

		if (base->materials[i].has_special_shader)
		{
			float glfw_time = glfwGetTime();
			RuntimeShader::instance().SetUniform(shader, "time", UniformType::flt1, &glfw_time);
			RuntimeShader::instance().SetUniform(shader, "lightSpaceMatrix", UniformType::mat4, matrix_ptr);

			if (simParams != nullptr)
			{
				RuntimeShader::instance().SetUniform(shader, "world_pos", UniformType::vec3, glm::value_ptr(pos));
				RuntimeShader::instance().SetUniform(shader, "wind_dir", UniformType::vec3, glm::value_ptr(simParams->wind_direction));
				RuntimeShader::instance().SetUniform(shader, "wind_strength", UniformType::flt1, &simParams->wind_strength);
			}
		}

		base->mesh_data.mSubmesh[i].DrawSubmesh();
	}

	glEnable(GL_CULL_FACE);
	glBindVertexArray(0);
}

void ModelInstance::DrawStroke(Camera* const cam)
{
	glBindVertexArray(base->mesh_data.mVao);

	//glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glPolygonMode(GL_FRONT_AND_BACK, GL_POINTS);

	GLuint stroke_shader = RuntimeShader::instance().GetShader(ShaderGroup::STROKE);

	//for (int i = 0; i < base->submeshCount(); i++)
	//{
		glUseProgram(stroke_shader);

		// Model matrix
		glm::mat4 T = glm::translate(pos);
		glm::mat4 S = glm::scale(scale*glm::vec3(base->mesh_data.mScaleFactor));
		glm::mat4 R = glm::rotate(rot.x * D2R, glm::vec3(1, 0, 0))*glm::rotate(rot.y * D2R, glm::vec3(0, 1, 0))*glm::rotate(rot.z * D2R, glm::vec3(0, 0, 1));
		glm::mat4 M = T * R * S;

		// Set space transform uniform variable
		glm::mat4 PVM = cam->PV_matrix * M;
		RuntimeShader::instance().SetUniform(stroke_shader,
			"PVM", UniformType::mat4, glm::value_ptr(PVM));

		glm::vec3 bBmax(base->mesh_data.mBbMax.x, base->mesh_data.mBbMax.y, base->mesh_data.mBbMax.z);
		RuntimeShader::instance().SetUniform(stroke_shader,
			"BbMax", UniformType::vec3, glm::value_ptr(bBmax));

		glm::vec3 bBmin(base->mesh_data.mBbMin.x, base->mesh_data.mBbMin.y, base->mesh_data.mBbMin.z);
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

std::string ModelInstance::GetIObjectName()
{
	return name;
}

std::vector<Handle> ModelInstance::GetIHandle()
{
	if (m_i_handles.size() == 0)
	{
		Handle* h = new Handle();
		h->type = HandleType::VEC3;
		h->opType = OperationType::OT_INPUT;
		h->name = "Position";
		h->v1 = glm::value_ptr(pos);
		m_i_handles.push_back(*h);

		h = new Handle();
		h->type = HandleType::VEC3;
		h->opType = OperationType::OT_INPUT;
		h->name = "Rotation";
		h->v1 = glm::value_ptr(rot);
		m_i_handles.push_back(*h);

		h = new Handle();
		h->type = HandleType::VEC3;
		h->opType = OperationType::OT_INPUT;
		h->name = "Scale";
		h->v1 = glm::value_ptr(scale);
		m_i_handles.push_back(*h);

		h = new Handle();
		h->type = HandleType::HT_BOOL;
		h->name = "Cast Shadow";
		h->b1 = &base->cast_shadow;
		m_i_handles.push_back(*h);

		for (int i = 0; i < base->materials.size(); i++)
		{
			h = new Handle();
			h->type = HandleType::COL3;
			h->name = "Main Color " + std::to_string(i); // Duplicate name will cause ImGui crash
			h->v1 = glm::value_ptr(base->materials[i].main_color);
			m_i_handles.push_back(*h);

			h = new Handle();
			h->type = HandleType::COL3;
			h->name = "Ambient Color " + std::to_string(i);
			h->v1 = glm::value_ptr(base->materials[i].ambt_color);
			m_i_handles.push_back(*h);

			h = new Handle();
			h->type = HandleType::COL3;
			h->name = "Specular Color " + std::to_string(i);
			h->v1 = glm::value_ptr(base->materials[i].spec_color);
			m_i_handles.push_back(*h);

			h = new Handle();
			h->type = HandleType::FLT1;
			h->opType = OperationType::SLIDE;
			h->name = "Shiness " + std::to_string(i);
			h->v1 = &base->materials[i].shiness;
			h->min = 0;
			h->max = 100;
			m_i_handles.push_back(*h);

			h = new Handle();
			h->type = HandleType::HT_BOOL;
			h->name = "Use Texture Color " + std::to_string(i);;
			h->b1 = &base->materials[i].useTextureColor;
			m_i_handles.push_back(*h);

			h = new Handle();
			h->type = HandleType::HT_BOOL;
			h->name = "Transparent " + std::to_string(i);
			h->b1 = &base->materials[i].isTransparent;
			m_i_handles.push_back(*h);

			h = new Handle();
			h->type = HandleType::FLT1;
			h->name = "Transparency " + std::to_string(i);
			h->v1 = &base->materials[i].transparency;
			m_i_handles.push_back(*h);
		}
	}

	return m_i_handles;
}

void ModelInstance::ClearHandles()
{
	m_i_handles.clear();
}

void Init_Resource_Paths(std::map<std::string, ResourcePath>& gResource_paths)
{
	ResourcePath* rp = new ResourcePath();
	rp->model_path = TREE1_MESH_NAME;
	rp->albedo_paths.push_back(DEFAULT_TEXTURE_NAME);
	rp->albedo_paths.push_back(DEFAULT_TEXTURE_NAME);
	rp->groups.push_back(ShaderGroup::LEAVES);
	rp->groups.push_back(ShaderGroup::LIT);
	rp->draw_back.push_back(false);
	rp->draw_back.push_back(false);
	rp->has_sp_shader.push_back(true);
	rp->has_sp_shader.push_back(false);
	rp->special_groups.push_back(ShaderGroup::LEAVES_SHADOW);
	rp->special_groups.push_back(ShaderGroup::SG_NONE);
	rp->cast_shadow = true;
	gResource_paths["Tree1"] = *rp;

	rp = new ResourcePath();
	rp->model_path = TREE2_MESH_NAME;
	rp->albedo_paths.push_back(DEFAULT_TEXTURE_NAME);
	rp->albedo_paths.push_back(DEFAULT_TEXTURE_NAME);
	rp->groups.push_back(ShaderGroup::LIT);
	rp->groups.push_back(ShaderGroup::LEAVES);
	rp->draw_back.push_back(false);
	rp->draw_back.push_back(false);
	rp->has_sp_shader.push_back(false);
	rp->has_sp_shader.push_back(true);
	rp->special_groups.push_back(ShaderGroup::SG_NONE);
	rp->special_groups.push_back(ShaderGroup::LEAVES_SHADOW);
	rp->cast_shadow = true;
	gResource_paths["Tree2"] = *rp;

	rp = new ResourcePath();
	rp->model_path = TREE3_MESH_NAME;
	rp->albedo_paths.push_back(DEFAULT_TEXTURE_NAME);
	rp->albedo_paths.push_back(DEFAULT_TEXTURE_NAME);
	rp->groups.push_back(ShaderGroup::LIT);
	rp->groups.push_back(ShaderGroup::LEAVES);
	rp->draw_back.push_back(false);
	rp->draw_back.push_back(false);
	rp->has_sp_shader.push_back(false);
	rp->has_sp_shader.push_back(true);
	rp->special_groups.push_back(ShaderGroup::SG_NONE);
	rp->special_groups.push_back(ShaderGroup::LEAVES_SHADOW);
	rp->cast_shadow = true;
	gResource_paths["Tree3"] = *rp;

	rp = new ResourcePath();
	rp->model_path = FISH1_MESH_NAME;
	rp->albedo_paths.push_back(FISH1_ALBEDO_0_NAME);
	rp->groups.push_back(ShaderGroup::FISH);
	rp->special_groups.push_back(ShaderGroup::FISH_SHADOW);
	rp->draw_back.push_back(false);
	rp->has_sp_shader.push_back(true);
	rp->cast_shadow = true;
	gResource_paths["Fish1"] = *rp;

	rp = new ResourcePath();
	rp->model_path = GROUND_MESH_NAME;
	rp->albedo_paths.push_back(GROUND_ALBEDO_0_NAME);
	rp->groups.push_back(ShaderGroup::LIT);
	rp->draw_back.push_back(false);
	//rp->albedo_paths.push_back(DEFAULT_TEXTURE_NAME);
	//rp->groups.push_back(ShaderGroup::LIT);
	//rp->draw_back.push_back(false);
	//rp->albedo_paths.push_back(DEFAULT_TEXTURE_NAME);
	//rp->groups.push_back(ShaderGroup::LIT);
	//rp->draw_back.push_back(false);
	rp->cast_shadow = true;
	gResource_paths["Ground1"] = *rp;

	rp = new ResourcePath();
	rp->model_path = MOUNTAIN1_MESH_NAME;
	rp->albedo_paths.push_back(MOUNTAIN1_ALBEDO_0_NAME);
	rp->groups.push_back(ShaderGroup::LIT);
	rp->draw_back.push_back(false);
	rp->cast_shadow = true;
	gResource_paths["Mountain1"] = *rp;

	rp = new ResourcePath();
	rp->model_path = MOUNTAIN2_MESH_NAME;
	rp->albedo_paths.push_back(MOUNTAIN2_ALBEDO_0_NAME);
	rp->groups.push_back(ShaderGroup::LIT);
	rp->draw_back.push_back(false);
	rp->cast_shadow = true;
	gResource_paths["Mountain2"] = *rp;

	rp = new ResourcePath();
	rp->model_path = CLOUD1_MESH_NAME;
	rp->albedo_paths.push_back(MOUNTAIN2_ALBEDO_0_NAME);
	rp->groups.push_back(ShaderGroup::LIT);
	rp->draw_back.push_back(false);
	rp->cast_shadow = true;
	gResource_paths["Cloud1"] = *rp;

	rp = new ResourcePath();
	rp->model_path = BALLON1_MESH_NAME;
	rp->albedo_paths.push_back(DEFAULT_TEXTURE_NAME);
	rp->groups.push_back(ShaderGroup::LIT);
	rp->draw_back.push_back(false);
	rp->albedo_paths.push_back(DEFAULT_TEXTURE_NAME);
	rp->groups.push_back(ShaderGroup::LIT);
	rp->draw_back.push_back(false);
	rp->cast_shadow = true;
	gResource_paths["Ballon1"] = *rp;

	rp = new ResourcePath();
	rp->model_path = BOAT1_MESH_NAME;
	rp->albedo_paths.push_back(DEFAULT_TEXTURE_NAME);
	rp->groups.push_back(ShaderGroup::LIT);
	rp->draw_back.push_back(false);
	rp->albedo_paths.push_back(DEFAULT_TEXTURE_NAME);
	rp->groups.push_back(ShaderGroup::LEAVES);
	rp->draw_back.push_back(true);
	rp->albedo_paths.push_back(DEFAULT_TEXTURE_NAME);
	rp->groups.push_back(ShaderGroup::LIT);
	rp->draw_back.push_back(false);
	rp->cast_shadow = true;
	rp->has_sp_shader.push_back(false);
	rp->has_sp_shader.push_back(true);
	rp->has_sp_shader.push_back(false);
	rp->special_groups.push_back(ShaderGroup::SG_NONE);
	rp->special_groups.push_back(ShaderGroup::LEAVES_SHADOW);
	rp->special_groups.push_back(ShaderGroup::SG_NONE);
	gResource_paths["Boat1"] = *rp;

	rp = new ResourcePath();
	rp->model_path = ROAD1_MESH_NAME;
	rp->albedo_paths.push_back(DEFAULT_TEXTURE_NAME);
	rp->groups.push_back(ShaderGroup::LIT);
	rp->draw_back.push_back(false);
	rp->cast_shadow = true;
	gResource_paths["Road1"] = *rp;

	rp = new ResourcePath();
	rp->model_path = ROAD2_MESH_NAME;
	rp->albedo_paths.push_back(DEFAULT_TEXTURE_NAME);
	rp->groups.push_back(ShaderGroup::LIT);
	rp->draw_back.push_back(false);
	rp->cast_shadow = true;
	gResource_paths["Road2"] = *rp;

	rp = new ResourcePath();
	rp->model_path = WATER1_MESH_NAME;
	rp->albedo_paths.push_back(DEFAULT_TEXTURE_NAME);
	rp->groups.push_back(ShaderGroup::WATER_STATIC);
	rp->draw_back.push_back(false);
	rp->cast_shadow = false;
	gResource_paths["Water1"] = *rp;

	rp = new ResourcePath();
	rp->model_path = WATER2_MESH_NAME;
	rp->albedo_paths.push_back(DEFAULT_TEXTURE_NAME);
	rp->groups.push_back(ShaderGroup::WATER_STATIC);
	rp->draw_back.push_back(false);
	rp->cast_shadow = false;
	gResource_paths["Water2"] = *rp;

	rp = new ResourcePath();
	rp->model_path = ROCK1_MESH_NAME;
	rp->albedo_paths.push_back(DEFAULT_TEXTURE_NAME);
	rp->groups.push_back(ShaderGroup::LIT);
	rp->draw_back.push_back(false);
	rp->cast_shadow = true;
	gResource_paths["Rock1"] = *rp;

	rp = new ResourcePath();
	rp->model_path = ROCK2_MESH_NAME;
	rp->albedo_paths.push_back(DEFAULT_TEXTURE_NAME);
	rp->groups.push_back(ShaderGroup::LIT);
	rp->draw_back.push_back(false);
	rp->cast_shadow = true;
	gResource_paths["Rock2"] = *rp;

	rp = new ResourcePath();
	rp->model_path = LADDER1_MESH_NAME;
	rp->albedo_paths.push_back(DEFAULT_TEXTURE_NAME);
	rp->groups.push_back(ShaderGroup::LIT);
	rp->draw_back.push_back(false);
	rp->cast_shadow = true;
	gResource_paths["Ladder1"] = *rp;

	rp = new ResourcePath();
	rp->model_path = HOUSE1_MESH_NAME;
	rp->albedo_paths.push_back(DEFAULT_TEXTURE_NAME);
	rp->groups.push_back(ShaderGroup::LIT);
	rp->albedo_paths.push_back(DEFAULT_TEXTURE_NAME);
	rp->groups.push_back(ShaderGroup::LIT);
	rp->albedo_paths.push_back(DEFAULT_TEXTURE_NAME);
	rp->groups.push_back(ShaderGroup::LIT);
	rp->draw_back.push_back(false);
	rp->draw_back.push_back(false);
	rp->draw_back.push_back(false);
	rp->cast_shadow = true;
	gResource_paths["House1"] = *rp;

	rp = new ResourcePath();
	rp->model_path = FLOWER1_MESH_NAME;
	rp->albedo_paths.push_back(DEFAULT_TEXTURE_NAME);
	rp->groups.push_back(ShaderGroup::LEAVES);
	rp->albedo_paths.push_back(DEFAULT_TEXTURE_NAME);
	rp->groups.push_back(ShaderGroup::LEAVES);
	rp->albedo_paths.push_back(DEFAULT_TEXTURE_NAME);
	rp->groups.push_back(ShaderGroup::LEAVES);
	rp->draw_back.push_back(true);
	rp->draw_back.push_back(true);
	rp->draw_back.push_back(true);
	rp->has_sp_shader.push_back(true);
	rp->has_sp_shader.push_back(true);
	rp->has_sp_shader.push_back(true);
	rp->special_groups.push_back(ShaderGroup::LEAVES_SHADOW);
	rp->special_groups.push_back(ShaderGroup::LEAVES_SHADOW);
	rp->special_groups.push_back(ShaderGroup::LEAVES_SHADOW);
	rp->cast_shadow = true;
	gResource_paths["Flower1"] = *rp;

	rp = new ResourcePath();
	rp->model_path = COW_MESH_NAME;
	rp->albedo_paths.push_back(DEFAULT_TEXTURE_NAME);
	rp->groups.push_back(ShaderGroup::LIT);
	rp->albedo_paths.push_back(DEFAULT_TEXTURE_NAME);
	rp->groups.push_back(ShaderGroup::LIT);
	rp->draw_back.push_back(false);
	rp->draw_back.push_back(false);
	rp->cast_shadow = true;
	gResource_paths["Cow1"] = *rp;

	rp = new ResourcePath();
	rp->model_path = DEAR_MESH_NAME;
	rp->albedo_paths.push_back(DEFAULT_TEXTURE_NAME);
	rp->groups.push_back(ShaderGroup::LIT);
	rp->albedo_paths.push_back(DEFAULT_TEXTURE_NAME);
	rp->groups.push_back(ShaderGroup::LIT);
	rp->albedo_paths.push_back(DEFAULT_TEXTURE_NAME);
	rp->groups.push_back(ShaderGroup::LIT);
	rp->draw_back.push_back(false);
	rp->draw_back.push_back(false);
	rp->draw_back.push_back(false);
	rp->cast_shadow = true;
	gResource_paths["Dear1"] = *rp;

	rp = new ResourcePath();
	rp->model_path = FIRE1_MESH_NAME;
	rp->albedo_paths.push_back(DEFAULT_TEXTURE_NAME);
	rp->groups.push_back(ShaderGroup::LIT);
	rp->draw_back.push_back(false);
	rp->cast_shadow = true;
	gResource_paths["FireHeap1"] = *rp;

	rp = new ResourcePath();
	rp->model_path = POND1_MESH_NAME;
	rp->albedo_paths.push_back(DEFAULT_TEXTURE_NAME);
	rp->groups.push_back(ShaderGroup::LIT);
	rp->draw_back.push_back(false);
	rp->albedo_paths.push_back(DEFAULT_TEXTURE_NAME);
	rp->groups.push_back(ShaderGroup::LIT);
	rp->draw_back.push_back(false);
	rp->albedo_paths.push_back(DEFAULT_TEXTURE_NAME);
	rp->groups.push_back(ShaderGroup::LIT);
	rp->draw_back.push_back(false);
	rp->cast_shadow = true;
	gResource_paths["Pond1"] = *rp;

	rp = new ResourcePath();
	rp->model_path = PLANE_MESH_NAME;
	rp->albedo_paths.push_back(FOG1_ALBEDO_0_NAME);
	rp->groups.push_back(ShaderGroup::PARTICLE);
	rp->draw_back.push_back(false);
	rp->cast_shadow = false;
	gResource_paths["Smoke1"] = *rp;

	rp = new ResourcePath();
	rp->model_path = PLANE_MESH_NAME;
	rp->albedo_paths.push_back(SNOW1_ALBEDO_0_NAME);
	rp->groups.push_back(ShaderGroup::PARTICLE);
	rp->draw_back.push_back(false);
	rp->cast_shadow = false;
	gResource_paths["SnowField1"] = *rp;

	rp = new ResourcePath();
	rp->model_path = PLANE_MESH_NAME;
	rp->albedo_paths.push_back(FIRE1_ALBEDO_0_NAME);
	rp->groups.push_back(ShaderGroup::PARTICLE);
	rp->draw_back.push_back(false);
	rp->cast_shadow = false;
	gResource_paths["Fire1"] = *rp;
}
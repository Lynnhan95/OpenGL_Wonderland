#include "LightMap.h"

// Update view volume bounding box
void ShadowBox::Update(Camera* cam)
{
	glm::vec3 hori_dir = cam->right();
	glm::vec3 vert_dir = glm::normalize(cam->up());
	glm::vec3 fwd_dir = cam->forward();

	// half values
	float tan_fov = tanf((cam->fov)*D2R);
	float far_width = SHADOW_DISTANCE * tan_fov;
	float far_height = far_width / window_aspect;
	float near_width = cam->near_clip * tan_fov;
	float near_height = near_width / window_aspect;

	glm::vec3 near_vert_dist = vert_dir * near_height;
	glm::vec3 near_hori_dist = hori_dir * near_width;
	glm::vec3 far_vert_dist = vert_dir * far_height;
	glm::vec3 far_hori_dist = hori_dir * far_width;

	// volume vertices
	glm::vec3 near_cn = cam->position() + fwd_dir * cam->near_clip;
	glm::vec3 near_lt = near_cn - near_hori_dist + near_vert_dist;
	glm::vec3 near_lb = near_cn - near_hori_dist - near_vert_dist;
	glm::vec3 near_rt = near_cn + near_hori_dist + near_vert_dist;
	glm::vec3 near_rb = near_cn + near_hori_dist - near_vert_dist;

	glm::vec3 far_cn = cam->position() + fwd_dir * (float)SHADOW_DISTANCE;
	glm::vec3 far_lt = far_cn - far_hori_dist + far_vert_dist;
	glm::vec3 far_lb = far_cn - far_hori_dist - far_vert_dist;
	glm::vec3 far_rt = far_cn + far_hori_dist + far_vert_dist;
	glm::vec3 far_rb = far_cn + far_hori_dist - far_vert_dist;

	float max_x = max(
		max(max(near_lt.x, near_lb.x), max(near_rt.x, near_rb.x)),
		max(max(far_lt.x, far_lb.x), max(far_rt.x, far_rb.x))
	);
	float max_z = max(
		max(max(near_lt.z, near_lb.z), max(near_rt.z, near_rb.z)),
		max(max(far_lt.z, far_lb.z), max(far_rt.z, far_rb.z))
	);
	float max_y = max(
		max(max(near_lt.y, near_lb.y), max(near_rt.y, near_rb.y)),
		max(max(far_lt.y, far_lb.y), max(far_rt.y, far_rb.y))
	);

	float min_x = min(
		min(min(near_lt.x, near_lb.x), min(near_rt.x, near_rb.x)),
		min(min(far_lt.x, far_lb.x), min(far_rt.x, far_rb.x))
	);
	float min_z = min(
		min(min(near_lt.z, near_lb.z), min(near_rt.z, near_rb.z)),
		min(min(far_lt.z, far_lb.z), min(far_rt.z, far_rb.z))
	);
	float min_y = min(
		min(min(near_lt.y, near_lb.y), min(near_rt.y, near_rb.y)),
		min(min(far_lt.y, far_lb.y), min(far_rt.y, far_rb.y))
	);

	float l = (max_x - min_x) / 2;
	float w = (max_z - min_z) / 2;
	float h = (max_y - min_y) / 2;
	center = glm::vec3((max_x + min_x) / 2, 0, (max_z + min_z) / 2);
	vertA = glm::vec3(l, h, w);
	vertB = glm::vec3(-l, -h, -w);
}

void LightMap::InitializeLightMap(unsigned int res_x, unsigned int res_y)
{
	if (type == LightMap_Type::LT_SHADOW_MAP)
	{
		glGenFramebuffers(1, &lightMapFBO);

		// create light texture
		glGenTextures(1, &texture_id);
		glBindTexture(GL_TEXTURE_2D, texture_id);

		//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, res_x, res_y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, res_x, res_y); // This fixed the abnormal sampling
																			   // probably caused by mipmap.

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		// Resolve oversampling
		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		// attach texture as FBO's depth buffer		
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, lightMapFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture_id, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		resolution_x = res_x;
		resolution_y = res_y;

		shadowBox = new ShadowBox();
	}
}

void LightMap::GenerateLightMap(Camera* cam, LightSource const light, std::vector<ModelInstance>& const mesh, SimParams* simParams)
{
	// Render depth of scene to texture (from light's perspective)
	if (type == LightMap_Type::LT_SHADOW_MAP)
	{
		shadowBox->Update(cam);

		// Compute ortho light projection
		lightProjection = glm::ortho<float>(shadowBox->vertB.x, shadowBox->vertA.x,
			shadowBox->vertB.y, shadowBox->vertA.y,
			shadowBox->vertB.z, shadowBox->vertA.z);
		glm::mat4 lightView = glm::lookAt(-normalize(light.profile.direction) + shadowBox->center, shadowBox->center, UP);
		lightSpaceMatrix = lightProjection * lightView;

		glUseProgram(RuntimeShader::instance().GetShader(ShaderGroup::SHADOW_MAP));

		RuntimeShader::instance().SetUniform(RuntimeShader::instance().GetShader(ShaderGroup::SHADOW_MAP),
			"lightSpaceMatrix", UniformType::mat4, glm::value_ptr(lightSpaceMatrix));

		glViewport(0, 0, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, lightMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);

		for (int i = 0; i < mesh.size(); i++)
		{
			if (mesh[i].base->cast_shadow)
			{
				mesh[i].DrawDepth(RuntimeShader::instance().GetShader(ShaderGroup::SHADOW_MAP), glm::value_ptr(lightSpaceMatrix), simParams);
			}
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Reset viewport
		glViewport(0, 0, curr_wWidth, curr_wHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
}

void LightMap::ApplyLightMap(GLuint shader, GLenum unit, float funit)
{
	if (type == LightMap_Type::LT_SHADOW_MAP)
	{
		glActiveTexture(unit);

		RuntimeShader::instance().SetUniform(shader,
			"lightSpaceMatrix", UniformType::mat4, glm::value_ptr(lightSpaceMatrix));

		glBindTexture(GL_TEXTURE_2D, texture_id);
		RuntimeShader::instance().SetUniform(shader, "shadowMap", UniformType::int1, &funit);
	}
}

void LightMap::Clear()
{
	texture_id = -1;
	delete shadowBox;
}
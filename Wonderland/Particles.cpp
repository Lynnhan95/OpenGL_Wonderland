#include "Particles.h"

ParticleSystem::ParticleSystem(ParticleSystemProperties properties, Prefab* prefab, glm::vec3 pos, glm::vec3 rot)
{
	this->properties = properties;
	this->prefab = prefab;

	m_prev_time = -1;
	m_passed_time = 0;
	m_since_last_emit = 0;

	m_is_playing = properties.play_on_awake;

	m_deprecated = false;

	position = pos;
	rotation = rot;

	m_emit_dir = glm::rotate(rotation.x * D2R, glm::vec3(1, 0, 0))*glm::rotate(rotation.y * D2R, glm::vec3(0, 1, 0))*glm::rotate(rotation.z * D2R, glm::vec3(0, 0, 1))*glm::vec4(UP, 0);
	m_emit_dir = glm::normalize(m_emit_dir);

	// Initialize particles
	for (int i = 0; i < properties.max_count; i++)
	{
		ParticleInstance particle = *new ParticleInstance();

		particle.isActive = false;
		particle.base = prefab;
		particle.lifetime = properties.particle_lifetime;

		m_particles.push_back(particle);
	}

	m_active_count = 0;

	// Prepare for instanced drawing

	shader_program = prefab->materials[0].shader_program; // Take the first shader

	if (!prefab->has_particle_vbo) // Prevent duplicate binding
	{
		glBindVertexArray(prefab->mesh_data.mVao);

		// Transparency
		int transp_loc = glGetAttribLocation(shader_program, "transp_attrib");
		glGenBuffers(1, &m_transpVBO);
		glBindBuffer(GL_ARRAY_BUFFER, m_transpVBO);
		glEnableVertexAttribArray(transp_loc);
		glVertexAttribPointer(transp_loc, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
		glVertexAttribDivisor(transp_loc, 1); // Tell OpenGL this is an instanced vertex attribute

		// P-V-M matrix
		int mvp_loc = glGetAttribLocation(shader_program, "PVM_attrib");
		glGenBuffers(1, &m_mvpVBO);
		glBindBuffer(GL_ARRAY_BUFFER, m_mvpVBO);
		for (int i = 0; i < 4; i++) // A matrix takes up 4 indices
		{
			glEnableVertexAttribArray(mvp_loc + i);
			glVertexAttribPointer(mvp_loc + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(i * sizeof(glm::vec4)));
			glVertexAttribDivisor(mvp_loc + i, 1);
		}

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		prefab->ex_vbo_1 = m_mvpVBO;
		prefab->ex_vbo_2 = m_transpVBO;
		prefab->has_particle_vbo = true;
	}
	else
	{
		m_mvpVBO = prefab->ex_vbo_1;
		m_transpVBO = prefab->ex_vbo_2;
	}
}

// TODO: Complete the update in Compute Shader
void ParticleSystem::Update(Camera* const camera, SimParams* simParams)
{
	if (m_deprecated)
	{
		return;
	}

	if (m_prev_time == -1)
	{
		m_prev_time = glfwGetTime();
	}

	if (m_is_playing)
	{
		float deltaTime = glfwGetTime() - m_prev_time;
		m_passed_time += deltaTime;
		m_since_last_emit += deltaTime;

		// Update particles
		// TODO: Make this process parallel
//#pragma omp parallel num_threads(4)
		for (int i = 0; i < m_particles.size(); i++)
		{
			if (m_particles[i].isActive)
			{
				m_particles[i].passed_time += deltaTime;

				// Timeout
				bool go_on = true;
				if (m_particles[i].passed_time >= m_particles[i].lifetime)
				{
					m_particles[i].isActive = false;
					m_active_count--;
					go_on = false;
					break;
				}

				if (!go_on)
				{
					break;
				}

				// Update spatial info
				m_particles[i].pos += m_particles[i].translate_speed * deltaTime;
				m_particles[i].rot += m_particles[i].rotate_speed * deltaTime;
				m_particles[i].alpha = properties.initial_alpha - (properties.initial_alpha - properties.alpha_over_lifetime)*(m_particles[i].passed_time / m_particles[i].lifetime);
				m_particles[i].scale = m_particles[i].init_scale * (1 - (1 - properties.scale_over_lifetime)*(m_particles[i].passed_time / m_particles[i].lifetime));
				// Update speed
				m_particles[i].translate_speed = m_particles[i].init_speed * (1 - (1 - properties.speed_over_lifetime)*(m_particles[i].passed_time / m_particles[i].lifetime));
				m_particles[i].rotate_speed *= (1 - (1 - properties.rot_speed_over_lifetime)*(m_particles[i].passed_time / m_particles[i].lifetime));
				
				// Apply simulation
				if (simParams != nullptr)
				{
					if (properties.affected_by_wind)
					{
						m_particles[i].pos += glm::normalize(simParams->wind_direction) * simParams->wind_strength * deltaTime * properties.wind_coef;
					}
					if (properties.use_gravity)
					{
						m_particles[i].pos += GRAVITY * deltaTime;
					}
				}
				
				// For distance sorting
				if (properties.isTransparent)
				{
					m_particles[i].camera_distance = glm::length(m_particles[i].pos - camera->position());
				}
			}
		}

		//std::cout << m_active_count << std::endl;

		// Fill particles
		if (m_active_count <= properties.max_count && m_since_last_emit >= 1.0f / properties.emission_rate
			&& (m_passed_time < properties.lifetime || properties.repeat)) // Stop filling if particle system timed out or maxed out
		{
			int num_to_activate = floor(m_since_last_emit*properties.emission_rate);
			int activated = 0;

//#pragma omp parallel num_threads(4)
			for (int i = 0; i < m_particles.size(); i++)
			{
				// Reset particle
				if (!m_particles[i].isActive)
				{
					//m_particles[i].passed_time = 0;
					m_particles[i].rot = (((rand() % 10) / 10.0f)*(properties.initial_rotation.second - properties.initial_rotation.first) + properties.initial_rotation.first);
					m_particles[i].rotate_speed = (((rand() % 10) / 10.0f)*(properties.initial_rotation_speed.second - properties.initial_rotation_speed.first) + properties.initial_rotation_speed.first);
					m_particles[i].alpha = properties.initial_alpha;
					m_particles[i].scale = glm::vec3(((rand() % 10) / 10.0f)*(properties.initial_scale.second - properties.initial_scale.first) + properties.initial_scale.first);

					switch (properties.shape)
					{
					case CONE:
						m_particles[i].pos = position;
						glm::vec3 speed_dir =
							glm::rotate((2 * ((rand() % 10) / 10.0f) - 1)*properties.cone_angle * D2R, glm::vec3(1, 0, 0))
							*glm::rotate((2 * ((rand() % 10) / 10.0f) - 1)*properties.cone_angle * D2R, glm::vec3(0, 1, 0))
							*glm::rotate((2 * ((rand() % 10) / 10.0f) - 1)*properties.cone_angle * D2R, glm::vec3(0, 0, 1))
							*glm::vec4(m_emit_dir, 0);
						speed_dir = glm::normalize(speed_dir);
						m_particles[i].translate_speed = speed_dir * (((rand() % 10) / 10.0f)*(properties.initial_speed.second - properties.initial_speed.first) + properties.initial_speed.first);
						break;

					case SPHERE:
						m_particles[i].pos = position;
						m_particles[i].translate_speed = glm::vec3((2 * ((rand() % 10) / 10.0f) - 1), (2 * ((rand() % 10) / 10.0f) - 1), (2 * ((rand() % 10) / 10.0f) - 1))
							* (((rand() % 10) / 10.0f)*(properties.initial_speed.second - properties.initial_speed.first) + properties.initial_speed.first);
						break;

					case PLANE:
						m_diag_1 = position + glm::vec3(properties.plane_width / 2, 0, properties.plane_height / 2);
						m_diag_2 = position - glm::vec3(properties.plane_width / 2, 0, properties.plane_height / 2);
						glm::mat4 rtm = glm::rotate(rotation.x * D2R, glm::vec3(1, 0, 0))*glm::rotate(rotation.y * D2R, glm::vec3(0, 1, 0))*glm::rotate(rotation.z * D2R, glm::vec3(0, 0, 1));
						m_diag_1 = rtm * glm::vec4(m_diag_1, 0);
						m_diag_2 = rtm * glm::vec4(m_diag_2, 0);
						m_particles[i].pos = glm::vec3(
							((rand() % 10) / 10.0f)*(m_diag_2.x - m_diag_1.x) + m_diag_1.x,
							((rand() % 10) / 10.0f)*(m_diag_2.y - m_diag_1.y) + m_diag_1.y,
							((rand() % 10) / 10.0f)*(m_diag_2.z - m_diag_1.z) + m_diag_1.z
						);
						m_particles[i].translate_speed = m_emit_dir * (((rand() % 10) / 10.0f)*(properties.initial_speed.second - properties.initial_speed.first) + properties.initial_speed.first);
						break;
					}

					m_particles[i].passed_time = 0;

					m_particles[i].init_speed = m_particles[i].translate_speed;
					m_particles[i].init_scale = m_particles[i].scale;
					m_particles[i].init_rot_speed = m_particles[i].rotate_speed;

					m_particles[i].isActive = true;

					m_active_count++;
					activated++;
				}

				if (activated >= num_to_activate)
				{
					break;
				}
			}
			m_since_last_emit = 0;
		}	
	}

	m_prev_time = glfwGetTime();

	if (!properties.repeat && m_passed_time >= properties.lifetime && m_active_count <= 0) // Wait for all particles to time out
	{
		Deprecate();
	}
}

void ParticleSystem::Play()
{
	m_is_playing = true;
}

void ParticleSystem::Stop()
{
	m_is_playing = false;
}

void ParticleSystem::Deprecate()
{
	m_deprecated = true;
}

// Particle distance compare function
bool Further(const ParticleInstance& p1, const ParticleInstance& p2)
{
	return p1.camera_distance > p2.camera_distance;
}

void ParticleSystem::Draw(Camera* const camera)
{
	if (m_deprecated || !m_is_playing || m_active_count < 1)
	{
		return;
	}

	// Draw transparent particles
	if (!properties.isTransparent)
	{
		glDisable(GL_BLEND);
	}
	else
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		std::sort(m_particles.begin(), m_particles.end(), Further); // Sort particles by distance
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glUseProgram(shader_program);
	glBindVertexArray(m_particles[0].base->mesh_data.mVao);

	// Albedo
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_particles[0].base->materials[0].albedo_id); // Take the first
	float unit = 0;
	RuntimeShader::instance().SetUniform(shader_program, "albedo", UniformType::flt1, &unit); // we bound our texture to texture unit 0

	// Normal
	//glActiveTexture(GL_TEXTURE1);
	//glBindTexture(GL_TEXTURE_2D, m_particles[0].base->materials[0].normal_id);

	// Specular
	//glActiveTexture(GL_TEXTURE2);
	//glBindTexture(GL_TEXTURE_2D, m_particles[0].base->materials[0].spec_id);

	// Color tint
	RuntimeShader::instance().SetUniform(shader_program, "color", UniformType::vec3, glm::value_ptr(properties.color));

	// Time
	//float glfw_time = glfwGetTime();
	//RuntimeShader::instance().SetUniform(shader_program, "time", UniformType::flt1, &glfw_time);

	// Instanced drawing
	std::vector<glm::mat4> mvp_matrices;
	std::vector<float> transparencies;

//#pragma omp parallel num_threads(4)
	for (int i = 0; i < m_particles.size(); ++i)
	{	
		if (m_particles[i].isActive)
		{
			// Model matrix
			glm::mat4 T = glm::translate(m_particles[i].pos);
			glm::mat4 S = glm::scale(m_particles[i].scale*glm::vec3(m_particles[i].base->mesh_data.mScaleFactor));
			glm::mat4 R = glm::rotate(m_particles[i].rot.x * D2R, glm::vec3(1, 0, 0))*glm::rotate(m_particles[i].rot.y * D2R, glm::vec3(0, 1, 0))*glm::rotate(m_particles[i].rot.z * D2R, glm::vec3(0, 0, 1));
			// Calculate billboard matrix
			if (properties.billboard)
			{
				// This method is only applicable to quads
				glm::mat4 billboard_mat = glm::rotate(-camera->rotation().y - 90 * D2R, glm::vec3(0, 1, 0));
				R = billboard_mat * R;
			}
			glm::mat4 M = T * R * S;

			// Compute instance matrix
			glm::mat4 PVM = camera->PV_matrix * M;

			// Push into arrays
			mvp_matrices.push_back(PVM);
			transparencies.push_back(m_particles[i].alpha);
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, m_mvpVBO); // This attribute comes from PVM matrices buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * m_active_count, mvp_matrices.data(), GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, m_transpVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * m_active_count, &transparencies[0], GL_DYNAMIC_DRAW);

	m_particles[0].base->mesh_data.DrawInstanced(m_active_count); // Take the first mesh base

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);

	// Release space
	mvp_matrices.clear();
	transparencies.clear();
}

void ParticleSystem::DrawStroke(Camera* const camera)
{
	glBindVertexArray(m_particles[0].base->mesh_data.mVao); // This is used to activate shader only

	//glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glPolygonMode(GL_FRONT_AND_BACK, GL_POINTS);

	GLuint stroke_shader = RuntimeShader::instance().GetShader(ShaderGroup::STROKE);

	//for (int i = 0; i < base->submeshCount(); i++)
	//{
		glUseProgram(stroke_shader);

		// Model matrix
		glm::mat4 T = glm::translate(position);
		glm::mat4 R = glm::rotate(rotation.x * D2R, glm::vec3(1, 0, 0))*glm::rotate(rotation.y * D2R, glm::vec3(0, 1, 0))*glm::rotate(rotation.z * D2R, glm::vec3(0, 0, 1));
		glm::mat4 M = T * R;

		// Set space transform uniform variable
		glm::mat4 PVM = camera->PV_matrix * M;
		RuntimeShader::instance().SetUniform(stroke_shader,
			"PVM", UniformType::mat4, glm::value_ptr(PVM));

		glm::vec3 bBmax = glm::vec3(0);
		glm::vec3 bBmin = glm::vec3(0);
		if (properties.shape == ParticleSystemShape::CONE)
		{
			float boundSize = properties.initial_scale.second / 2;
			bBmax = glm::vec3(boundSize);
			bBmin = glm::vec3(-boundSize);
		}
		else if (properties.shape = ParticleSystemShape::PLANE)
		{
			float boundThickness = properties.initial_scale.second / 2;
			bBmax.x = properties.plane_width / 2;
			bBmax.y = boundThickness;
			bBmax.z = properties.plane_height / 2;
			bBmin.x = -properties.plane_width / 2;
			bBmin.y = -boundThickness;
			bBmin.z = -properties.plane_height / 2;
		}
		RuntimeShader::instance().SetUniform(stroke_shader,
			"BbMax", UniformType::vec3, glm::value_ptr(bBmax));	
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

void ParticleSystem::Translate(glm::vec3 distance)
{
	if (m_deprecated)
	{
		return;
	}

	position += distance;
	m_diag_1 += distance;
	m_diag_2 += distance;
}

void ParticleSystem::Rotate(glm::vec3 angles)
{
	if (m_deprecated)
	{
		return;
	}

	glm::mat4 rtm = glm::rotate(rotation.x * D2R, glm::vec3(1, 0, 0))*glm::rotate(rotation.y * D2R, glm::vec3(0, 1, 0))*glm::rotate(rotation.z * D2R, glm::vec3(0, 0, 1));
	rotation += angles;
	m_emit_dir = rtm * glm::vec4(m_emit_dir, 0);
}

bool ParticleSystem::isDeprecated()
{
	return m_deprecated;
}

void ParticleSystem::Release()
{
	m_particles.clear();
	glDeleteBuffers(1, &m_mvpVBO);
	glDeleteBuffers(1, &m_transpVBO);
}

std::string ParticleSystem::GetIObjectName()
{
	return name;
}

std::vector<Handle> ParticleSystem::GetIHandle()
{
	if (m_i_handles.size() == 0)
	{
		Handle* h = new Handle();
		h->type = HandleType::VEC3;
		h->opType = OperationType::OT_INPUT;
		h->name = "Emitter Position";
		h->v1 = glm::value_ptr(position);
		m_i_handles.push_back(*h);

		h = new Handle();
		h->type = HandleType::COL3;
		h->name = "Color";
		h->v1 = glm::value_ptr(properties.color);
		m_i_handles.push_back(*h);

		h = new Handle();
		h->type = HandleType::HT_BOOL;
		h->name = "Play";
		h->b1 = &m_is_playing;
		m_i_handles.push_back(*h);

		// TODO: Expose more handles
	}

	return m_i_handles;
}

void ParticleSystem::ClearHandles()
{
	m_i_handles.clear();
}

// Hard coded profile initializer
void Init_ParticleSystem_Properties(std::map<std::string, ParticleSystemProperties>& gPartiSystem_profiles)
{
	gPartiSystem_profiles.clear();

	// White fog
	ParticleSystemProperties profile = *new ParticleSystemProperties();
	profile.play_on_awake = true;
	profile.billboard = true;
	profile.shape = ParticleSystemShape::CONE;
	profile.initial_scale = std::pair<float, float>(0.05f, 0.25f);
	profile.initial_speed = std::pair<float, float>(0.25f, 0.75f);
	profile.initial_rotation = std::pair<glm::vec3, glm::vec3>(glm::vec3(90, -180, 0), glm::vec3(90, 180, 0));
	profile.initial_rotation_speed = std::pair<glm::vec3, glm::vec3>(glm::vec3(0, -90, 0), glm::vec3(0, 90, 0));
	profile.isTransparent = true;
	profile.initial_alpha = 1;
	profile.scale_over_lifetime = 3.0f;
	profile.speed_over_lifetime = 0.6f;
	profile.alpha_over_lifetime = 0;
	profile.rot_speed_over_lifetime = 1;
	profile.lifetime = 0;
	profile.particle_lifetime = 2.5f;
	profile.emission_rate = 10;
	profile.max_count = 50;
	profile.repeat = true;
	profile.cone_angle = 30;
	profile.affected_by_wind = true;
	profile.wind_coef = 0.4f;
	gPartiSystem_profiles["Smoke1"] = profile;

	// Snow field
	profile = *new ParticleSystemProperties();
	profile.play_on_awake = true;
	profile.billboard = true;
	profile.shape = ParticleSystemShape::PLANE;
	profile.initial_scale = std::pair<float, float>(0.02f, 0.1f);
	profile.initial_speed = std::pair<float, float>(-0.4f, -1.0f);
	profile.initial_rotation = std::pair<glm::vec3, glm::vec3>(glm::vec3(90, -180, 0), glm::vec3(90, 180, 0));
	profile.initial_rotation_speed = std::pair<glm::vec3, glm::vec3>(glm::vec3(0, -45, 0), glm::vec3(0, 45, 0));
	profile.isTransparent = true;
	profile.initial_alpha = 1;
	profile.scale_over_lifetime = 1;
	profile.speed_over_lifetime = 1.1f;
	profile.alpha_over_lifetime = 0.2f;
	profile.rot_speed_over_lifetime = 1;
	profile.lifetime = 0;
	profile.particle_lifetime = 16.0f;
	profile.emission_rate = 30;
	profile.max_count = 270;
	profile.repeat = true;
	profile.plane_height = 20;
	profile.plane_width = 15;
	profile.affected_by_wind = true;
	profile.wind_coef = 0.2f;
	gPartiSystem_profiles["SnowField1"] = profile;

	// Fire
	profile = *new ParticleSystemProperties();
	profile.play_on_awake = true;
	profile.billboard = true;
	profile.shape = ParticleSystemShape::CONE;
	profile.initial_scale = std::pair<float, float>(0.35f, 0.4f);
	profile.initial_speed = std::pair<float, float>(0.1f, 0.15f);
	profile.initial_rotation = std::pair<glm::vec3, glm::vec3>(glm::vec3(90, -10, 0), glm::vec3(90, 10, 0));
	profile.initial_rotation_speed = std::pair<glm::vec3, glm::vec3>(glm::vec3(0, -20, 0), glm::vec3(0, 20, 0));
	profile.isTransparent = true;
	profile.initial_alpha = 1;
	profile.scale_over_lifetime = 0.1f;
	profile.speed_over_lifetime = 3.0f;
	profile.alpha_over_lifetime = 0;
	profile.rot_speed_over_lifetime = 1.5;
	profile.lifetime = 0;
	profile.particle_lifetime = 1.1f;
	profile.emission_rate = 20;
	profile.max_count = 30;
	profile.repeat = true;
	profile.cone_angle = 5;
	profile.affected_by_wind = true;
	profile.wind_coef = 0.05f;
	gPartiSystem_profiles["Fire1"] = profile;
}
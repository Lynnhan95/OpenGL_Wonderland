#version 430

uniform sampler2D albedo;
uniform sampler2D shadowMap;
uniform vec3 Ambt_Color;
uniform vec3 Diff_Color;
uniform vec3 Spec_Color;
uniform float shiness;
uniform float transparency;
uniform int use_texture_color;
//uniform float time;
uniform vec3 viewPos;

// Point light
// Up to 10 point light allowed
struct PointLight
{
	vec3  position;
	vec3  color;
	float attenuation;
	float radius;
	float intensity_ambt;
	float intensity_diff;
	float intensity_spec;
	int   soft_edge;
	float soft_ratio;
};
#define NUM_POINT_LIGHTS 10 // Maximum number of point lights
uniform PointLight pointLights[NUM_POINT_LIGHTS];
uniform int active_pointlight_num;

// Directional light
// Only one directional light allowed
uniform vec3  dir_light_color_0;
uniform vec3  dir_light_dir_0;
uniform float dir_light_iambt_0;
uniform float dir_light_idiff_0;
uniform float dir_light_ispec_0;

in vec3 frag_pos;
in vec2 tex_coord;
in vec3 normal_w;
in vec4 fragPos_lightSpace;

out vec4 fragcolor;

//vec2 poissonDisk[4] = vec2[](
//  vec2(-0.94201624, -0.39906216),
//  vec2(0.94558609, -0.76890725),
//  vec2(-0.094184101, -0.92938870),
//  vec2(0.34495938, 0.29387760)
//);


// Shadow calculation from https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
// --------------------------------------------------------
float shadow_calculation(vec4 fragPosLightSpace)
{
	// Temporary fix, should find the root cause
	//float distance = length(frag_pos - viewPos);
	//if (distance > 10)
		//return 0.0;

	// Perform perspective divide
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

	// Transform to [0,1] range
	projCoords = projCoords * 0.5f + 0.5f;

	// Resolve oversampling
	if (projCoords.z > 1.0f)
	{
		return 0.0f;
	}

	// Get closest depth value from light's perspective (using [0,1] range fragPosLightSpace as coords)
	float closestDepth = texture(shadowMap, projCoords.xy).r;
	// Get depth of current fragment from light's perspective
	float currentDepth = projCoords.z;

	// Remove shadow acne
	float bias = max(0.005f * (1.0f - dot(normal_w, dir_light_dir_0)), 0.0005f);

	float shadow = 0.0f;

	// Percentage-closer filtering
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	for (int x = -1; x <= 1; ++x) // 3x3 sampling
	{
		for (int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
			// Check whether current frag pos is in shadow
			shadow += currentDepth - bias > pcfDepth ? 1.0f : 0.0f;
		}
	}
	shadow /= 9.0f;

	//float shadow_poi = 0;
	// Poisson Sampling; from http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-16-shadow-mapping/
	//for (int i = 0; i < 4; i++)
	//{
		//if (texture(shadowMap, projCoords.xy + poissonDisk[i] / 700.0f ).r < currentDepth - bias)
		//{
			//shadow_poi += 0.25f;
		//}
	//}
	//shadow *= shadow_poi;

	// Original
	//shadow = currentDepth - bias > closestDepth ? 1.0f : 0.0f;

	return shadow;
}

vec3 calc_point_light(vec3 fragColor, PointLight light, vec3 viewDir)
{
	float distance = length(frag_pos - light.position);

	// Blinn-Phong
	float I;
	float Att_diff, Att_spec;
	float cutOff = (light.soft_edge == 1) ? light.radius*(1 + light.soft_ratio) : light.radius;
	if (distance <= cutOff)
	{
		vec3 light_dir = normalize(light.position - frag_pos);
		float atten = 1.0f / ((1.0f / light.attenuation)*(1.0f + 0.35f*distance + 0.44f*distance*distance)); // Constant, Linear, Quadratic
		vec3 hf = normalize(light_dir + viewDir);
		Att_diff = atten * max(0, dot(normal_w, light_dir));
		Att_spec = atten * pow(max(0, dot(hf, normal_w)), shiness);
		//Att_spec = atten * pow(max(0, dot(reflect(-light_dir, normal_w), viewDir)), shiness); // Phong
		if (use_texture_color == 0)
		{
			I = dot(light.intensity_ambt*light.color, Ambt_Color) 
				+ Att_diff * dot(light.intensity_diff*light.color, Diff_Color) 
				+ Att_spec * dot(light.intensity_spec*light.color, Spec_Color);
		}
		else
		{
			I = dot(light.intensity_ambt*light.color, fragColor) 
				+ Att_diff * dot(light.intensity_diff*light.color, fragColor) 
				+ Att_spec * dot(light.intensity_spec*light.color, Spec_Color);
		}

		// Smooth light edge
		if (light.soft_edge == 1 && distance > light.radius)
		{
			float delta = cutOff - distance;
			float epsilon = light.soft_ratio * light.radius;
			float intensity = clamp(delta / epsilon, 0.0f, 1.0f);
			I *= intensity;
		}
	}
	else
	{
		return vec3(0, 0, 0);
	}

	fragColor *= I;

	// Mix light color with texture color
	fragColor *= light.color;

	return fragColor;
}

vec3 calc_directional_light(vec3 fragColor, vec3 viewDir)
{
	// Blinn-Phong
	float I;
	float Att_diff, Att_spec;
	vec3 hf = normalize(dir_light_dir_0 + viewDir);
	Att_diff = max(0, dot(normal_w, dir_light_dir_0));
	Att_spec = pow(max(0, dot(hf, normal_w)), shiness);

	// Apply shadow
	float shadow = shadow_calculation(fragPos_lightSpace);
	Att_diff *= (1.0f - shadow);
	Att_spec *= (1.0f - shadow);

	//Att_spec = pow(max(0, dot(reflect(-dir_light_dir_0, normal_w), viewDir)), shiness); // Phong
	if (use_texture_color == 0)
	{
		I = dot(dir_light_iambt_0*dir_light_color_0, Ambt_Color) 
			+ Att_diff * dot(dir_light_idiff_0*dir_light_color_0, Diff_Color) 
			+ Att_spec * dot(dir_light_ispec_0*dir_light_color_0, Spec_Color);
	}
	else
	{
		I = dot(dir_light_iambt_0*dir_light_color_0, fragColor) 
			+ Att_diff * dot(dir_light_idiff_0*dir_light_color_0, fragColor) 
			+ Att_spec * dot(dir_light_ispec_0*dir_light_color_0, Spec_Color);
	}
	fragColor *= I;

	fragColor *= dir_light_color_0;

	return fragColor;
}

void main(void)
{   	
	fragcolor = texture2D(albedo, tex_coord);

	if (fragcolor.a <= 0)
	{
		discard;
	}

	// Point lights
	vec3 viewDir = normalize(viewPos - frag_pos);
	vec3 light_cumu = vec3(0, 0, 0);
	for (int i = 0; i < active_pointlight_num; i++)
	{
		light_cumu += calc_point_light(fragcolor.xyz, pointLights[i], viewDir);
	}

	// Directional light
	light_cumu += calc_directional_light(fragcolor.xyz, viewDir);

	fragcolor.xyz = light_cumu;

	// Mix albedo color
	fragcolor.xyz *= Diff_Color;

	// Apply transparency
	fragcolor.a *= transparency;

	//vec3 projCoords = fragPos_lightSpace.xyz / fragPos_lightSpace.w;
	//projCoords = projCoords * 0.5f + 0.5f;
	//fragcolor.xy = projCoords.xy;
	//fragcolor.z = 0;

	//fragcolor.xyz = vec3(fragPos_lightSpace.z, fragPos_lightSpace.z, fragPos_lightSpace.z);

	// Validating normal
	//fragcolor.rgb = normal_w.xyz;
}
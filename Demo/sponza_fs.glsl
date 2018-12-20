#version 330

//Create structure for light sources and pass in uniform array of instances
struct Light
{
	vec3 position;
	float range;
	vec3 colour;
	float cone_angle;
	vec3 cone_direction;
};
const int kNoOfLights = 22;
uniform Light light_sources[kNoOfLights];

//Create structure for materials and pass in uniform instance
struct Material
{
	vec3 ambient_colour;
	vec3 diffuse_colour;
	bool has_diffuse_texture;
	sampler2D diff_texture;
	vec3 specular_colour;
	bool has_specular_texture;
	sampler2D spec_texture;
	float shininess;
	bool is_shiny;
};
uniform Material mat;

uniform vec3 scene_ambient_light;
uniform vec3 camera_position;

//Add in variables for each of the streamed attributes
in vec3 varying_position;
in vec3 varying_normal;
in vec2 varying_texture_coordinates;

out vec4 fragment_colour;

//Function to calculate diffuse values for the lights in Lambert reflection
vec3 DiffuseLightSource(Light light_)
{
	vec3 diffuse_colour;

	//Check if material has diffuse texture
	if (mat.has_diffuse_texture)
	{
		vec3 tex_colour = texture(mat.diff_texture, varying_texture_coordinates).rgb;
		diffuse_colour = mat.diffuse_colour * tex_colour;
	}
	else
		diffuse_colour = mat.diffuse_colour;

	vec3 sum = vec3(0.f, 0.f, 0.f);

	vec3 light_vector = normalize(light_.position - varying_position);
	vec3 normalised_varying_normal = normalize(varying_normal);

	float diffuse_intensity = max(0.f, dot(light_vector, normalised_varying_normal));
	float dist = distance(varying_position, light_.position);
	float attenuation = smoothstep(light_.range, light_.range / 2, dist);

	sum = light_.colour * diffuse_colour * diffuse_intensity * attenuation;

	return sum;
}

//Function to calculate specular values for the lights in Phong reflection
vec3 SpecularLightSource(Light light_)
{
	vec3 specular_colour;

	//Check if material has specular texture
	if (mat.has_specular_texture)
	{
		vec3 tex_colour = texture(mat.spec_texture, varying_texture_coordinates).rgb;
		specular_colour = mat.specular_colour * tex_colour;
	}
	else
		specular_colour = mat.specular_colour;

	vec3 sum = vec3(0.f, 0.f, 0.f);

	vec3 light_vector = normalize(light_.position - varying_position);
	vec3 normalised_varying_normal = normalize(varying_normal);
	vec3 camera_direction = normalize(camera_position - varying_position);
	vec3 reflected_light = reflect(-light_vector, normalised_varying_normal);

	float specular_intensity = max(0.f, dot(camera_direction, reflected_light));
	specular_intensity = pow(specular_intensity, mat.shininess / 5);

	float dist = distance(varying_position, light_.position);
	float attenuation = smoothstep(light_.range, light_.range / 2, dist);

	sum = light_.colour * specular_colour * specular_intensity * attenuation;
	
	return sum;
}

//Diffuse lighting for a spotlight
vec3 SpotlightLightSource(Light spotlight_)
{
	vec3 light_vector = normalize(spotlight_.position - varying_position);
	float light_to_surface_angle = smoothstep(cos(0.5f * radians(spotlight_.cone_angle)), 1, dot(-light_vector, normalize(spotlight_.cone_direction)));
	float diffuse_intensity = max(0.f, dot(light_vector, normalize(varying_normal)));
	float dist = distance(spotlight_.position, varying_position);
	float attenuation = smoothstep(spotlight_.range, spotlight_.range / 2, dist);

	return diffuse_intensity * mat.diffuse_colour * attenuation * light_to_surface_angle * spotlight_.colour;
}

void main(void)
{
	vec3 intensity_to_eye = vec3(0.f, 0.f, 0.f);

	//Create instance of a spotlight and assign values to the variables
	Light spotlight;
	spotlight.position = vec3(-35.f, 58.f, -3.f);
	spotlight.range = 80.f;
	spotlight.colour = vec3(0.5f, 0.66f, 0.85f);
	spotlight.cone_angle = 25.f;
	spotlight.cone_direction = vec3(0.f, -1.f, 0.f);
	
	intensity_to_eye += SpotlightLightSource(spotlight);

	//Apply Lambert reflection to all lights and Phong where material is shiny
	for (int i = 0; i < kNoOfLights; i++)
	{
		intensity_to_eye += DiffuseLightSource(light_sources[i]);

		if (mat.is_shiny)
			intensity_to_eye += SpecularLightSource(light_sources[i]);
	}
	intensity_to_eye += (mat.ambient_colour * scene_ambient_light);

	fragment_colour = vec4(intensity_to_eye, 1.0f);
}
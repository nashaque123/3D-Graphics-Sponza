#include "MyView.hpp"
#include <sponza/sponza.hpp>
#include <tygra/FileHelper.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <cassert>

MyView::MyView()
{
}

MyView::~MyView() {
}

void MyView::setScene(const sponza::Context * scene)
{
    scene_ = scene;
}

void MyView::windowViewWillStart(tygra::Window * window)
{
    assert(scene_ != nullptr);

	start_time_ = std::chrono::system_clock::now();

	GLint compile_status = GL_FALSE;

	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	std::string vertex_shader_string
		= tygra::createStringFromFile("resource:///sponza_vs.glsl");
	const char * vertex_shader_code = vertex_shader_string.c_str();
	glShaderSource(vertex_shader, 1,
		(const GLchar **)&vertex_shader_code, NULL);
	glCompileShader(vertex_shader);
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compile_status);
	if (compile_status != GL_TRUE)
	{
		const int string_length = 1024;
		GLchar log[string_length] = "";
		glGetShaderInfoLog(vertex_shader, string_length, NULL, log);
		std::cerr << log << std::endl;
	}

	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	std::string fragment_shader_string
		= tygra::createStringFromFile("resource:///sponza_fs.glsl");
	const char * fragment_shader_code = fragment_shader_string.c_str();
	glShaderSource(fragment_shader, 1,
		(const GLchar **)&fragment_shader_code, NULL);
	glCompileShader(fragment_shader);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compile_status);
	if (compile_status != GL_TRUE)
	{
		const int string_length = 1024;
		GLchar log[string_length] = "";
		glGetShaderInfoLog(fragment_shader, string_length, NULL, log);
		std::cerr << log << std::endl;
	}

	//Create shader program & shader in variables
	shader_program_ = glCreateProgram();
	glAttachShader(shader_program_, vertex_shader);

	//glBindAttribLocation for all shader streamed IN variables
	glBindAttribLocation(shader_program_, kVertexPosition, "vertex_position");
	glBindAttribLocation(shader_program_, kVertexNormal, "vertex_normal");
	glBindAttribLocation(shader_program_, kTextureCoordinates, "texture_coordinates");

	glDeleteShader(vertex_shader);
	glAttachShader(shader_program_, fragment_shader);
	glDeleteShader(fragment_shader);
	glLinkProgram(shader_program_);

	GLint link_status = GL_FALSE;
	glGetProgramiv(shader_program_, GL_LINK_STATUS, &link_status);
	if (link_status != GL_TRUE)
	{
		const int string_length = 1024;
		GLchar log[string_length] = "";
		glGetProgramInfoLog(shader_program_, string_length, NULL, log);
		std::cerr << log << std::endl;
	}

	/*
		The framework provides a builder class that allows access to all the mesh data	
	*/

	sponza::GeometryBuilder builder;
	const auto& source_meshes = builder.getAllMeshes();

	//Loop through each mesh in the scene
	for each (const sponza::Mesh& source in source_meshes)
	{
		//To access the actual mesh raw data we can get the array
		//Get the normals, elements and texture coordinates in a similar way
		const auto& positions = source.getPositionArray();
		const auto& normals = source.getNormalArray();
		const auto& elements = source.getElementArray();
		const auto& textureCoordinates = source.getTextureCoordinateArray();

		MeshGL myMesh;
		myMesh.id = source.getId();

		//Create VBOs for position, normals, elements and texture coordinates
		glGenBuffers(1, &myMesh.positionVBO);
		glBindBuffer(GL_ARRAY_BUFFER, myMesh.positionVBO);
		glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, kNullId);

		glGenBuffers(1, &myMesh.normalVBO);
		glBindBuffer(GL_ARRAY_BUFFER, myMesh.normalVBO);
		glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, kNullId);

		glGenBuffers(1, &myMesh.elementVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, myMesh.elementVBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements.size() * sizeof(unsigned int), elements.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, kNullId);
		myMesh.numElements = (int)elements.size();

		glGenBuffers(1, &myMesh.textureCoordinatesVBO);
		glBindBuffer(GL_ARRAY_BUFFER, myMesh.textureCoordinatesVBO);
		glBufferData(GL_ARRAY_BUFFER, textureCoordinates.size() * sizeof(glm::vec2), textureCoordinates.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, kNullId);

		//Create a VAO to wrap all the VBOs
		glGenVertexArrays(1, &myMesh.vao);
		glBindVertexArray(myMesh.vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, myMesh.elementVBO);
		glBindBuffer(GL_ARRAY_BUFFER, myMesh.positionVBO);
		glEnableVertexAttribArray(kVertexPosition);
		glVertexAttribPointer(kVertexPosition, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), TGL_BUFFER_OFFSET(0));
		glBindBuffer(GL_ARRAY_BUFFER, myMesh.normalVBO);
		glEnableVertexAttribArray(kVertexNormal);
		glVertexAttribPointer(kVertexNormal, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), TGL_BUFFER_OFFSET(0));
		glBindBuffer(GL_ARRAY_BUFFER, myMesh.textureCoordinatesVBO);
		glEnableVertexAttribArray(kTextureCoordinates);
		glVertexAttribPointer(kTextureCoordinates, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), TGL_BUFFER_OFFSET(0));
		glBindBuffer(GL_ARRAY_BUFFER, kNullId);
		glBindVertexArray(kNullId);

		//Store in a mesh structure and add to a container for later use
		m_meshVector.push_back(myMesh);

		//Load in all the textures
		tygra::Image diff0 = tygra::createImageFromPngFile("resource:///diff0.png");
		if (diff0.doesContainData()) {
			glGenTextures(1, &diff0_texture);
			glBindTexture(GL_TEXTURE_2D, diff0_texture);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
				GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			GLenum pixel_formats[] = { 0, GL_RED, GL_RG, GL_RGB, GL_RGBA };
			glTexImage2D(GL_TEXTURE_2D,
				0,
				GL_RGBA,
				diff0.width(),
				diff0.height(),
				0,
				pixel_formats[diff0.componentsPerPixel()],
				diff0.bytesPerComponent() == 1
				? GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT,
				diff0.pixelData());
			glGenerateMipmap(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, kNullId);
		}

		tygra::Image diff1 = tygra::createImageFromPngFile("resource:///diff1.png");
		if (diff1.doesContainData()) {
			glGenTextures(1, &diff1_texture);
			glBindTexture(GL_TEXTURE_2D, diff1_texture);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
				GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			GLenum pixel_formats[] = { 0, GL_RED, GL_RG, GL_RGB, GL_RGBA };
			glTexImage2D(GL_TEXTURE_2D,
				0,
				GL_RGBA,
				diff1.width(),
				diff1.height(),
				0,
				pixel_formats[diff1.componentsPerPixel()],
				diff1.bytesPerComponent() == 1
				? GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT,
				diff1.pixelData());
			glGenerateMipmap(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, kNullId);
		}

		tygra::Image spec1 = tygra::createImageFromPngFile("resource:///spec1.png");
		if (spec1.doesContainData()) {
			glGenTextures(1, &spec1_texture);
			glBindTexture(GL_TEXTURE_2D, spec1_texture);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
				GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			GLenum pixel_formats[] = { 0, GL_RED, GL_RG, GL_RGB, GL_RGBA };
			glTexImage2D(GL_TEXTURE_2D,
				0,
				GL_RGBA,
				spec1.width(),
				spec1.height(),
				0,
				pixel_formats[spec1.componentsPerPixel()],
				spec1.bytesPerComponent() == 1
				? GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT,
				spec1.pixelData());
			glGenerateMipmap(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, kNullId);
		}

		tygra::Image spec2 = tygra::createImageFromPngFile("resource:///spec2.png");
		if (spec2.doesContainData()) {
			glGenTextures(1, &spec2_texture);
			glBindTexture(GL_TEXTURE_2D, spec2_texture);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
				GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			GLenum pixel_formats[] = { 0, GL_RED, GL_RG, GL_RGB, GL_RGBA };
			glTexImage2D(GL_TEXTURE_2D,
				0,
				GL_RGBA,
				spec2.width(),
				spec2.height(),
				0,
				pixel_formats[spec2.componentsPerPixel()],
				spec2.bytesPerComponent() == 1
				? GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT,
				spec2.pixelData());
			glGenerateMipmap(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, kNullId);
		}
	}

}

void MyView::windowViewDidReset(tygra::Window * window,
                                int width,
                                int height)
{
    glViewport(0, 0, width, height);
}

void MyView::windowViewDidStop(tygra::Window * window)
{
	//Delete all the buffers when program is closed to prevent memory leaks
	glDeleteProgram(shader_program_);

	for (auto &p : m_meshVector)
	{
		glDeleteBuffers(1, &p.positionVBO);
		glDeleteBuffers(1, &p.normalVBO);
		glDeleteBuffers(1, &p.positionVBO);
		glDeleteBuffers(1, &p.elementVBO);
		glDeleteBuffers(1, &p.textureCoordinatesVBO);
		glDeleteVertexArrays(1, &p.vao);
	}
}

void MyView::windowViewRender(tygra::Window * window)
{
	assert(scene_ != nullptr);

	// Configure pipeline settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// Clear buffers from previous frame
	glClearColor(0.f, 0.f, 0.25f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shader_program_);
	 
	// Compute viewport
	GLint viewport_size[4];
	glGetIntegerv(GL_VIEWPORT, viewport_size);
	const float aspect_ratio = viewport_size[2] / (float)viewport_size[3];

	//Compute projection matrix
	const auto& cam = scene_->getCamera();
	glm::mat4 projection_xform = glm::perspective(glm::radians(cam.getVerticalFieldOfViewInDegrees()), aspect_ratio, cam.getNearPlaneDistance(), cam.getFarPlaneDistance());

	//Compute view matrix
	const auto& camera_pos = (const glm::vec3&)scene_->getCamera().getPosition();
	const auto& camera_at_pos = camera_pos + (glm::vec3&)scene_->getCamera().getDirection();
	const auto& world_up = (glm::vec3&)scene_->getUpDirection();

	//Compute camera view matrix and combine with projection matrix
	glm::mat4 view_xform = glm::lookAt(camera_pos, camera_at_pos, world_up);

	//Create combined view * projection matrix and pass to shader as a uniform
	glm::mat4 combined_matrix = projection_xform * view_xform;
	GLuint combined_matrix_id = glGetUniformLocation(shader_program_, "combined_matrix");
	glUniformMatrix4fv(combined_matrix_id, 1, GL_FALSE, glm::value_ptr(combined_matrix));

	//Get light data from scene then plug the values into the shader
	glm::vec3 scene_ambient_light = (glm::vec3&)scene_->getAmbientLightIntensity();
	GLuint scene_ambient_light_id = glGetUniformLocation(shader_program_, "scene_ambient_light");
	glUniform3fv(scene_ambient_light_id, 1, glm::value_ptr(scene_ambient_light));

	glm::vec3 camera_position = camera_pos;
	GLuint camera_position_id = glGetUniformLocation(shader_program_, "camera_position");
	glUniform3fv(camera_position_id, 1, glm::value_ptr(camera_position));

	const auto& light_sources = scene_->getAllLights();

	for (int l = 0; l < light_sources.size(); l++)
	{
		glm::vec3 position = (glm::vec3&)light_sources[l].getPosition();
		float range = light_sources[l].getRange();
		glm::vec3 colour = (glm::vec3&)light_sources[l].getIntensity();

		std::string pos_uniform_id = "light_sources[" + std::to_string(l) + "].position";
		GLuint position_id = glGetUniformLocation(shader_program_, pos_uniform_id.c_str());
		glUniform3fv(position_id, 1, glm::value_ptr(position));

		std::string range_uniform_id = "light_sources[" + std::to_string(l) + "].range";
		GLuint range_id = glGetUniformLocation(shader_program_, range_uniform_id.c_str());
		glUniform1f(range_id, range);

		std::string colour_uniform_id = "light_sources[" + std::to_string(l) + "].colour";
		GLuint colour_id = glGetUniformLocation(shader_program_, colour_uniform_id.c_str());
		glUniform3fv(colour_id, 1, glm::value_ptr(colour));
	}

	//Render each mesh by looping through mesh container
	for (const auto& mesh : m_meshVector)
	{
		glBindVertexArray(mesh.vao);

		//Render each instance of mesh with its own model matrix
		const auto& instances = scene_->getInstancesByMeshId(mesh.id);

		for (auto i : instances)
		{
			//For each instance, call getTransformationMatrix and pass to the shader as a uniform
			glm::mat4 world_matrix = (glm::mat4x3&)scene_->getInstanceById(i).getTransformationMatrix();
			GLuint world_matrix_id = glGetUniformLocation(shader_program_, "world_matrix");
			glUniformMatrix4fv(world_matrix_id, 1, GL_FALSE, glm::value_ptr(world_matrix));

			//Get material for this instance
			const auto& material_id = scene_->getInstanceById(i).getMaterialId();
			const auto& material = scene_->getMaterialById(material_id);
			
			//Get the material colours and pass to the shader
			const glm::vec3& ambient_colour = (glm::vec3&)material.getAmbientColour();
			GLuint ambient_colour_id = glGetUniformLocation(shader_program_, "mat.ambient_colour");
			glUniform3fv(ambient_colour_id, 1, glm::value_ptr(ambient_colour));

			const glm::vec3& diffuse_colour = (glm::vec3&)material.getDiffuseColour();
			GLuint diffuse_colour_id = glGetUniformLocation(shader_program_, "mat.diffuse_colour");
			glUniform3fv(diffuse_colour_id, 1, glm::value_ptr(diffuse_colour));

			const glm::vec3& specular_colour = (glm::vec3&)material.getSpecularColour();
			GLuint specular_colour_id = glGetUniformLocation(shader_program_, "mat.specular_colour");
			glUniform3fv(specular_colour_id, 1, glm::value_ptr(specular_colour));

			const float& shininess = material.getShininess();
			GLuint shininess_id = glGetUniformLocation(shader_program_, "mat.shininess");
			glUniform1f(shininess_id, shininess);

			const bool& is_shiny = material.isShiny();
			GLuint is_shiny_id = glGetUniformLocation(shader_program_, "mat.is_shiny");
			glUniform1i(is_shiny_id, is_shiny);

			//Get material textures and pass to shader
			GLuint diff_texture_id = glGetUniformLocation(shader_program_, "mat.diff_texture");
			GLuint has_diffuse_texture_id = glGetUniformLocation(shader_program_, "mat.has_diffuse_texture");

			if (material.getDiffuseTexture() == "diff0.png")
			{
				glActiveTexture(GL_TEXTURE0 + kDiffTex);
				glBindTexture(GL_TEXTURE_2D, diff0_texture);
				glUniform1i(diff_texture_id, kDiffTex);
				glUniform1i(has_diffuse_texture_id, true);
			}
			else if (material.getDiffuseTexture() == "diff1.png")
			{
				glActiveTexture(GL_TEXTURE0 + kDiffTex);
				glBindTexture(GL_TEXTURE_2D, diff1_texture);
				glUniform1i(diff_texture_id, kDiffTex);
				glUniform1i(has_diffuse_texture_id, true);
			}
			else
				glUniform1i(has_diffuse_texture_id, false);

			GLuint spec_texture_id = glGetUniformLocation(shader_program_, "mat.spec_texture");
			GLuint has_specular_texture_id = glGetUniformLocation(shader_program_, "mat.has_specular_texture");

			if (material.getSpecularTexture() == "spec1.png")
			{
				glActiveTexture(GL_TEXTURE0 + kSpecTex);
				glBindTexture(GL_TEXTURE_2D, spec1_texture);
				glUniform1i(spec_texture_id, kSpecTex);
				glUniform1i(has_specular_texture_id, true);
			}
			else if (material.getSpecularTexture() == "spec2.png")
			{
				glActiveTexture(GL_TEXTURE0 + kSpecTex);
				glBindTexture(GL_TEXTURE_2D, spec2_texture);
				glUniform1i(spec_texture_id, kSpecTex);
				glUniform1i(has_specular_texture_id, true);
			}
			else
				glUniform1i(has_specular_texture_id, false);

			//Render the mesh
			glDrawElements(GL_TRIANGLES, mesh.numElements, GL_UNSIGNED_INT, 0);
		}
	}
}

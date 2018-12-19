#version 330

//Add uniforms to take in the matrix
uniform mat4 combined_matrix;
uniform mat4 world_matrix;

//Add in variables for each of the streamed attributes
in vec3 vertex_position;
in vec3 vertex_normal;
in vec2 texture_coordinates;

//Specify out variables to be varied to the FS
out vec3 varying_position;
out vec3 varying_normal;
out vec2 varying_texture_coordinates;

void main(void)
{
	//Transform the in variables to world space and pass to FS
	varying_position = mat4x3(world_matrix) * vec4(vertex_position, 1.0);
	varying_normal = mat3(world_matrix) * vertex_normal;
	varying_texture_coordinates = (texture_coordinates + 1) / 2;

	gl_Position = combined_matrix * world_matrix * vec4(vertex_position, 1.0);
}

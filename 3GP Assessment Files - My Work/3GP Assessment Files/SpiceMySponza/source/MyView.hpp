#pragma once

#include <sponza/sponza_fwd.hpp>
#include <tygra/WindowViewDelegate.hpp>
#include <tgl/tgl.h>
#include <glm/glm.hpp>
#include <chrono>
#include <vector>
#include <memory>

class MyView : public tygra::WindowViewDelegate
{
public:
    
    MyView();
    
    ~MyView();
    
    void setScene(const sponza::Context * scene);

private:

    void windowViewWillStart(tygra::Window * window) override;
    
    void windowViewDidReset(tygra::Window * window,
                            int width,
                            int height) override;

    void windowViewDidStop(tygra::Window * window) override;
    
    void windowViewRender(tygra::Window * window) override;

private:

    const sponza::Context * scene_;

	std::chrono::system_clock::time_point start_time_;

	GLuint diff0_texture{ 0 };
	GLuint diff1_texture{ 0 };
	GLuint spec1_texture{ 0 };
	GLuint spec2_texture{ 0 };
	GLuint shader_program_{ 0 };

	//Defines values for Vertex attributes
	const static GLuint kNullId = 0;

	enum VertexAttribIndexes
	{
		kVertexPosition = 0,
		kVertexNormal = 1,
		kTextureCoordinates = 2 
	};
	enum FragmentDataIndexes
	{
		kFragmentColour = 0
	};
	enum TextureIndexes
	{
		kDiffTex = 0,
		kSpecTex = 1
	};

	//Create a mesh structure to hold VBO ids etc.
	struct MeshGL
	{
		int id{ 0 };

		//VertexBufferObjects for vertex positions and indices
		GLuint positionVBO{ 0 };
		GLuint normalVBO{ 0 };
		GLuint elementVBO{ 0 };
		GLuint textureCoordinatesVBO{ 0 };

		//VertexArrayObject for shape's vertex array settings
		GLuint vao{ 0 };

		int numElements{ 0 };
	};

	//Create a container of these mesh
	std::vector<MeshGL> m_meshVector;
};

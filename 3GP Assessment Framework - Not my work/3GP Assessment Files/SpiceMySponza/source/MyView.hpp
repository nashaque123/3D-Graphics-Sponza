#pragma once

#include <sponza/sponza_fwd.hpp>
#include <tygra/WindowViewDelegate.hpp>
#include <tgl/tgl.h>
#include <glm/glm.hpp>

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

	// Me from here down
	GLuint shader_program_{ 0 };

	const static GLuint kNullId = 0;

	// TODO: define values for your Vertex attributes

	// TODO: create a mesh structure to hold VBO ids etc.

	// TODO: create a container of these mesh e.g.
	//std::vector<Mesh> m_meshVector;
};

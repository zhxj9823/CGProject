#ifndef RENDERER_H
#define RENDERER_H

#include <map>

#include <glm/glm.hpp>

#include "texture.h"
#include "shader.h"
#include "model.h"
#include "camera.h"
#include"resource_manager.h"


// A renderer class for rendering text displayed by a font loaded using the 
// FreeType library. A single font is loaded, processed into a list of Character
// items for later rendering.
class Renderer
{
public:
	// Shader used for rendering
	Shader ModelShader;
	Model OurModel;
	// Constructor
	Renderer();
	// Pre-compiles a list of characters from the given font
	void Load();
	// Renders a string of text using the precompiled list of characters
	void RenderPlane();
private:
	// Render state
	GLuint VAO, VBO;
	Camera camera;

};

#endif 
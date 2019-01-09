#ifndef RENDERER_H
#define RENDERER_H

#include <map>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "texture.h"
#include "shader.h"
#include "model.h"
#include "camera.h"


// A renderer class for rendering text displayed by a font loaded using the 
// FreeType library. A single font is loaded, processed into a list of Character
// items for later rendering.
class Renderer
{
public:
	Renderer(Shader shader, Model plane);
	void DrawPlane(glm::vec3 position, glm::vec3 size, glm::vec3 Up, glm::vec3 Right, Camera & camera);
private:
	// Render state
	Shader shader;
	Model plane;
};

#endif 
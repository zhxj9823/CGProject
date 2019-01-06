#include "renderer.h"

// The Width of the screen
const GLuint SCREEN_WIDTH = 1280;
// The height of the screen
const GLuint SCREEN_HEIGHT = 960;

Renderer::Renderer(Shader shader, Model plane)
{
	this->shader = shader;
	this->plane = plane;
}

void Renderer::DrawPlane(glm::vec3 position, glm::vec3 size,Camera camera)
{
	// Activate corresponding render state	
	this->shader.Use();
	// view/projection transformations
	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
	glm::mat4 view = camera.GetViewMatrix();
	this->shader.SetMatrix4("projection", projection);
	this->shader.SetMatrix4("view", view);

	// render the loaded model
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, position); // translate it down so it's at the center of the scene
	model = glm::scale(model, size);	// it's a bit too big for our scene, so scale it down
	this->shader.SetMatrix4("model", model);
	this->plane.Draw(this->shader);
}
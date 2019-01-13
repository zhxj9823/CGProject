#include "renderer.h"

// The Width of the screen
const GLuint SCREEN_WIDTH = 1280;
// The height of the screen
const GLuint SCREEN_HEIGHT = 960;

Renderer::Renderer(Model plane, bool t)
{
	this->plane = plane;
	this->isPlane = t;
}

void Renderer::DrawPlane(Shader shader, glm::vec3 position, glm::vec3 size, glm::vec3 right, glm::vec3 up, Camera & camera)
{
	// Activate corresponding render state	
	shader.Use();
	// view/projection transformations
	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
	glm::mat4 view = camera.GetViewMatrix();
	shader.SetMatrix4("projection", projection);
	shader.SetMatrix4("view", view);

	// render the loaded model
	glm::mat4 model = glm::mat4(1.0f);
	right = glm::normalize(right);
	model = glm::translate(model, position); // translate it down so it's at the center of the scene
	glm::vec3 a = glm::cross(right, glm::vec3(-1.0f, 0.0f, 0.0f));
	if (a != glm::vec3(0.0f))
		model = glm::rotate(model, -acos(glm::dot(right, glm::vec3(-1.0f, 0.0f, 0.0f))), a);
	else
		model = glm::rotate(model, -acos(glm::dot(right, glm::vec3(-1.0f, 0.0f, 0.0f))), glm::vec3(0.0f, 1.0f, 0.0f));
	a = glm::cross(up, glm::vec3(0.0f, 1.0f, 0.0f));
	if (a != glm::vec3(0.0f))
		model = glm::rotate(model, acos(glm::dot(up, glm::vec3(0.0f, 1.0f, 0.0f))), a);
	model = glm::scale(model, size);	// it's a bit too big for our scene, so scale it down
	if (this->isPlane)model = glm::translate(model, -glm::vec3(0.0f, 3.6f, 6.0f));
	shader.SetMatrix4("model", model);
	this->plane.Draw(shader);
}
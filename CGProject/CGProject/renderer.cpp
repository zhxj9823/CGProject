#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "renderer.h"
#include "resource_manager.h"

// The Width of the screen
const GLuint SCREEN_WIDTH = 1280;
// The height of the screen
const GLuint SCREEN_HEIGHT = 960;

Renderer::Renderer()
{
	// Load and configure shader
	this->ModelShader = ResourceManager::LoadShader("shaders/model_loading.vs", "shaders/model_loading.fs", nullptr, "text");
	camera=Camera(glm::vec3(0.0f, 0.0f, 5.0f));
}

void Renderer::Load(const char path[])
{
	this->OurModel=Model(path);
}

void Renderer::RenderPlane()
{
	// Activate corresponding render state	
	this->ModelShader.Use();
	// view/projection transformations
	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
	glm::mat4 view = camera.GetViewMatrix();
	ModelShader.SetMatrix4("projection", projection);
	ModelShader.SetMatrix4("view", view);

	// render the loaded model
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f)); // translate it down so it's at the center of the scene
	model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));	// it's a bit too big for our scene, so scale it down
	ModelShader.SetMatrix4("model", model);
	OurModel.Draw(ModelShader);
}
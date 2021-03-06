#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "renderer.h"
#include "camera.h"


// Container object for holding all state relevant for a single
// game object entity. Each object in the game likely needs the
// minimal of state as described within GameObject.
class GameObject
{
public:
	// Object state
	glm::vec3   Position, Size, Velocity;
	glm::vec3	Up, Right;
	glm::vec3	WorldUp;
	glm::vec3	Acceleration;
	GLboolean   Destroyed;
	GameObject();
	GameObject(glm::vec3 pos, glm::vec3 size, glm::vec3 velocity = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f));
	GameObject(glm::vec3 center);
	// Draw
	virtual void Draw(Shader shader,Renderer & renderer, Camera & camera);
	glm::vec3 Move(GLfloat dt);
	double Distance(glm::vec3 center);
	void ProcessKeyboard(Camera_Movement direction, GLfloat deltaTime);
	void Update(GLfloat dt);
};

#endif
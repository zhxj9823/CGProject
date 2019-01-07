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
	GLboolean   Destroyed;
	Camera		camera;
	GameObject();
	GameObject(glm::vec3 pos,glm::vec3 size,Camera camera,glm::vec3 velocity =glm::vec3(0.0f,0.0f,0.0f));
	// Draw
	virtual void Draw(Renderer & renderer);
	glm::vec3 Move(GLfloat dt);
};

#endif
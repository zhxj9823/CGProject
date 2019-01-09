#include "PhysicsEngine.h"
#include <glm/glm.hpp>
#include <glad/glad.h> 
#include <glm/gtc/matrix_transform.hpp>

GLboolean CheckCollision(GameObject &one, GameObject &two)
{
glm::vec3 size= glm::vec3(10.0f,10.0f,10.0f);

	// Collision x-axis?
	bool collisionX = one.Position.x + size.x >= two.Position.x &&
		two.Position.x + size.x >= one.Position.x;
	// Collision y-axis?
	bool collisionY = one.Position.y + size.y >= two.Position.y &&
		two.Position.y + size.y >= one.Position.y;
	bool collisionZ = one.Position.z + size.z >= two.Position.z &&
		two.Position.z + size.z >= one.Position.z;
	// Collision only if on both axes
	return collisionX && collisionY && collisionZ;
}
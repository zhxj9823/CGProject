#include "PhysicsEngine.h"

GLboolean CheckCollision(GameObject &one, GameObject &two) 
{
	
	// Collision x-axis?
	bool collisionX = one.Position.x + one.Size.x >= two.Position.x &&
		two.Position.x + two.Size.x >= one.Position.x;
	// Collision y-axis?
	bool collisionY = one.Position.y + one.Size.y >= two.Position.y &&
		two.Position.y + two.Size.y >= one.Position.y;
	bool collisionZ = one.Position.z + one.Size.z >= two.Position.z &&
		two.Position.z + two.Size.z >= one.Position.z;
	// Collision only if on both axes
	return collisionX && collisionY && collisionZ;
}
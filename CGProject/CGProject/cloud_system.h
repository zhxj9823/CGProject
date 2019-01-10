#pragma once

#include<vector>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "ParticleSystem.h"

class CloudPuff :public Particle
{
	GLuint size;
	GLfloat DistanceToCam;
};

class VolumetricCloud
{
	std::vector<CloudPuff>Puffs;
	glm::vec3 Center, DistanceFromCamera;
	GLfloat Radius, LastCamera, LastLight;

};
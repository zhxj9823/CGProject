#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

#include "shader.h"
#include "texture.h"
#include "game_object.h"

class Particle 
{
	friend class ParticleGenerator;
	glm::vec3 position, velocity;
	glm::vec4 color;
	GLfloat life;

	Particle()
		: position(0.0f), velocity(0.0f), color(1.0f), life(0.0f) { }
};

class ParticleGenerator {
public:
	// Constructor
	ParticleGenerator(Shader shader, Texture texture, GLuint amount);	
	// Render all particles
	void draw(const glm::mat4 & projection, const glm::mat4 & view, const glm::vec3 camera_front, glm::vec3 scale = glm::vec3(1.0f));
	// Update all particles
	void update(GLfloat dt, glm::vec3 pos, glm::vec3 velocity, GLuint newParticles, glm::vec3 offset);
	~ParticleGenerator();
private:
	// State
	std::vector<Particle> particles;
	GLuint amount;
	// Render state
	Shader shader;
	Texture texture;
	GLuint VAO;
	// Initializes buffer and vertex attributes
	void init();
	// Returns the first Particle index that's currently unused e.g. Life <= 0.0f or 0 if no particle is currently inactive
	GLuint firstUnusedParticle();
	// Respawns particle
	void respawnParticle(Particle &particle, glm::vec3 pos, glm::vec3 velocity, glm::vec3 offset);
};
#include "ParticleSystem.h"
#include <iostream>

ParticleGenerator::ParticleGenerator(Shader shader, Texture2D texture, GLuint amount)
	: shader(shader), texture(texture), amount(amount)
{
	init();
}

ParticleGenerator::~ParticleGenerator()
{
}

void ParticleGenerator::update(GLfloat dt, glm::vec3 pos, glm::vec3 velocity, GLuint newParticles, glm::vec3 offset)
{
	// Add new particles
	for (GLuint i = 0; i < newParticles; ++i)
	{
		int unusedParticle = this->firstUnusedParticle();
		// std::cout << "unusedparticle: " << unusedParticle << std::endl;

		this->respawnParticle(this->particles[unusedParticle], pos, velocity, offset);
	}
	// Update all particles
	for (GLuint i = 0; i < this->amount; ++i)
	{
		Particle &p = this->particles[i];
		p.life -= dt; // reduce life
		if (p.life > 0.0f)
		{ // particle is alive, thus update
			p.position -= p.velocity * dt;
			p.color.a -= dt * 2.5;
		}
	}
}

void ParticleGenerator::draw(const glm::mat4 & projection, const glm::mat4 & view, const glm::vec3 camera_front, glm::vec3 scale)
{
	// std::cout << "38" << std::endl;
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	this->shader.Use();
	this->shader.SetInteger("sprite", 0);
	this->shader.SetMatrix4("projection", projection);
	this->shader.SetMatrix4("view", view);
	
	glm::mat4 model = glm::mat4();
	glm::vec3 v1 = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 v2 = camera_front;
	glm::vec3 normal_vec = glm::cross(v1, v2);
	GLfloat angle = glm::acos(glm::dot(v1, v2));

	// std::cout << "particle position: "  << particles[0].position.x << " " << particles[0].position.y << " " << particles[0].position.z << " " <<std::endl;
	// std::cout << "particle speed: "  << particles[0].velocity.x << " " << particles[0].velocity.y << " " << particles[0].velocity.z << " " << std::endl;


	for (Particle particle : this->particles)
	{
		// std::cout <<"life: "<<particle.life << std::endl;
		particle.life += 40;
		if (particle.life > 0.0f)
		{

			model = glm::mat4();
			model = glm::translate(model, particle.position);
			model = glm::rotate(model, angle, normal_vec);
			model = glm::scale(model, scale);
			this->shader.SetMatrix4("model", model);
			this->shader.SetVector4f("color", particle.color);
			glActiveTexture(GL_TEXTURE0);
			this->texture.Bind();
			glBindVertexArray(this->VAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glBindVertexArray(0);
		}
	}
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void ParticleGenerator::init()
{
	// Set up mesh and attribute properties
	GLuint VBO;
	GLfloat particle_quad[] = {
		-1.0f, 1.0f, 0.0f, 1.0f,
		1.0f, -1.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f, 0.0f,

		-1.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f, 0.0f };
	glGenVertexArrays(1, &this->VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(this->VAO);
	// Fill mesh buffer
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(particle_quad), particle_quad, GL_STATIC_DRAW);
	// Set mesh attributes
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid *)0);
	glBindVertexArray(0);

	// Create this->amount default particle instances
	for (GLuint i = 0; i < this->amount; ++i)
		this->particles.push_back(Particle());
}

// Stores the index of the last particle used (for quick access to next dead particle)
GLuint lastUsedParticle = 0;
GLuint ParticleGenerator::firstUnusedParticle()
{
	// First search from last used particle, this will usually return almost instantly
	for (GLuint i = lastUsedParticle; i < this->amount; ++i)
	{
		if (this->particles[i].life <= 0.0f)
		{
			lastUsedParticle = i;
			return i;
		}
	}
	// Otherwise, do a linear search
	for (GLuint i = 0; i < lastUsedParticle; ++i)
	{
		if (this->particles[i].life <= 0.0f)
		{
			lastUsedParticle = i;
			return i;
		}
	}
	// All particles are taken, override the first one (note that if it repeatedly hits this case, more particles should be reserved)
	lastUsedParticle = 0;
	return 0;
}

GLfloat getRandomFloat()
{
	return ((rand() % 100) - 50) / 100.0f;
}

void ParticleGenerator::respawnParticle(Particle &particle, glm::vec3 pos, glm::vec3 velocity, glm::vec3 offset)
{
	GLfloat random1 = ((rand() % 100) - 50) / 50.0f;
	GLfloat random2 = ((rand() % 100) - 50) / 50.0f;
	GLfloat rColor = 0.5 + ((rand() % 100) / 100.0f);
	particle.position = pos + offset;
	particle.color = glm::vec4(rColor, rColor, rColor, 1.0f);
	particle.life = 0.3f;
	particle.velocity = velocity * 0.1f;
	// std::cout << "set life to 1" << std::endl;

}

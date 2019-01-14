#include "ParticleSystemSmoke.h"
#include <iostream>

ParticleGeneratorSmoke::ParticleGeneratorSmoke(Shader shader, Texture texture, GLuint amount)
	: shader(shader), texture(texture), amount(amount),Life(7.0f)
{

	init();
}

ParticleGeneratorSmoke::~ParticleGeneratorSmoke()
{
}

void ParticleGeneratorSmoke::update(GLfloat dt, glm::vec3 pos, glm::vec3 velocity, GLuint newParticles, glm::vec3 offset)
{
	Life -= 1.0f*dt;
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
		p.life -= 2*dt; // reduce life

		if (p.life > 0.0f)
		{ // particle is alive, thus update
			p.position += p.velocity*dt*glm::vec3(0.001f);
			//if(i%2==0)
			//	p.position -= glm::vec3(0.10f) * dt;
			//else
				//p.position += glm::vec3(0.10f) * dt;
			p.color.a -= dt * 0.05;
		}
	}
}

void ParticleGeneratorSmoke::draw(const glm::mat4 & projection, const glm::mat4 & view, const glm::vec3 camera_front, glm::vec3 scale)
{
	// std::cout << "38" << std::endl;
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	this->shader.Use();
	this->shader.SetInteger("sprite", 0);
	this->shader.SetMatrix4("projection", projection);
	this->shader.SetMatrix4("view", view);
	// 可能需要放到里面的循环里去
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
		//particle.life += 1;
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
			particle.life -= 0.1f;
			glBindVertexArray(this->VAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glBindVertexArray(0);
		}
	}
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void ParticleGeneratorSmoke::init()
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
GLuint lastUsedParticle1 = 0;
GLuint ParticleGeneratorSmoke::firstUnusedParticle()
{
	// First search from last used particle, this will usually return almost instantly
	for (GLuint i = lastUsedParticle1; i < this->amount; ++i)
	{
		if (this->particles[i].life <= 0.0f)
		{
			lastUsedParticle1 = i;
			return i;
		}
	}
	// Otherwise, do a linear search
	for (GLuint i = 0; i < lastUsedParticle1; ++i)
	{
		if (this->particles[i].life <= 0.0f)
		{
			lastUsedParticle1 = i;
			return i;
		}
	}
	// All particles are taken, override the first one (note that if it repeatedly hits this case, more particles should be reserved)
	lastUsedParticle1 = 0;
	return 0;
}

void ParticleGeneratorSmoke::respawnParticle(Particle &particle, glm::vec3 pos, glm::vec3 velocity, glm::vec3 offset)
{
	GLfloat random1 = ((rand() % 100) - 50) / 50.0f;
	GLfloat random2 = ((rand() % 100) - 50) / 50.0f;
	GLfloat rColor = 0.5 + ((rand() % 100) / 100.0f);
	particle.position = pos + velocity * 0.01f*glm::vec3((rand() % 1000)-500)/500.0f;
	particle.color = glm::vec4(rColor, rColor, rColor, rColor - 0.5f);
	particle.life = 5.0f;
	particle.velocity = velocity * 0.01f;
	// std::cout << "set life to 1" << std::endl;

}

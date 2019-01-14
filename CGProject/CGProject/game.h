#ifndef GAME_H
#define GAME_H

#include <vector>
#include<list>
#include <tuple>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "camera.h"
#include "game_object.h"
#include "framebuffer.h"
#include "stb_image.h"
#include <irrKlang/irrKlang.h>

#include "text_renderer.h"
#include "renderer.h"
#include"ParticleSystem.h"

// Represents the current state of the game
enum GameState {
	GAME_ACTIVE,
	GAME_MENU,
	GAME_WIN
};

// Represents the current state of the game
enum GameView {
	FIRST_PERSON,
	THIRD_PERSON
};

struct explosionParticle {
	ParticleGenerator* particleSystem;
	GLfloat life;
	glm::vec3 position;

	explosionParticle(Shader shader, Texture texture, GLuint amount, GLfloat life, glm::vec3 pos)
		:particleSystem(new ParticleGenerator(shader, texture, amount)), life(life), position(pos)
	{

	}
};

// Game holds all game-related state and functionality.
// Combines all game-related data into a single class for
// easy access to each of the components and manageability.
class Game
{
public:
	// Game state
	GameState State;
	GameView View;
	GLboolean Keys[1024];
	GLboolean KeysProcessed[1024];
	GLboolean leftMouse, rightMouse;
	GLuint Width, Height;
	Camera cameras[2];
	irrklang::ISoundEngine *SoundEngine;
	TextRenderer *Text;
	Renderer *missle;
	Renderer *Plane;
	Shader skyboxShader;
	GLuint skyboxVAO, skyboxVBO;
	GLuint skyboxTexture;
	GLfloat rendernear;
	GLfloat renderfar;
	GLuint depthMapFBO;
	GLuint depthMap;
	std::vector<GameObject> GameObjects;
	std::list<explosionParticle*> explosionParticles;
	Framebuffer scene_framebuffer;
	Framebuffer cloud_framebuffer;
	Framebuffer pingpong_framebuffer[2];
	// Constructor/Destructor
	Game(GLuint width, GLuint height);
	~Game();
	// Initialize game state (load all shaders/textures/levels)
	void Init();
	// GameLoop
	void ProcessInput(GLfloat dt);
	void Update(GLfloat dt);
	void Render();
	void CreatMissle();
	void render_quad();
	void ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset);
	void ProcessMouseScroll(GLfloat yoffset);
};

#endif
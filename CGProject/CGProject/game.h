#ifndef GAME_H
#define GAME_H
#include <vector>
#include <tuple>
#include <list>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "camera.h"
#include "game_object.h"
#include "framebuffer.h"
#include "stb_image.h"
#include <irrKlang/irrKlang.h>

#include "text_renderer.h"

#include "ParticleSystem.h"

// The Width of the screen
const GLuint SCREEN_WIDTH = 1280;
// The height of the screen
const GLuint SCREEN_HEIGHT = 960;

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
	std::vector<GameObject> GameObjects;
	std::list<explosionParticle*> explosionParticles;
	//Shader skyShader;
	Framebuffer scene_framebuffer;
	Framebuffer cloud_framebuffer;
	Framebuffer pingpong_framebuffer[2];
	Shader skyboxShader;
	GLuint skyboxVAO, skyboxVBO;
	GLuint skyboxTexture;
	// Constructor/Destructor
	Game(GLuint width, GLuint height);
	~Game();
	// Initialize game state (load all shaders/textures/levels)
	void Init();
	// GameLoop
	void ProcessInput(GLfloat dt);
	void Update(GLfloat dt);
	void Render();
	void render_quad();
	void ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset);
	void ProcessMouseScroll(GLfloat yoffset);
};

#endif
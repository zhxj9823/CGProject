#ifndef GAME_H
#define GAME_H
#include <vector>
#include <tuple>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "camera.h"
#include "game_object.h"
#include <irrKlang/irrKlang.h>

#include "text_renderer.h"

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
	// Constructor/Destructor
	Game(GLuint width, GLuint height);
	~Game();
	// Initialize game state (load all shaders/textures/levels)
	void Init();
	// GameLoop
	void ProcessInput(GLfloat dt);
	void Update(GLfloat dt);
	void Render();
	void ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset);
	void ProcessMouseScroll(GLfloat yoffset);
};

#endif
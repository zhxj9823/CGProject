#include <algorithm>
#include <sstream>
#include <iostream>

#include "game.h"
#include "resource_manager.h"
#include "game_object.h"

Renderer *plane;
GameObject *Enemy;
GameObject *Player;


Game::Game(GLuint width, GLuint height) : State(GAME_MENU), View(FIRST_PERSON), Keys(), Width(width), Height(height), leftMouse(GL_FALSE), rightMouse(GL_FALSE)
{
	for (int i = 0; i < 1024; i++)
	{
		Keys[i] = false;
	}
	cameras[0] = Camera(glm::vec3(0.0f, 0.0f, 10.0f));
	cameras[1] = Camera(glm::vec3(0.0f, 0.0f, 100.0f));
}

Game::~Game()
{
	delete Text;
	SoundEngine->drop();
}

void Game::Init()
{
	SoundEngine = irrklang::createIrrKlangDevice();
	if (!SoundEngine) {
		std::cout << "Cannot play the sound" << std::endl;
	}

	SoundEngine->play2D("audio/breakout.mp3", GL_TRUE);

	Text = new TextRenderer(this->Width, this->Height);
	Text->Load("fonts/ocraext.TTF", 24);

	// Load shaders
	ResourceManager::LoadShader("shaders/model_loading.vs", "shaders/model_loading.fs", nullptr, "plane");
	ResourceManager::LoadModel("su33/su33.obj", "su33");
	plane = new Renderer(ResourceManager::GetShader("plane"), ResourceManager::GetModel("su33"));

	glm::vec3 pos = glm::vec3(0.0f);
	glm::vec3 size = glm::vec3(0.5f);
	glm::vec3 v = glm::vec3(0.0f, 0.0f, 0.1f);
	Enemy = new GameObject(pos, size, cameras[FIRST_PERSON], v);
	pos = cameras[FIRST_PERSON].Position;
	v = glm::vec3(0.0f, 0.0f, -0.1f);
	Player = new GameObject(pos, size, cameras[FIRST_PERSON], v);
}

void Game::Update(GLfloat dt)
{
	cameras[FIRST_PERSON].Position += dt * Player->Velocity;
	Enemy->Move(dt);
	Player->Position = cameras[FIRST_PERSON].Position + glm::vec3(0.0f, -1.0f, -1.0f);
}


void Game::ProcessInput(GLfloat dt)
{
	if (State == GAME_ACTIVE)
	{
		if (Keys[GLFW_KEY_W])
		{
			cameras[View].ProcessKeyboard(FORWARD, dt);
		}
		if (Keys[GLFW_KEY_S])
		{
			cameras[View].ProcessKeyboard(BACKWARD, dt);
		}
		if (Keys[GLFW_KEY_A])
		{
			cameras[View].ProcessKeyboard(LEFT, dt);
		}
		if (Keys[GLFW_KEY_D])
		{
			cameras[View].ProcessKeyboard(RIGHT, dt);
		}
		if (Keys[GLFW_KEY_LEFT_SHIFT] || Keys[GLFW_KEY_RIGHT_SHIFT])
		{
			View = (View == FIRST_PERSON) ? THIRD_PERSON : FIRST_PERSON;
		}
	}
}

void Game::Render()
{
	Text->RenderText("Press W or S to select level", 245.0f, Height / 2 + 20.0f, 0.75f);        // Begin rendering to postprocessing quad
	Enemy->camera = cameras[FIRST_PERSON];
	Enemy->Draw(*plane);
	Player->Draw(*plane);
}

void Game::ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset)
{
	if (leftMouse)
	{
		cameras[View].ProcessMouseMovement(xoffset, yoffset);
	}
}

void Game::ProcessMouseScroll(GLfloat yoffset)
{
	cameras[View].ProcessMouseScroll(yoffset);
}
#include <algorithm>
#include <sstream>
#include <iostream>
#include <time.h>

#include "game.h"
#include "resource_manager.h"
#include "game_object.h"
#include "PhysicsEngine.h"

#define EnemyNum 10

Renderer *plane;
GameObject *Enemy[EnemyNum];
GameObject *Player;
std::vector<GameObject> missles;
GLfloat lastTime = -2.0f;


Game::Game(GLuint width, GLuint height) : State(GAME_MENU), View(FIRST_PERSON), Keys(), Width(width), Height(height), leftMouse(GL_FALSE), rightMouse(GL_FALSE)
{
	for (int i = 0; i < 1024; i++)
	{
		Keys[i] = false;
	}
	cameras[0] = Camera(FIRST_PERSON,glm::vec3(0.0f, 0.0f, 10.0f));
	cameras[1] = Camera(THIRD_PERSON,glm::vec3(0.0f, 0.0f, 100.0f));
}

Game::~Game()
{
	delete Text;
	SoundEngine->drop();
}

void Game::Init()
{
	srand((unsigned)time(NULL));

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
	ResourceManager::LoadModel("AVMT300/AIM120D3.obj", "AIM120D");
	//ResourceManager::LoadModel("F-35A/F-35A.obj", "F35A");
	plane = new Renderer(ResourceManager::GetShader("plane"), ResourceManager::GetModel("su33"));
	missle = new Renderer(ResourceManager::GetShader("plane"), ResourceManager::GetModel("AIM120D"));

	ResourceManager::LoadTexture("./particle.png", GL_TRUE, "particle");
	ResourceManager::LoadTexture("./flame.png", GL_TRUE, "flame");
	ResourceManager::LoadShader("shaders/particle.vs", "shaders/particle.frag", nullptr, "particle");

	glm::vec3 pos = cameras[FIRST_PERSON].Position;
	glm::vec3 size = glm::vec3(0.5f);
	glm::vec3 v = glm::vec3(0.0f, 0.0f, -0.1f);
	Player = new GameObject(pos, size, v);
	for (int i = 0; i < EnemyNum; i++) {
		double t = rand();
		double h = (rand() % 1000) / 100 - 5;
		pos = glm::vec3(100 * cos(t), h, 100 * sin(t));
		t = rand();
		double v1 = (rand() % 1000) / 100 + 3;
		v = glm::vec3(v1*cos(t), 0, v1*sin(t));
		Enemy[i] = new GameObject(pos, size, v);
	}
}

void Game::Update(GLfloat dt)
{
	cameras[FIRST_PERSON].Position += dt * Player->Velocity;
	for (int i = 0; i < EnemyNum; i++) {
		Enemy[i]->Move(dt);
		if (Enemy[i]->Destroyed || Enemy[i]->Distance(cameras[FIRST_PERSON].Position) > 100) {
			delete Enemy[i];
			glm::vec3 size = glm::vec3(0.5f);
			double t = rand();
			double h = (rand() % 1000) / 100 - 5;
			glm::vec3 pos = glm::vec3(100 * cos(t), h, 100 * sin(t));
			t = rand();
			double v1 = (rand() % 1000) / 100 + 3;
			glm::vec3 v = glm::vec3(v1*cos(t), 0, v1*sin(t));
			Enemy[i] = new GameObject(pos, size, v);
		}
	}
	for (int i = 0; i < missles.size(); i++)
	{
		missles[i].Move(dt);
		if (missles[i].Destroyed || missles[i].Distance(cameras[FIRST_PERSON].Position) > 100) {
			missles.erase(missles.begin() + i);
			i--;
		}
		//delete &missle;
	}
	Player->Position = cameras[FIRST_PERSON].Position;
	Player->Up = cameras[FIRST_PERSON].PlaneUp;
	Player->Right = cameras[FIRST_PERSON].PlaneRight;
	/*
	for (auto &object1 : GameObjects)
	{
		for (auto &missle : missles) {
			if (CheckCollision(object1, missle)) {
				//±¬Õ¨
				explosionParticle *newep = new explosionParticle(ResourceManager::GetShader("particle"), ResourceManager::GetTexture("flame"), 50, 0.5f, object1.Position);
				explosionParticles.push_back(newep);
			}

		}
	}*/

	for (int i = 0; i < EnemyNum; i++)
	{
		for (auto &missle : missles) {
			if (CheckCollision(*Enemy[i], missle)) {
				//±¬Õ¨
				explosionParticle *newep = new explosionParticle(ResourceManager::GetShader("particle"), ResourceManager::GetTexture("flame"), 50, 0.5f, Enemy[i]->Position);
				explosionParticles.push_back(newep);
				Enemy[i]->Destroyed = true;
				missle.Destroyed = true;
			}

		}
	}

	/*
	for (int i = 0; i < EnemyNum; i++)
	{
			if (CheckCollision(*Enemy[i], *Player)) {
				//±¬Õ¨
				explosionParticle *newep = new explosionParticle(ResourceManager::GetShader("particle"), ResourceManager::GetTexture("flame"), 50, 0.5f, Player->Position);
				explosionParticles.push_back(newep);
			}

	}*/
	for (auto &es : explosionParticles)
	{
		es->life -= dt;
		if (es->life > 0)
			es->particleSystem->update(dt, es->position, glm::vec3(0.0f), 2, glm::vec3(((rand() % 100) - 50) / 50.0f, ((rand() % 100) - 50) / 50.0f, ((rand() % 100) - 50) / 50.0f));
	}
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
		if (Keys[GLFW_KEY_SPACE]) {
			GLfloat currentTime = glfwGetTime();
			if (currentTime - lastTime >= 2.0f) {
				CreatMissle();
				lastTime = currentTime;
			}


		}
	}
}

void Game::CreatMissle() {
	glm::vec3 pos = cameras[FIRST_PERSON].Position;
	glm::vec3 size = glm::vec3(0.05f);
	glm::vec3 v = 20.0f*glm::normalize(glm::cross(cameras[FIRST_PERSON].Up, cameras[FIRST_PERSON].Right));

	GameObject *missle = new GameObject(pos, size, v);
	missles.push_back(*missle);
}

void Game::Render()
{
	Text->RenderText("Press W or S to select level", 245.0f, Height / 2 + 20.0f, 0.75f);        // Begin rendering to postprocessing quad
	for (int i = 0; i < EnemyNum; i++) {
		Enemy[i]->Draw(*plane, cameras[FIRST_PERSON]);
	}
	for (auto &misslee : missles)
	{
		misslee.Draw(*missle, cameras[FIRST_PERSON]);
	}
	Player->Draw(*plane, cameras[FIRST_PERSON]);

	rendernear = 0.1f;
	renderfar = 5000.0f;
	glm::mat4 projection = glm::perspective(cameras[View].Zoom, (float)this->Width / (float)this->Height, rendernear, renderfar);
	glm::mat4 view = cameras[View].GetViewMatrix();
	glDisable(GL_DEPTH_TEST);
	for (auto &es : explosionParticles)
	{
		if (es->life > 0)
		{
			es->particleSystem->draw(projection, view, cameras[View].Front, glm::vec3(10.0f));
		}
	}
	glEnable(GL_DEPTH_TEST);
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
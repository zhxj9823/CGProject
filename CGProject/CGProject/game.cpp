#include <algorithm>
#include <sstream>
#include <iostream>
#include <time.h>

#include "game.h"
#include "resource_manager.h"
#include "game_object.h"
#include "PhysicsEngine.h"
#include "glError.h"

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
	srand((unsigned)time(NULL));

	SoundEngine = irrklang::createIrrKlangDevice();
	if (!SoundEngine)
	{
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
	plane = new Renderer(ResourceManager::GetModel("su33"));
	missle = new Renderer(ResourceManager::GetModel("AIM120D"));

	ResourceManager::LoadTexture("./particle.png", GL_TRUE, "particle");
	ResourceManager::LoadTexture("./flame.png", GL_TRUE, "flame");
	ResourceManager::LoadShader("shaders/particle.vs", "shaders/particle.frag", nullptr, "particle");

	rendernear = 0.1f;
	renderfar = 1000.0f;

	glm::vec3 pos = cameras[FIRST_PERSON].Position;
	glm::vec3 size = glm::vec3(0.5f);
	glm::vec3 v = glm::vec3(0.0f, 0.0f, -0.1f);
	Player = new GameObject(pos, size, v);
	for (int i = 0; i < EnemyNum; i++)
	{
		double t = rand();
		double h = (rand() % 1000) / 100 - 5;
		pos = glm::vec3(100 * cos(t), h, 100 * sin(t));
		t = rand();
		double v1 = (rand() % 1000) / 100 + 3;
		v = glm::vec3(v1 * cos(t), 0, v1 * sin(t));
		Enemy[i] = new GameObject(pos, size, v);
	}

	ResourceManager::LoadShader("shaders/shadow_mapping_depth.vs", "shaders/shadow_mapping_depth.fs", nullptr, "simpleDepthShader");
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	glGenFramebuffers(1, &depthMapFBO);
	// create depth texture
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = {1.0, 1.0, 1.0, 1.0};
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Framebuffer Error" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	ResourceManager::GetShader("plane").Use();
	ResourceManager::GetShader("plane").SetInteger("diffuseTexture", 0);
	ResourceManager::GetShader("plane").SetInteger("shadowMap", 1);
	glGetError();

	ResourceManager::LoadShader("./resources/shaders/skybox.vert", "./resources/shaders/skybox.frag", nullptr, "skybox_shader");
	skyboxShader = ResourceManager::GetShader("skybox_shader");
	float skyboxVertices[] = {
		// back
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		// left
		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,
		// right
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 // front
		 -1.0f, -1.0f,  1.0f,
		 -1.0f,  1.0f,  1.0f,
		  1.0f,  1.0f,  1.0f,
		  1.0f,  1.0f,  1.0f,
		  1.0f, -1.0f,  1.0f,
		 -1.0f, -1.0f,  1.0f,
		 // top
		 -1.0f,  1.0f, -1.0f,
		  1.0f,  1.0f, -1.0f,
		  1.0f,  1.0f,  1.0f,
		  1.0f,  1.0f,  1.0f,
		 -1.0f,  1.0f,  1.0f,
		 -1.0f,  1.0f, -1.0f,
		 // bottom
		 -1.0f, -1.0f, -1.0f,
		 -1.0f, -1.0f,  1.0f,
		  1.0f, -1.0f, -1.0f,
		  1.0f, -1.0f, -1.0f,
		 -1.0f, -1.0f,  1.0f,
		  1.0f, -1.0f,  1.0f
	};

	// skybox VAO
	//unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	//glCheckError();
	std::vector<std::string> faces
	{
		"resources/textures/skybox/right.jpg",
		"resources/textures/skybox/left.jpg",
		"resources/textures/skybox/top.jpg",
		"resources/textures/skybox/bottom.jpg",
		"resources/textures/skybox/front.jpg",
		"resources/textures/skybox/back.jpg"
	};

	//unsigned int skyboxTexture;
	glGenTextures(1, &skyboxTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);

	int width, height, nrChannels;

	// draw six faces: right, left, top, bottom, front, back
	for (unsigned int i = 0; i < 6; i++)
	{
		unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	//glCheckError();
	// set texture uniform
	skyboxShader = ResourceManager::GetShader("skybox_shader");
	skyboxShader.Use();
	skyboxShader.SetInteger("skybox", 0);
	//glCheckError();
}

void Game::Update(GLfloat dt)
{
	cameras[FIRST_PERSON].Position += dt * Player->Velocity;
	for (int i = 0; i < EnemyNum; i++)
	{
		Enemy[i]->Move(dt);
		if (Enemy[i]->Destroyed || Enemy[i]->Distance(cameras[FIRST_PERSON].Position) > 100)
		{
			delete Enemy[i];
			glm::vec3 size = glm::vec3(0.5f);
			double t = rand();
			double h = (rand() % 1000) / 100 - 5;
			glm::vec3 pos = glm::vec3(100 * cos(t), h, 100 * sin(t));
			t = rand();
			double v1 = (rand() % 1000) / 100 + 3;
			glm::vec3 v = glm::vec3(v1 * cos(t), 0, v1 * sin(t));
			Enemy[i] = new GameObject(pos, size, v);
		}
	}
	for (int i = 0; i < missles.size(); i++)
	{
		missles[i].Move(dt);
		if (missles[i].Destroyed || missles[i].Distance(cameras[FIRST_PERSON].Position) > 100)
		{
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
				//爆炸
				explosionParticle *newep = new explosionParticle(ResourceManager::GetShader("particle"), ResourceManager::GetTexture("flame"), 50, 0.5f, object1.Position);
				explosionParticles.push_back(newep);
			}

		}
	}*/

	for (int i = 0; i < EnemyNum; i++)
	{
		for (auto &missle : missles)
		{
			if (CheckCollision(*Enemy[i], missle))
			{
				//爆炸
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
			//爆炸
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
		if (Keys[GLFW_KEY_SPACE])
		{
			GLfloat currentTime = glfwGetTime();
			if (currentTime - lastTime >= 2.0f)
			{
				CreatMissle();
				lastTime = currentTime;
			}
		}
	}
}

void Game::CreatMissle()
{
	glm::vec3 pos = cameras[FIRST_PERSON].Position;
	glm::vec3 size = glm::vec3(0.05f);
	glm::vec3 v = 20.0f * glm::normalize(glm::cross(cameras[FIRST_PERSON].Up, cameras[FIRST_PERSON].Right));

	GameObject *missle = new GameObject(pos, size, v);
	missles.push_back(*missle);
}

void Game::Render()
{
	Text->RenderText("Press W or S to select level", 245.0f, Height / 2 + 20.0f, 0.75f); // Begin rendering to postprocessing quad

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	glm::vec3 lightPos(-20.0f, 40.0f, -10.0f);
	// 1. render depth of scene to texture (from light's perspective)
	// --------------------------------------------------------------
	glm::mat4 lightProjection, lightView;
	glm::mat4 lightSpaceMatrix;
	float near_plane = 1.0f, far_plane = 7.5f;
	lightProjection = glm::perspective(glm::radians(45.0f), (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT, near_plane, far_plane); // note that if you use a perspective projection matrix you'll have to change the light position as the current light position isn't enough to reflect the whole scene
	//lightProjection = glm::ortho(-100.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
	lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
	lightSpaceMatrix = lightProjection * lightView;
	// render scene from light's point of view
	ResourceManager::GetShader("simpleDepthShader").Use();
	ResourceManager::GetShader("simpleDepthShader").SetMatrix4("lightSpaceMatrix", lightSpaceMatrix);
	glGetError();
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	for (int i = 0; i < EnemyNum; i++)
	{
		Enemy[i]->Draw(ResourceManager::GetShader("simpleDepthShader"), *plane, cameras[FIRST_PERSON]);
	}
	for (auto &misslee : missles)
	{
		misslee.Draw(ResourceManager::GetShader("simpleDepthShader"), *missle, cameras[FIRST_PERSON]);
	}
	Player->Draw(ResourceManager::GetShader("simpleDepthShader"), *plane, cameras[FIRST_PERSON]);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glGetError();
	// reset viewport
	glViewport(0, 0, Width, Height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 2. render scene as normal using the generated depth/shadow map
	// --------------------------------------------------------------
	glViewport(0, 0, Width, Height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	ResourceManager::GetShader("plane").Use();
	glm::mat4 projection = glm::perspective(glm::radians(cameras[View].Zoom), (float)Width / (float)Height, rendernear, renderfar);
	glm::mat4 view = cameras[View].GetViewMatrix();
	ResourceManager::GetShader("plane").SetMatrix4("projection", projection);
	ResourceManager::GetShader("plane").SetMatrix4("view", view);
	// set light uniforms
	ResourceManager::GetShader("plane").SetVector3f("viewPos", cameras[View].Position);
	ResourceManager::GetShader("plane").SetVector3f("lightPos", lightPos);
	ResourceManager::GetShader("plane").SetMatrix4("lightSpaceMatrix", lightSpaceMatrix);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	for (int i = 0; i < EnemyNum; i++)
	{
		Enemy[i]->Draw(ResourceManager::GetShader("plane"),*plane, cameras[FIRST_PERSON]);
	}
	for (auto &misslee : missles)
	{
		misslee.Draw(ResourceManager::GetShader("plane"),*missle, cameras[FIRST_PERSON]);
	}
	Player->Draw(ResourceManager::GetShader("plane"),*plane, cameras[FIRST_PERSON]);
	glGetError();
	projection = glm::perspective(cameras[View].Zoom, (float)this->Width / (float)this->Height, rendernear, renderfar);
	view = cameras[View].GetViewMatrix();
	for (auto &es : explosionParticles)
	{
		if (es->life > 0)
		{
			es->particleSystem->draw(projection, view, cameras[View].Front, glm::vec3(10.0f));
		}
	}
	// draw skybox
	glDepthFunc(GL_LEQUAL);
	//glDepthMask(GL_FALSE);
	skyboxShader.Use();
	projection = glm::perspective(glm::radians(cameras[View].Zoom), (float)Width / (float)Height, 0.1f, 1000.0f);
	view = glm::mat4(glm::mat3(cameras[View].GetViewMatrix())); // reset the view matrix
	skyboxShader.SetMatrix4("projection", projection);  // set projection uniform to the shader
	skyboxShader.SetMatrix4("view", view); // set view uniform to the shader

	// skybox cube
	glBindVertexArray(skyboxVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	//glDepthMask(GL_TRUE); // set depth function back to default
	glDepthFunc(GL_LESS);
	glGetError();
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
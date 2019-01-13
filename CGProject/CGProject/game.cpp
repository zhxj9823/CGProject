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
	cameras[0] = Camera(0,glm::vec3(0.0f, 0.0f, 10.0f));
	cameras[1] = Camera(1,glm::vec3(0.0f, 0.0f, 100.0f));
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
	plane = new Renderer(ResourceManager::GetShader("plane"), ResourceManager::GetModel("su33"),true);
	missle = new Renderer(ResourceManager::GetShader("plane"), ResourceManager::GetModel("AIM120D"),false);

	ResourceManager::LoadTexture("./particle.png", GL_TRUE, "particle");
	ResourceManager::LoadTexture("./flame.png", GL_TRUE, "flame");
	ResourceManager::LoadShader("shaders/particle.vs", "shaders/particle.frag", nullptr, "particle");

	glm::vec3 pos = cameras[FIRST_PERSON].Position;
	glm::vec3 size = glm::vec3(0.5f);
	glm::vec3 v = glm::vec3(0.0f, 0.0f, -1.0f);
	Player = new GameObject(pos, size, v);
	for (int i = 0; i < EnemyNum; i++) {
		Enemy[i] = new GameObject(cameras[FIRST_PERSON].Position);
	}


	// Load shaders

	//ResourceManager::LoadShader("./resources/shaders/sky.vert", "./resources/shaders/sky.frag", nullptr, "sky_shader");

	//ResourceManager::LoadShader("./resources/shaders/terrain.vert", "./resources/shaders/terrain.frag", nullptr, "terrain_shader");

	ResourceManager::LoadShader("./resources/shaders/resolve.vert", "./resources/shaders/resolve_noise.frag", nullptr, "resolve_shader");

	ResourceManager::LoadShader("./resources/shaders/blur.vert", "./resources/shaders/blur.frag", nullptr, "blur_shader");

	ResourceManager::LoadShader("./resources/shaders/post.vert", "./resources/shaders/post.frag", nullptr, "post_shader");

	ResourceManager::LoadShader("./resources/shaders/skybox.vert", "./resources/shaders/skybox.frag", nullptr, "skybox_shader");




	/* Load models */





	// Load textures

	//ResourceManager::LoadTexture2DFromEx5("./resources/textures/terrain.ex5", "terrain");







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
	Player->Update(dt);
	for (int i = 0; i < EnemyNum; i++) {
		Enemy[i]->Move(dt);
		if (Enemy[i]->Destroyed || Enemy[i]->Distance(cameras[FIRST_PERSON].Position) > 100) {
			delete Enemy[i];
			Enemy[i] = new GameObject(cameras[FIRST_PERSON].Position);
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
	cameras[FIRST_PERSON].Position=Player->Position;
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
			Player->ProcessKeyboard(FORWARD, dt);
		}
		if (Keys[GLFW_KEY_S])
		{
			cameras[View].ProcessKeyboard(BACKWARD, dt);
			Player->ProcessKeyboard(BACKWARD, dt);
		}
		if (Keys[GLFW_KEY_A])
		{
			cameras[View].ProcessKeyboard(LEFT, dt);
			Player->ProcessKeyboard(LEFT, dt);
		}
		if (Keys[GLFW_KEY_D])
		{
			cameras[View].ProcessKeyboard(RIGHT, dt);
			Player->ProcessKeyboard(RIGHT, dt);
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
	glm::vec3 v = 50.0f*glm::normalize(glm::cross(cameras[FIRST_PERSON].Up, cameras[FIRST_PERSON].Right));
	glm::vec3 up = cameras[FIRST_PERSON].Up;

	GameObject *missle = new GameObject(pos, size, v, up);
	missles.push_back(*missle);
}

void Game::Render()
{
	glm::mat4 view = cameras[View].GetViewMatrix();
	glm::mat4 projection = glm::perspective(glm::radians(cameras[View].Zoom), (float)Width / (float)Height, 0.1f, 1000.0f);
	//glm::mat4 model;
	//model = glm::translate(model, glm::vec3(0.0f,-10.0f,0.0f));

	ResourceManager::GetShader("resolve_shader").Use();
	ResourceManager::GetShader("resolve_shader").SetMatrix4("view", view);
	ResourceManager::GetShader("resolve_shader").SetMatrix4("proj", projection);
	//ResourceManager::GetShader("resolve_shader").SetMatrix4("model", model);
	ResourceManager::GetShader("resolve_shader").SetMatrix4("inv_view", glm::mat4(glm::inverse(view)));
	ResourceManager::GetShader("resolve_shader").SetMatrix4("inv_proj", glm::mat4(glm::inverse(projection)));
	ResourceManager::GetShader("resolve_shader").SetVector2f("view_port", glm::vec2(Width, Height));
	ResourceManager::GetShader("resolve_shader").SetVector3f("camera_pos", glm::vec3(cameras[View].Position));
	ResourceManager::GetShader("resolve_shader").SetVector3f("sun_pos", glm::vec3(cameras[View].Position + glm::vec3(1000.0f, 1000.0f, 1000.0f)));

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
	std::cout <<"PlayerPosition:" <<Player->Position.x<< " "<<Player->Position.y<<" "<< Player->Position.z << std::endl;
	std::cout << "Player V:" << Player->Velocity.x << " " << Player->Velocity.y << " " << Player->Velocity .z << endl;
	std::cout << "Player Up:" << Player->Up.x << " " << Player->Up.y << " " << Player->Up.z << endl;

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
	projection = glm::perspective(cameras[View].Zoom, (float)this->Width / (float)this->Height, rendernear, renderfar);
	view = cameras[View].GetViewMatrix();
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
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
	if (!SoundEngine)
	{
		std::cout << "Cannot play the sound" << std::endl;
	}

	SoundEngine->play2D("audio/breakout.mp3", GL_TRUE);

	Text = new TextRenderer(this->Width, this->Height);
	Text->Load("fonts/ocraext.TTF", 24);

	// Load shaders
	ResourceManager::LoadShader("./resources/shaders/resolve.vert", "./resources/shaders/resolve_noise.frag", nullptr, "resolve_shader");
	ResourceManager::LoadShader("./resources/shaders/blur.vert", "./resources/shaders/blur.frag", nullptr, "blur_shader");
	ResourceManager::LoadShader("./resources/shaders/post.vert", "./resources/shaders/post.frag", nullptr, "post_shader");
	//ResourceManager::LoadShader("./resources/shaders/skybox.vert", "./resources/shaders/skybox.frag", nullptr, "skybox_shader");
		
	framebuffer_scene_init(&scene_framebuffer, Width, Height);
	framebuffer_cloud_init(&cloud_framebuffer, Width, Height);
	framebuffer_pingpong_init(pingpong_framebuffer, Width, Height);
	ResourceManager::LoadShader("shaders/model_loading.vs", "shaders/model_loading.fs", nullptr, "plane");
	ResourceManager::LoadModel("su33/su33.obj", "su33");
	ResourceManager::LoadModel("AVMT300/AIM120D3.obj", "AIM120D");
	//ResourceManager::LoadModel("F-35A/F-35A.obj", "F35A");
	plane = new Renderer(ResourceManager::GetModel("su33"), true);
	missle = new Renderer(ResourceManager::GetModel("AIM120D"), false);

	ResourceManager::LoadTexture("./particle.png", GL_TRUE, "particle");
	ResourceManager::LoadTexture("./flame.png", GL_TRUE, "flame");
	ResourceManager::LoadShader("shaders/particle.vs", "shaders/particle.frag", nullptr, "particle");

// Load textures
	//ResourceManager::LoadTexture2DFromEx5("./resources/textures/terrain.ex5", "terrain");
	ResourceManager::LoadTexture3DFromEx5("./resources/textures/noise5.ex5","cloud");
	ResourceManager::LoadTexture1DPhase("./resources/textures/phase.txt", "mie_texture");
	
	ResourceManager::cloud_preprocess(ResourceManager::GetTexture("cloud"),"cloud_structure_texture");

	ResourceManager::GetShader("resolve_shader").Use();
	ResourceManager::SendShaderTexture3D(ResourceManager::GetShader("resolve_shader"), ResourceManager::GetTexture("cloud"), "cloud_texture");
	ResourceManager::SendShaderTexture3D(ResourceManager::GetShader("resolve_shader"), ResourceManager::GetTexture("cloud_structure_texture"), "cloud_structure");
	//ResourceManager::SendShaderTexture2D(ResourceManager::GetShader("terrain_shader"), ResourceManager::GetTexture("terrain"), "terrain_texture");
	ResourceManager::SendShaderTexture1D(ResourceManager::GetShader("resolve_shader"), ResourceManager::GetTexture("mie_texture"), "mie_texture");

	rendernear = 0.1f;
	renderfar = 1000.0f;

	glm::vec3 pos = cameras[FIRST_PERSON].Position;
	glm::vec3 size = glm::vec3(0.5f);
	glm::vec3 v = glm::vec3(0.0f, 0.0f, -1.0f);
	Player = new GameObject(pos, size, v);
	for (int i = 0; i < EnemyNum; i++)
	{
		Enemy[i] = new GameObject(cameras[FIRST_PERSON].Position);
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
	//skyboxShader = ResourceManager::GetShader("skybox_shader");
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
	}
	cameras[FIRST_PERSON].Position = Player->Position;
	
	for (int i = 0; i < EnemyNum; i++)
	{
		for (auto &missle : missles) {
			if (CheckCollision(*Enemy[i], missle)) {
				//爆炸
				explosionParticle *newep = new explosionParticle(ResourceManager::GetShader("particle"), ResourceManager::GetTexture("flame"), 50, 0.5f, Enemy[i]->Position);
				explosionParticles.push_back(newep);
				Enemy[i]->Destroyed = true;
				missle.Destroyed = true;
			}

		}
	}

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
	//Text->RenderText("Press W or S to select level", 245.0f, Height / 2 + 20.0f, 0.75f); // Begin rendering to postprocessing quad
	glm::mat4 view = cameras[View].GetViewMatrix();
	glm::mat4 projection = glm::perspective(glm::radians(cameras[View].Zoom), (float)Width / (float)Height, 0.1f, 1000.0f);
	//glm::mat4 model;
	//model = glm::translate(model, glm::vec3(0.0f,-10.0f,0.0f));
	glm::vec3 lightPos(100 * glm::cos(glfwGetTime()), 100.0f, 100 * glm::sin(glfwGetTime()));

	ResourceManager::GetShader("resolve_shader").Use();
	ResourceManager::GetShader("resolve_shader").SetMatrix4("view", view);
	ResourceManager::GetShader("resolve_shader").SetMatrix4("proj", projection);
	//ResourceManager::GetShader("resolve_shader").SetMatrix4("model", model);
	ResourceManager::GetShader("resolve_shader").SetMatrix4("inv_view", glm::mat4(glm::inverse(view)));
	ResourceManager::GetShader("resolve_shader").SetMatrix4("inv_proj", glm::mat4(glm::inverse(projection)));
	ResourceManager::GetShader("resolve_shader").SetVector2f("view_port", glm::vec2(Width, Height));
	ResourceManager::GetShader("resolve_shader").SetVector3f("camera_pos", glm::vec3(cameras[View].Position));
	ResourceManager::GetShader("resolve_shader").SetVector3f("sun_pos", lightPos);

	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	
	// 1. render depth of scene to texture (from light's perspective)
	// --------------------------------------------------------------
	glm::mat4 lightProjection, lightView;
	glm::mat4 lightSpaceMatrix;
	float near_plane = 0.1f, far_plane = 5000.0f;
	//lightProjection = glm::perspective(glm::radians(45.0f), (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT, near_plane, far_plane); // note that if you use a perspective projection matrix you'll have to change the light position as the current light position isn't enough to reflect the whole scene
	lightProjection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, near_plane, far_plane);
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
	
	/* Render skybox and text */
	glBindFramebuffer(GL_FRAMEBUFFER, scene_framebuffer.fbo);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glGetError();
	// reset viewport

	// 2. render scene as normal using the generated depth/shadow map
	// --------------------------------------------------------------
	glViewport(0, 0, Width, Height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	ResourceManager::GetShader("plane").Use();
	projection = glm::perspective(glm::radians(cameras[View].Zoom), (float)Width / (float)Height, rendernear, renderfar);
	view = cameras[View].GetViewMatrix();
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
	glDisable(GL_DEPTH_TEST);
	for (auto &es : explosionParticles)
	{
		if (es->life > 0)
		{
			es->particleSystem->draw(projection, view, cameras[View].Front, glm::vec3(10.0f));
		}
	}
	glEnable(GL_DEPTH_TEST);

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

	
	/* Render clouds via resolve shader */
	glBindFramebuffer(GL_FRAMEBUFFER, cloud_framebuffer.fbo);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ResourceManager::GetShader("resolve_shader").Use();
		
	glActiveTexture(GL_TEXTURE10);
	glBindTexture(GL_TEXTURE_2D, scene_framebuffer.color_buffer[0]);	
	GLuint diffuse_buffer = glGetUniformLocation(ResourceManager::GetShader("resolve_shader").ID, "diffuse_buffer");
	glUniform1i(diffuse_buffer, 10);

	render_quad();
	
	/* Pinpong buffers for bloom */
	GLboolean horizontal = true, first_iteration = true;
	GLuint amount = 10;
	ResourceManager::GetShader("blur_shader").Use();
	for (GLuint i = 0; i < amount; i++) {
		glBindFramebuffer(GL_FRAMEBUFFER, pingpong_framebuffer[horizontal].fbo);
		glUniform1i(glGetUniformLocation(ResourceManager::GetShader("blur_shader").ID, "horizontal"), horizontal);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, first_iteration ? cloud_framebuffer.color_buffer[1] : pingpong_framebuffer[!horizontal].color_buffer[0]);
		GLuint image = glGetUniformLocation(ResourceManager::GetShader("blur_shader").ID, "image");
		glUniform1i(image, 5);
		render_quad();		
		horizontal = !horizontal;
		if (first_iteration)
			first_iteration = false;
	}
	
	/* Render to screen via post shader */
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ResourceManager::GetShader("post_shader").Use();	
	glActiveTexture(GL_TEXTURE11);
	glBindTexture(GL_TEXTURE_2D, cloud_framebuffer.color_buffer[0]);
	GLuint hdr_buffer = glGetUniformLocation(ResourceManager::GetShader("post_shader").ID, "HDR_buffer");
	glUniform1i(hdr_buffer, 11);

	glActiveTexture(GL_TEXTURE12);
	glBindTexture(GL_TEXTURE_2D, pingpong_framebuffer[!horizontal].color_buffer[0]);
	GLuint bloom = glGetUniformLocation(ResourceManager::GetShader("post_shader").ID, "bloom_blur");
	glUniform1i(bloom, 12);
	
	render_quad();	
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

void Game::render_quad()
{
	static GLuint quad_vao = 0;
	static GLuint quad_vbo = 0;
	if (quad_vao == 0) {
		GLfloat quadVertices[] = {
			// Positions        // Texture Coords
			-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		glGenVertexArrays(1, &quad_vao);
		glGenBuffers(1, &quad_vbo);
		glBindVertexArray(quad_vao);
		glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);
		
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);
	}
	glBindVertexArray(quad_vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

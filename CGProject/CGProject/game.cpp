#include <algorithm>
#include <sstream>
#include <iostream>

#include "game.h"
#include "resource_manager.h"
#include "glError.h"

Game::Game(GLuint width, GLuint height) : State(GAME_MENU), Width(width), Height(height), leftMouse(GL_FALSE), rightMouse(GL_FALSE)
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
	//ResourceManager::LoadShader("./resources/shaders/sky.vert", "./resources/shaders/sky.frag", nullptr, "sky_shader");
	//ResourceManager::LoadShader("./resources/shaders/terrain.vert", "./resources/shaders/terrain.frag", nullptr, "terrain_shader");
	ResourceManager::LoadShader("./resources/shaders/resolve.vert", "./resources/shaders/resolve_noise.frag", nullptr, "resolve_shader");
	ResourceManager::LoadShader("./resources/shaders/blur.vert", "./resources/shaders/blur.frag", nullptr, "blur_shader");
	ResourceManager::LoadShader("./resources/shaders/post.vert", "./resources/shaders/post.frag", nullptr, "post_shader");
	ResourceManager::LoadShader("./resources/shaders/skybox.vert", "./resources/shaders/skybox.frag", nullptr, "skybox_shader");
		
	framebuffer_scene_init(&scene_framebuffer, SCREEN_WIDTH, SCREEN_HEIGHT);
	framebuffer_cloud_init(&cloud_framebuffer, SCREEN_WIDTH, SCREEN_HEIGHT);
	framebuffer_pingpong_init(pingpong_framebuffer, SCREEN_WIDTH, SCREEN_HEIGHT);
	/* Load models */
	

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
	
}


void Game::ProcessInput(GLfloat dt)
{
	if(State==GAME_ACTIVE)
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
	std::cout<<"Rendering"<<std::endl;
	glm::mat4 view = cameras[View].GetViewMatrix();
	glm::mat4 projection = glm::perspective(glm::radians(cameras[View].Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 1000.0f);
	//glm::mat4 model;
	//model = glm::translate(model, glm::vec3(0.0f,-10.0f,0.0f));

	ResourceManager::GetShader("resolve_shader").Use();
	ResourceManager::GetShader("resolve_shader").SetMatrix4("view", view);
	ResourceManager::GetShader("resolve_shader").SetMatrix4("proj", projection);
	//ResourceManager::GetShader("resolve_shader").SetMatrix4("model", model);
	ResourceManager::GetShader("resolve_shader").SetMatrix4("inv_view", glm::mat4(glm::inverse(view)));
	ResourceManager::GetShader("resolve_shader").SetMatrix4("inv_proj", glm::mat4(glm::inverse(projection)));
	ResourceManager::GetShader("resolve_shader").SetVector2f("view_port", glm::vec2(SCREEN_WIDTH, SCREEN_HEIGHT));
	ResourceManager::GetShader("resolve_shader").SetVector3f("camera_pos", glm::vec3(cameras[View].Position));
	ResourceManager::GetShader("resolve_shader").SetVector3f("sun_pos", glm::vec3(cameras[View].Position + glm::vec3(1000.0f, 1000.0f, 1000.0f)));

	/* Render skybox and text */
	glBindFramebuffer(GL_FRAMEBUFFER, scene_framebuffer.fbo);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Text->RenderText("Press W or S to select level", 245.0f, Height / 2 + 20.0f, 0.75f);
	
	// draw skybox
	glDepthFunc(GL_LEQUAL);
	//glDepthMask(GL_FALSE);
	skyboxShader.Use();
	projection = glm::perspective(glm::radians(cameras[View].Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 1000.0f);
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
	if(leftMouse)
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

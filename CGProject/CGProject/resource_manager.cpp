#define _CRT_SECURE_NO_WARNINGS

#include "resource_manager.h"

#include <cstdio>
#include <cstring>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <fstream>

#include "stb_image.h"

// Instantiate static variables
std::map<std::string, Texture> ResourceManager::Textures;
std::map<std::string, Shader> ResourceManager::Shaders;

Shader ResourceManager::LoadShader(const GLchar *vShaderFile, const GLchar *fShaderFile, const GLchar *gShaderFile, std::string name)
{
	Shaders[name] = loadShaderFromFile(vShaderFile, fShaderFile, gShaderFile);
	return Shaders[name];
}

Shader ResourceManager::GetShader(std::string name)
{
	return Shaders[name];
}

Texture ResourceManager::LoadTexture(const GLchar *file, GLboolean alpha, std::string name)
{
	Textures[name] = loadTextureFromFile(file, alpha);
	return Textures[name];
}

Texture ResourceManager::GetTexture(std::string name)
{
	return Textures[name];
}

void ResourceManager::Clear()
{
	// (Properly) delete all shaders
	for (auto iter : Shaders)
		glDeleteProgram(iter.second.ID);
	// (Properly) delete all textures
	for (auto iter : Textures)
		glDeleteTextures(1, &iter.second.ID);
}

Shader ResourceManager::loadShaderFromFile(const GLchar *vShaderFile, const GLchar *fShaderFile, const GLchar *gShaderFile)
{
	// 1. Retrieve the vertex/fragment source code from filePath
	std::string vertexCode;
	std::string fragmentCode;
	std::string geometryCode;
	try
	{
		// Open files
		std::ifstream vertexShaderFile(vShaderFile);
		std::ifstream fragmentShaderFile(fShaderFile);
		std::stringstream vShaderStream, fShaderStream;
		// Read file's buffer contents into streams
		vShaderStream << vertexShaderFile.rdbuf();
		fShaderStream << fragmentShaderFile.rdbuf();
		// close file handlers
		vertexShaderFile.close();
		fragmentShaderFile.close();
		// Convert stream into string
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
		// If geometry shader path is present, also load a geometry shader
		if (gShaderFile != nullptr)
		{
			std::ifstream geometryShaderFile(gShaderFile);
			std::stringstream gShaderStream;
			gShaderStream << geometryShaderFile.rdbuf();
			geometryShaderFile.close();
			geometryCode = gShaderStream.str();
		}
	}
	catch (std::exception e)
	{
		std::cout << "ERROR::SHADER: Failed to read shader files" << std::endl;
	}
	const GLchar *vShaderCode = vertexCode.c_str();
	const GLchar *fShaderCode = fragmentCode.c_str();
	const GLchar *gShaderCode = geometryCode.c_str();
	// 2. Now create shader object from source code
	Shader shader;
	shader.Compile(vShaderCode, fShaderCode, gShaderFile != nullptr ? gShaderCode : nullptr);
	return shader;
}

Texture ResourceManager::loadTextureFromFile(const GLchar *file, GLboolean alpha)
{
	// Create Texture object
	Textures[file] = *(new Texture);
	if (alpha)
	{
		Textures[file].Internal_Format = GL_RGBA;
		Textures[file].Image_Format = GL_RGBA;
	}
	// Load image
	int width, height, nrChannels;
	unsigned char *image = stbi_load(file, &width, &height, &nrChannels, 0);
	// Now generate texture
	Textures[file].Generate(width, height, image);
	// And finally free image data
	stbi_image_free(image);
	return Textures[file];
}

void ResourceManager::LoadTexture1DPhase(const GLchar *file, std::string name)
{
	// Create Texture object
	Textures[name] = *(new Texture);

	FILE *fp = fopen(file, "r");

	if (fp == NULL)
	{
		std::cout << "Cannot open file:" << file << std::endl;
	}

	/* Create image */
	uint8_t* data = (uint8_t*)calloc(1801, sizeof *data);

	/* Read image*/
	int i = 0;
	char value[128];
	while (fgets(value, 128, fp))
	{
		value[strlen(value) - 1] = '\0';
		data[i++] = (uint8_t)(255 * atof(value));
	}

	glGenTextures(1, &Textures[file].ID);
	glBindTexture(GL_TEXTURE_1D, Textures[file].ID);

	glTexImage1D(GL_TEXTURE_1D, 0, GL_RED, 1800, 0, GL_RED, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_1D);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);

	free(data);
}

void ResourceManager::LoadTexture2DFromEx5(const GLchar *file, std::string name)
{
	// Create Texture object
	Textures[name] = *(new Texture);

	FILE *fp = fopen(file, "r");

	if (fp == NULL)
	{
		std::cout << "Cannot open file:" << file << std::endl;
	}

	/* Read header */
	fscanf(fp, "%d", &Textures[name].Width);
	fscanf(fp, "%d", &Textures[name].Height);

	/* Create image */
	uint8_t* data = (uint8_t*)calloc(Textures[name].Width * Textures[name].Height * 4, sizeof *data);

	/* Read image*/
	int i = 0;
	uint32_t pixel;
	while (fscanf(fp, "%d", &pixel) == 1)
	{
		data[i++] = pixel >> 24;
		data[i++] = pixel >> 16;
		data[i++] = pixel >> 8;
		data[i++] = pixel >> 0;
	}

	glGenTextures(1, &Textures[name].ID);
	glBindTexture(GL_TEXTURE_2D, Textures[name].ID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Textures[name].Width, Textures[name].Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	free(data);
}

void ResourceManager::LoadTexture3DFromEx5(const GLchar *file, std::string name)
{
	// Create Texture object
	Textures[name] = *(new Texture);

	FILE *fp = fopen(file, "r");

	if (fp == NULL)
	{
		std::cout << "Cannot open file:" << file << std::endl;
	}

	/* Read header */
	fscanf(fp, "%d", &Textures[name].Width);
	fscanf(fp, "%d", &Textures[name].Height);
	fscanf(fp, "%d", &Textures[name].Depth);

	/* Create image */
	uint8_t* data = (uint8_t*)calloc(Textures[name].Width * Textures[name].Height * Textures[name].Depth * 4, sizeof *data);

	/* Read image*/
	int i = 0;
	uint32_t pixel;
	while (fscanf(fp, "%d", &pixel) == 1)
	{
		data[i++] = pixel >> 24;
		data[i++] = pixel >> 16;
		data[i++] = pixel >> 8;
		data[i++] = pixel >> 0;
	}

	glGenTextures(1, &Textures[name].ID);
	glBindTexture(GL_TEXTURE_3D, Textures[name].ID);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, Textures[name].Width, Textures[name].Height, Textures[name].Depth, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_3D);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);

	free(data);
}

/* 1 = inside cloud, else 0 */
GLubyte structure(GLubyte pixel)
{
	if (pixel / 255.0 > 0.35)
	{
		return 1;
	}
	return 0;
}

void ResourceManager::cloud_preprocess(Texture source, const GLchar *name)
{
	Textures[name] = *(new Texture);
	/* Read the source texture */
	GLubyte* cloud_pixels = (GLubyte*)malloc(source.Width * source.Height * source.Depth * 4 * sizeof(GLubyte));
	glBindTexture(GL_TEXTURE_3D, source.ID);
	glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_UNSIGNED_BYTE, cloud_pixels);

	/* Set size of structure texture */
	Textures[name].Width = 32; /* Creates 4x4x4 cubes of original 128x128x128 texture */
	Textures[name].Height = 32;
	Textures[name].Depth = 32;

	/* Pick the red channel and procces it */
	GLubyte* temp = (GLubyte*)malloc(source.Width * source.Height * source.Depth * sizeof(GLubyte));
	for (int i = 0; i < source.Width * source.Height * source.Depth; i++)
	{
		temp[i] = structure(cloud_pixels[i * 4]);
	}

	GLubyte* new_structure = (GLubyte*)malloc(Textures[name].Width * Textures[name].Height * Textures[name].Depth * sizeof(GLubyte));

	/* Analyze every 4x4x4 block of the original texture */
	int inside = 0;
	for (int i = 0; i < Textures[name].Width; i++)
	{
		for (int j = 0; j < Textures[name].Height; j++)
		{
			for (int k = 0; k < Textures[name].Depth; k++)
			{
				inside = 0;
				for (int u = 0; u < 4; u++)
				{
					for (int v = 0; v < 4; v++)
					{
						for (int w = 0; w < 4; w++)
						{

							int x = i * 4 + u;
							int y = j * 4 + v;
							int z = k * 4 + w;

							inside += temp[x + y * source.Height + z * source.Height * source.Depth];
						}
					}
				}

				if (inside < 8)
				{
					new_structure[i + j * Textures[name].Height + k * Textures[name].Height * Textures[name].Depth] = 0;
				}
				else
				{
					new_structure[i + j * Textures[name].Height + k * Textures[name].Height * Textures[name].Depth] = 255;
				}
			}
		}
	}

	/* Post process -- expanding the structure to reduce artifacts */
	GLubyte* post_pixels = (GLubyte*)calloc(Textures[name].Width * Textures[name].Height * Textures[name].Depth, sizeof(GLubyte));

	for (int i = 0; i < Textures[name].Width; i++)
	{
		for (int j = 0; j < Textures[name].Height; j++)
		{
			for (int k = 0; k < Textures[name].Depth; k++)
			{
				if (new_structure[k + j * Textures[name].Height + i * Textures[name].Height * Textures[name].Depth] == 0)
				{
					continue;
				}

				for (int ii = -1; ii < 2; ii++)
				{
					if (i + ii < 0 || i + ii >= Textures[name].Width)
						continue;
					for (int jj = -1; jj < 2; jj++)
					{
						if (j + jj < 0 || j + jj >= Textures[name].Height)
							continue;
						for (int kk = -1; kk < 2; kk++)
						{
							if (k + kk < 0 || k + kk >= Textures[name].Depth)
								continue;

							post_pixels[(k + kk) + (j + jj) * Textures[name].Height + (i + ii) * Textures[name].Height * Textures[name].Depth] = 255;
						}
					}
				}
			}
		}
	}

	/* Bind the new Textures[name] */
	glGenTextures(1, &Textures[name].ID);
	glBindTexture(GL_TEXTURE_3D, Textures[name].ID);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, Textures[name].Width, Textures[name].Height, Textures[name].Depth, 0, GL_RED, GL_UNSIGNED_BYTE, post_pixels);
	glGenerateMipmap(GL_TEXTURE_3D);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);

	free(cloud_pixels);
	free(temp);
	free(new_structure);
	free(post_pixels);	
}

void ResourceManager::SendShaderTexture1D(Shader shader, Texture texture, const char *name)
{
	glUseProgram(shader.ID);
	GLint uni = glGetUniformLocation(shader.ID, name);
	glUniform1i(uni, texture.index);
	glActiveTexture(GL_TEXTURE0 + texture.index);
	glBindTexture(GL_TEXTURE_1D, texture.ID);
}

void ResourceManager::SendShaderTexture2D(Shader shader, Texture texture, const char *name)
{
	glUseProgram(shader.ID);
	GLint uni = glGetUniformLocation(shader.ID, name);
	glUniform1i(uni, texture.index);
	glActiveTexture(GL_TEXTURE0 + texture.index);
	glBindTexture(GL_TEXTURE_2D, texture.ID);
}

void ResourceManager::SendShaderTexture3D(Shader shader, Texture texture, const char *name)
{
	glUseProgram(shader.ID);
	GLint uni = glGetUniformLocation(shader.ID, name);
	glUniform1i(uni, texture.index);
	glActiveTexture(GL_TEXTURE0 + texture.index);
	glBindTexture(GL_TEXTURE_3D, texture.ID);
}
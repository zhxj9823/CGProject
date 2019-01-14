#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "glad/glad.h"

typedef struct Framebuffer {
	GLuint fbo;
	GLuint color_buffer[2];
} Framebuffer;

void framebuffer_scene_init(Framebuffer* f, int screen_width, int screen_height);
void framebuffer_cloud_init(Framebuffer* f, int screen_width, int screen_height);
void framebuffer_pingpong_init(Framebuffer f[2], int screen_width, int screen_height);

#endif

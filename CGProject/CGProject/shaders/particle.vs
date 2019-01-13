#version 330 core
layout (location = 0) in vec4 vertex; 

out vec2 TexCoords;
out vec4 ParticleColor;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform vec4 color;

void main()
{
    float scale = 0.1f;
    TexCoords = vertex.zw;
    ParticleColor = color;
    gl_Position = projection * view * model * vec4((scale * vertex.xy), 0.0f, 1.0f);
}

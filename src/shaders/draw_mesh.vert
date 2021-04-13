#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;        // TODO unused
layout(location = 2) in vec2 texCoord;

out vec2 fsTexCoord;

uniform mat4 MVP;

void main()
{
    fsTexCoord = texCoord;

    gl_Position = MVP * vec4(position, 1.0);
}


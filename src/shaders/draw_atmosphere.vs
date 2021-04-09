#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

out vec3 fsPosition;
out vec3 fsNormal;
out vec2 fsTexCoord;

uniform mat4 M;     // Model matrix
uniform mat4 MVP;   // Model - View - Projection matrix

void main()
{
    vec4 posVec4 = vec4(position, 1.0);
    fsPosition = vec3(M * posVec4);
	gl_Position = MVP * posVec4;

    // TODO precompute normal matrix
    mat3 N = transpose(inverse(mat3(M)));
    fsNormal = normalize(N * normal);

    fsTexCoord = texCoord;
}

#version 450 core

out vec4 FragColor;

in vec3 tex_coords;

uniform samplerCube skybox;

void main()
{    
    FragColor = texture(skybox, tex_coords);
}


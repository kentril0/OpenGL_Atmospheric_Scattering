#version 450

in vec2 fsTexCoord;
//layout(location = 0) in vec3 fs_position;
//layout(location = 1) in vec3 fs_normal;
//layout(location = 2) in vec2 fsTexCoord;
//layout(location = 3) in vec4 fs_color;

out vec4 final_color;

//uniform sampler2D tex;

void main()
{   
    //final_color = texture(tex, fsTexCoord); 
    final_color = vec4(1.0);
}


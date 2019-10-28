#version 330 core
layout (location = 0) in vec2 l_pos;

void main()
{
    gl_Position = vec4(l_pos.x, l_pos.y, 0.0, 1.0); 

}
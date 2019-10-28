#version 330 core
layout (location = 0) in vec3 l_position;

varying vec3	v_texCoords;

uniform mat4	projection;
uniform mat4	view;

void main()
{
    v_texCoords = l_position;
    gl_Position = projection * view * vec4(l_position, 1.0);
}  
#version 330 core
out vec4 fragColor;

varying vec3 v_texCoords;

uniform samplerCube skybox;

void main()
{    
    fragColor = texture(skybox, v_texCoords);
}
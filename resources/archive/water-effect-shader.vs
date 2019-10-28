#version 330 core
layout(location = 0) in vec3 _position;
layout(location = 1) in vec2 _texCoord; 

uniform mat4 model; 
uniform mat4 view; 
uniform mat4 projection; 

varying vec3 position;
varying vec2 texCoord;

void main() 
{ 
	// note that we read the multiplication from right to left 
	gl_Position = projection * view * model * vec4(_position, 1.0); 
	//FragPos = vec3(model * vec4(_position, 1.0));
	position = _position;
	texCoord = _texCoord;
}
#version 330 core
layout(location = 0) in vec3 l_position;
layout(location = 1) in vec3 l_normal;
layout(location = 2) in vec2 l_texCoord;

uniform mat4	model; 
uniform mat4	view; 
uniform mat4	projection; 
uniform bool	withOcean;
uniform float	scale;

varying vec3	v_position;
varying vec3	v_normal;
varying vec2	v_texCoord;
varying float	v_scale;
varying float	v_withOcean;

void main() 
{ 
	vec3 newPosition = l_position, newNorm = l_normal;
	newPosition.y *= 0.5;

	if (!withOcean) 
	{
		newPosition.y = 0.0;
		newNorm = vec3(0.0,1.0,0.0);
	}

	gl_Position = projection * view * model * vec4(newPosition, 1.0); 
	v_position = newPosition * model[0].x / scale + model[3].xyz / scale;
	v_normal = newNorm;
	v_texCoord = l_texCoord;
	v_scale = scale;
	v_withOcean = withOcean ? 1.0 : 0.0;
}
#version 330 core
layout(location = 0) in vec3 l_position;
layout(location = 1) in vec3 l_normal; 
layout(location = 2) in vec2 l_texCoord; 

uniform mat4	model; 
uniform mat4	view; 
uniform mat4	projection;
uniform vec3	viewPos;
uniform vec3	lightDirection;
uniform mat4	lightSpaceMatrix;
uniform bool	withOcean;
uniform float	scale;

varying vec3	v_position;
varying vec3	v_normal;
varying vec2	v_texCoord;
varying float	v_scale;
varying vec3	v_scaledPosition;
varying vec3	v_viewPos;
varying vec3	v_lightDirection;
varying vec4	v_fragPosLightSpace;
varying vec3	v_lightTBN;
varying vec3	v_viewTBN;
varying float	v_withOcean;

void main() 
{ 
	float sscale = scale;

	vec3 newPosition = l_position, newNorm = l_normal;
	newPosition.y *= 0.5;

	if (!withOcean) 
	{
		newPosition.y = 0.0;
		newNorm = vec3(0.0,1.0,0.0);
	}

	vec3 actualPosition = newPosition * model[0].x / scale + model[3].xyz / scale;

    vec3 tangent = vec3(1.0, 0.0, 0.0);
    vec3 norm = newNorm;		// TODO change these
    vec3 binormal = cross(norm, tangent);
	tangent = cross(norm, binormal);

    vec3 reverseViewDir = viewPos - actualPosition * sscale;
    v_viewTBN.x = dot(reverseViewDir, tangent);
    v_viewTBN.y = dot(reverseViewDir, binormal);
    v_viewTBN.z = dot(reverseViewDir, norm);
    vec3 reverseLight = -lightDirection;
    v_lightTBN.x = dot(reverseLight, tangent);
    v_lightTBN.y = dot(reverseLight, binormal);
    v_lightTBN.z = dot(reverseLight, norm);

	v_position = actualPosition;
	v_normal = newNorm;
	v_texCoord = l_texCoord;
	v_scale = sscale;
	v_scaledPosition = v_position * sscale;
	v_viewPos = viewPos;
	v_lightDirection = lightDirection;
	v_fragPosLightSpace = lightSpaceMatrix * model * vec4(newPosition, 1.0);
	v_withOcean = withOcean ? 1.0 : 0.0;

	gl_Position = projection * view * model * vec4(newPosition, 1.0); 
}

#version 330 core
layout(location = 0) in vec3 l_position;
layout(location = 1) in vec3 l_normal;
layout(location = 2) in vec3 l_tangent;
layout(location = 3) in vec2 l_texCoord;

uniform mat4 model; 
uniform mat4 view; 
uniform mat4 projection; 

uniform float renderMode;	// 0 = reflection, 1 = refraction, 2 = final pass

uniform vec3 viewPos;
uniform vec3 lightDirection;

uniform mat4 lightSpaceMatrix;
uniform float scale;

varying mat3	v_TBN;
varying vec3	v_normal;
varying vec3	v_position;
varying vec2	v_texCoord;
varying float	v_renderMode;
varying vec3	v_viewPos;
varying vec3	v_lightDirection;
varying vec4	v_fragPosLightSpace;
varying float	v_scale;

void main() 
{ 
	const float REFLECTION = 0.0, REFRACTION = 1.0, FINAL_PASS = 2.0;
	
	mat4 newModel = model;
	if (renderMode == REFLECTION)
	{
		mat4 refl;
		refl[0] = vec4(1.0, 0.0, 0.0, 0.0);
		refl[1] = vec4(0.0, -1.0, 0.0, 0.0);
		refl[2] = vec4(0.0, 0.0, 1.0, 0.0);
		refl[3] = vec4(0.0, 0.0, 0.0, 1.0);
		newModel = model * refl;
	}

    mat4 mvp = projection * view * newModel;
	vec3 asdf = l_position;
	gl_Position = mvp * vec4(l_position, 1.0); 
	v_position = l_position;
	v_scale = scale;

	vec3 N = mat3(transpose(inverse(model))) * l_normal;
	vec3 T = mat3(transpose(inverse(model))) * l_tangent;
	T = normalize(T - dot(T, N) * N);
	vec3 B = cross(N, T);
	v_TBN = mat3(T, B, N);
	v_normal = l_normal;

	v_texCoord = l_texCoord;
    v_renderMode = renderMode;
	v_viewPos = viewPos;
	v_lightDirection = lightDirection;
	v_fragPosLightSpace = lightSpaceMatrix * model * vec4(l_position, 1.0);
}

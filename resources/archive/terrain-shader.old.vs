#version 330 core
layout(location = 0) in vec3 _position;
layout(location = 1) in vec4 _mixMapColor; 
layout(location = 2) in vec3 _normal;
layout(location = 3) in vec3 _tangent;
layout(location = 4) in vec2 _texCoord;

uniform mat4 model; 
uniform mat4 view; 
uniform mat4 projection; 

uniform float renderMode;	// 0 = reflection, 1 = refraction, 2 = final pass

uniform vec3 viewPos;
uniform vec3 lightDirection;

varying vec4 mmColor;
varying mat3 TBN;
varying vec3 normal;
varying vec3 position;
varying vec2 texCoord;
varying float render_mode;
varying vec3 view_pos;
varying vec3 light_direction;

varying vec4 waterTex0;
varying vec4 waterTex3;
varying vec4 waterTex4;

void main() 
{ 
	mat4 newModel = model;
	if (renderMode == 0.0)
	{
		mat4 refl;
		refl[0] = vec4(1.0, 0.0, 0.0, 0.0);
		refl[1] = vec4(0.0, -1.0, 0.0, 0.0);
		refl[2] = vec4(0.0, 0.0, 1.0, 0.0);
		refl[3] = vec4(0.0, 0.0, 0.0, 1.0);
		newModel = model * refl;
	}
	// note that we read the multiplication from right to left 
	vec3 newPos = _position;
	if (renderMode == 2.0 && newPos.y < 0) newPos.y = 0;
    mat4 mvp = projection * view * newModel;
	gl_Position = mvp * vec4(newPos, 1.0); 
	//FragPos = vec3(model * vec4(_position, 1.0));
	position = _position;
	mmColor = _mixMapColor;

	vec3 N = mat3(transpose(inverse(model))) * _normal;
	vec3 T = mat3(transpose(inverse(model))) * _tangent;
	T = normalize(T - dot(T, N) * N);
	vec3 B = cross(N, T);
	TBN = mat3(T, B, N);
	normal = _normal;

	if (renderMode == 2.0)
	{
        vec3 mpos, temp;
        vec3 tangent = vec3(1.0, 0.0, 0.0);
        vec3 norm = vec3(0.0, 1.0, 0.0);
        vec3 binormal = vec3(0.0, 0.0, 1.0);
        temp = viewPos - newPos;
        waterTex4.x = dot(temp, tangent);
        waterTex4.y = dot(temp, binormal);
        waterTex4.z = dot(temp, norm);
        temp = -lightDirection;
        waterTex0.x = dot(temp, tangent);
        waterTex0.y = dot(temp, binormal);
        waterTex0.z = dot(temp, norm);
        waterTex3 = mvp * vec4(position, 1.0);
    }


	texCoord = _texCoord;
    render_mode = renderMode;
	view_pos = viewPos;
	light_direction = lightDirection;
}

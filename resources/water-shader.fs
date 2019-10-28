#version 330 core												 
out vec4 fragColor;

varying vec3		v_position;
varying vec3		v_normal;
varying vec2		v_texCoord;
varying float		v_scale;
varying float		v_withOcean;

uniform vec3		viewPos;
uniform vec4		fogColor;
uniform float		viewDistance;
uniform samplerCube	skybox;
uniform vec2		windowSize;
uniform float		time;
uniform sampler2D	water_normalmap;
uniform sampler2D	water_dudvmap;
											 
void main()													 
{
	float linearDistance = length(v_position.xz - viewPos.xz);
	linearDistance = abs(linearDistance);
	
	float fogFactor = max(min((viewDistance - linearDistance) / (viewDistance - viewDistance / 5), 1), 0);


	vec3 microfacet = normalize(v_normal);

	if (v_withOcean == 1.0)
	{
		// ------------ Calculate texture displacement to simulate water ripples --------------- //
		const float waterScale = 8.0;
		vec3 v1 = texture(water_normalmap, (v_texCoord + vec2(0.0,-time / 1.2)) * waterScale).gbr;
		vec3 v2 = texture(water_normalmap, (v_texCoord + vec2(0.0,time)) * waterScale).gbr;
		vec3 v3 = texture(water_normalmap, (v_texCoord + vec2(-time / 1.5, 0.0)) * waterScale).gbr;
		vec3 v4 = texture(water_normalmap, (v_texCoord + vec2(time / 1.1, 0.0)) * waterScale).gbr;
		v1 = (v1-vec3(0.5)) * (2.0);
		v2 = (v2-vec3(0.5)) * (2.0);
		v3 = (v3-vec3(0.5)) * (2.0);
		v4 = (v4-vec3(0.5)) * (2.0);
		if (v1.y < 0.0) v1 = -v1;
		if (v2.y < 0.0) v2 = -v2;
		if (v3.y < 0.0) v3 = -v3;
		if (v4.y < 0.0) v4 = -v4;
		vec3 vNorm = normalize(v1 + v2 + v3 + v4);
		// ---------------------------------------------------------------------------------------//

		microfacet = normalize(microfacet + vNorm);
	}
	
	vec3 I = normalize(v_position - viewPos);
	vec3 R = reflect(I, microfacet);
	vec4 color = vec4(texture(skybox, R).rgb, 1.0);
	fragColor = mix(fogColor, color, fogFactor);
} 																 
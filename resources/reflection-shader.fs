#version 330 core
out vec4 fragColor;

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

uniform vec3		lightColor;
uniform vec4		fogColor;
uniform float		viewDistance;
uniform sampler2D	refractionTex;
uniform sampler2D	depthTex;
uniform sampler2D	reflectionTex;
uniform vec2		windowSize;
uniform float		time;
uniform sampler2D	water_normalmap;
uniform sampler2D	water_dudvmap;
uniform sampler2D	shadowMap;
uniform bool		withoutReflection;
uniform bool		withoutRefraction;
uniform bool		withoutShadow;

float snoise(vec3 v);

void main() 
{
	vec3 waterHue = vec3(0, 0.188, 0.20);
	//vec3 waterHue = vec3(0.47, 0.71, 0.87);
	//vec3 waterHue = vec3(0.40, 0.44, 0.54);
	
	//vec3 waterHue = vec3(0.05);

	vec3 vNorm = vec3(0.0,0.0,1.0);
	vec2 fragTexCoord = gl_FragCoord.xy/windowSize;


	if (!withoutReflection || !withoutRefraction)
	{
		// ------------ Calculate texture displacement to simulate water ripples --------------- //
		const float waterScale = 60.0, amplitude = 0.02;
		vec2 texCoord1 = v_texCoord + vec2(0.0, -time/5), texCoord2 = v_texCoord + vec2(0.0, time/5);
		vec3 disdis = texture(water_dudvmap, (texCoord2*waterScale) * 0.25).rgb;
		vec3 fdist = texture(water_dudvmap, (texCoord1*waterScale) + vec2(disdis*amplitude)).rgb;
		fdist = normalize(fdist);
		fdist *= 0.005;
		vec3 nmap = texture(water_normalmap, texCoord1*waterScale + vec2(disdis*amplitude)).rgb;
		nmap = (nmap-vec3(0.5)) * (2.0);
		vNorm = normalize(nmap);
		// ---------------------------------------------------------------------------------------//


		vec2 waterTexCoord = fragTexCoord + vec2(fdist);
		waterTexCoord = clamp(waterTexCoord,0.001,0.999);

		vec3 refl, refr;

		vec3 invfres, fres;
		invfres = vec3( dot(vNorm, normalize(v_viewTBN)) );
		fres = vec3(1.0) -invfres;

		vec3 invdepth;
		if (!withoutReflection) 
		{
			refl = texture(reflectionTex, waterTexCoord).rgb;
			vec3 source = texture(reflectionTex, fragTexCoord).rgb;
			vec3 difference = source - refl;
			difference = abs(difference);
			if (refl == vec3(1.0)) refl = source;
			refl = mix(refl, vec3(0.0), 0.5);
			refl *= fres;
		}
		if (!withoutRefraction) 
		{
			refr = texture(refractionTex, waterTexCoord).rgb;
			vec3 source = texture(refractionTex, fragTexCoord).rgb;
			vec3 difference = source - refr;
			difference = abs(difference);
			if (refr == vec3(1.0)) refr = source;
			float zNear = 0.1;
			float zFar  = 500.0;
			float depth = texture(depthTex, gl_FragCoord.xy / windowSize).r;
			float z_e = (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));
			vec3 wdepth = vec3(max(0.0,min(1.0,z_e)));
			wdepth = vec3(pow(wdepth.x, 4.0));
			invdepth = 1.0 - wdepth;

			refr *= invfres;
			refr *= invdepth;
		}

		waterHue = refr + refl;
	}
  
	float shadow = 0.0;
	const float shadowMax = 0.8;


	if (!withoutShadow)
	{
		vec3 projCoords = v_fragPosLightSpace.xyz / v_fragPosLightSpace.w;
		projCoords = projCoords * 0.5 + 0.5;
		float closestDepth = texture(shadowMap, projCoords.xy).r;
		float currentDepth = projCoords.z;
		float bias = 0.0001;
		vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
		for(int x = -2; x <= 2; ++x)
			for(int y = -2; y <= 2; ++y)
			{
				float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
				shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
			}

		if(projCoords.z > 1.0)
			shadow = 0.0;
	}

	vec3 res;
	
	if (v_withOcean == 1.0)
	{
		const float wwaterScale = 8.0;
		vec3 v1 = texture(water_normalmap, (v_texCoord + vec2(0.0,-time / 1.2)) * wwaterScale).gbr;
		vec3 v2 = texture(water_normalmap, (v_texCoord + vec2(0.0,time)) * wwaterScale).gbr;
		vec3 v3 = texture(water_normalmap, (v_texCoord + vec2(-time / 1.5, 0.0)) * wwaterScale).gbr;
		vec3 v4 = texture(water_normalmap, (v_texCoord + vec2(time / 1.1, 0.0)) * wwaterScale).gbr;
		v1 = (v1-vec3(0.5)) * (2.0);
		v2 = (v2-vec3(0.5)) * (2.0);
		v3 = (v3-vec3(0.5)) * (2.0);
		v4 = (v4-vec3(0.5)) * (2.0);
		if (v1.y < 0.0) v1 = -v1;
		if (v2.y < 0.0) v2 = -v2;
		if (v3.y < 0.0) v3 = -v3;
		if (v4.y < 0.0) v4 = -v4;
		vec3 vvNorm = normalize(v1 + v2 + v3 + v4);
		vvNorm = normalize(vvNorm);

		// ------------------ Calculate lighting ------------------------- //
		vec3 lighting;
		float ambientStrength = 0.1;
		vec3 lightDir = normalize(-v_lightDirection);
		vec3 source = texture(reflectionTex, fragTexCoord).rgb;
		vec3 ambient = ambientStrength * source;
		vec3 viewDir = normalize(v_viewPos - v_position);
		vec3 microfacet = normalize(vvNorm + normalize(v_normal));
		//microfacet /= 3;
		//microfacet = normalize(microfacet);
		float diff =  max(dot(microfacet, lightDir), 0.0);
		vec3 diffuse = diff * lightColor * source;

		vec3 closerViewDir = viewDir;
		closerViewDir.y *= 2;
		closerViewDir = normalize(closerViewDir);
		float invfres, fres;
		invfres = ( min(1.0,pow(dot(microfacet, closerViewDir) * 1.0, 1)) );
		//invfres = clamp(0.0,0.9,invfres);
		fres = (1.0) -invfres;
		fres = pow(fres, 1.0);
		if (fres < 0.05) fres = 0.05;/*
		if (fres > 0.95) fres = 0.95;*/
		invfres = (1.0) - fres;

		vec3 reflectDir = reflect(-lightDir, microfacet);
		vec3 flatReflDir = reflect(-lightDir, vec3(0.0,1.0,0.0));
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), 512);
		float flatSpec = pow(max(dot(viewDir, flatReflDir), 0.0), 512);
		spec *= flatSpec;
		if (shadow != 0.0) spec = 0.0;
		// --------------------------------------------------------------- //

		vec3 waterCol = vec3(0.04, 0.16, 0.17) * 0.3;
		res =  mix(waterCol, (waterCol * invfres + source * diff * fres) * max(min(1.0, 1.0 - shadow), 0.6), 1.0) + spec/* + (waterCol)*/;
	}
	else
	{
		shadow = max(shadowMax, 1.0 - shadow);
		res = shadow * waterHue;

		// ------------------ Calculate specular ------------------------- //
		vec3 vRef = normalize(reflect(-normalize(v_lightTBN), vNorm));
		float stemp =max(0, dot(normalize(v_viewTBN), vRef) );
		float spec = pow(stemp, 128);
		if (shadow == shadowMax) spec = 0.0;
		// --------------------------------------------------------------- //

		res = res + spec;
	}
	

	// ----------------- Calculate fog ------------------------------- //
	float linearDistance = length(v_scaledPosition.xz - v_viewPos.xz * v_scale);
	linearDistance = abs(linearDistance);
	float fogFactor = max(min((viewDistance - linearDistance) / (viewDistance - viewDistance / 5), 1), 0);
	fragColor = vec4(mix(fogColor.xyz, res, fogFactor), v_withOcean == 1.0 ? 1.0 : 1.0);
}

												

//
// Description : Array and textureless GLSL 2D/3D/4D simplex 
//               noise functions.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : stegu
//     Lastmod : 20110822 (ijm)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
//               https://github.com/stegu/webgl-noise
// 

vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x) {
     return mod289(((x*34.0)+1.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v)
  { 
  const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
  const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

// First corner
  vec3 i  = floor(v + dot(v, C.yyy) );
  vec3 x0 =   v - i + dot(i, C.xxx) ;

// Other corners
  vec3 g = step(x0.yzx, x0.xyz);
  vec3 l = 1.0 - g;
  vec3 i1 = min( g.xyz, l.zxy );
  vec3 i2 = max( g.xyz, l.zxy );

  //   x0 = x0 - 0.0 + 0.0 * C.xxx;
  //   x1 = x0 - i1  + 1.0 * C.xxx;
  //   x2 = x0 - i2  + 2.0 * C.xxx;
  //   x3 = x0 - 1.0 + 3.0 * C.xxx;
  vec3 x1 = x0 - i1 + C.xxx;
  vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
  vec3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y

// Permutations
  i = mod289(i); 
  vec4 p = permute( permute( permute( 
             i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
           + i.y + vec4(0.0, i1.y, i2.y, 1.0 )) 
           + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

// Gradients: 7x7 points over a square, mapped onto an octahedron.
// The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
  float n_ = 0.142857142857; // 1.0/7.0
  vec3  ns = n_ * D.wyz - D.xzx;

  vec4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,7*7)

  vec4 x_ = floor(j * ns.z);
  vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

  vec4 x = x_ *ns.x + ns.yyyy;
  vec4 y = y_ *ns.x + ns.yyyy;
  vec4 h = 1.0 - abs(x) - abs(y);

  vec4 b0 = vec4( x.xy, y.xy );
  vec4 b1 = vec4( x.zw, y.zw );

  //vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
  //vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
  vec4 s0 = floor(b0)*2.0 + 1.0;
  vec4 s1 = floor(b1)*2.0 + 1.0;
  vec4 sh = -step(h, vec4(0.0));

  vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
  vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

  vec3 p0 = vec3(a0.xy,h.x);
  vec3 p1 = vec3(a0.zw,h.y);
  vec3 p2 = vec3(a1.xy,h.z);
  vec3 p3 = vec3(a1.zw,h.w);

//Normalise gradients
  vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;

// Mix final noise value
  vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
  m = m * m;
  return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1), 
                                dot(p2,x2), dot(p3,x3) ) );
  }	
#version 330 core												 
out vec4 fragColor;		

varying mat3	v_TBN;
varying vec3	v_normal;
varying vec3	v_position;
varying vec2	v_texCoord;
varying float	v_renderMode;
varying vec3	v_viewPos;
varying vec3	v_lightDirection;
varying vec4	v_fragPosLightSpace;
varying float	v_scale;

uniform vec3		lightColor;

uniform vec4		fogColor;
uniform float		viewDistance;

uniform sampler2D	sand;
uniform sampler2D	grass;
uniform sampler2D	limeStone;
uniform sampler2D	darkStone;
uniform sampler2D	snow;

uniform sampler2D	sandNormalMap;
uniform sampler2D	rocksNormalMap;
uniform sampler2D	stoneNormalMap;
uniform sampler2D	raggedStoneNormalMap;
uniform float		texScale;

uniform sampler2D	shadowMap;

uniform bool		withoutShadow;

float snoise(vec3 v);
											 
void main()													 
{
	const float REFLECTION = 0.0, REFRACTION = 1.0, FINAL_PASS = 2.0;
    const float PI = 3.1415926535;

	vec3 lakeColorLight = vec3(0.098, 0.286, 0.298);
    vec3 lakeColorDark = vec3(0, 0.188, 0.20);
	float normalScale = 6.0;

	//float foam = 1-max(0.0, min(1.0, abs(v_position.y)/1.0));
	//float foamShift = pow((snoise(vec3(v_position.xz/10, time * 50))+1)/2 ,1)* foam;

	if (v_renderMode == REFLECTION && v_position.y <= -1.0) discard;
	else if (v_renderMode == REFRACTION && v_position.y > 1.0) discard;

    // refer to https://stackoverflow.com/questions/1110844/multiple-texture-images-blended-together-onto-3d-ground?rq=1
	
	vec3 texSand = texture(sand, v_texCoord * texScale).rgb;
    vec3 texGrass = texture(grass, v_texCoord * texScale).rgb;
    vec3 texDarkStone = texture(darkStone, v_texCoord * texScale).rgb;
    vec3 texLimeStone = texture(limeStone, v_texCoord * texScale).rgb;
    vec3 texSnow = texture(snow, v_texCoord * texScale).rgb;

    vec3 color = texSand;

    float snowLine = snoise(v_position * vec3(0.01,0.01,0.01)) * 100.0 + 500.0;
    float treeLine = snoise(v_position * vec3(0.006,0.006,0.006)) * 50.0 + 300.0;
    float beachLine = snoise(v_position * vec3(0.01,0.01,0.01)) * 5.0;

    float theta = atan(v_normal.y/sqrt(pow(v_normal.x, 2) + pow(v_normal.z, 2)));
	theta = abs(theta);
    bool dirt = false;
    bool snow = true;
    float n = snoise(v_position * vec3(0.01,0.01,0.01));
    n = (n + 1.0)/2.0;

    vec3 norm = texture(sandNormalMap, v_texCoord * normalScale).rgb;
    norm = normalize(norm * 2.0- 1.0);
    norm = normalize(v_TBN * norm);

    if (v_position.y > beachLine) 
	{
        vec3 thisNorm = v_normal;

		// flat ground
        if (theta > PI / 7) 
		{
			// chance of bumpy ground
            if (n > 0.3)
			{
                thisNorm = texture(rocksNormalMap, v_texCoord * normalScale).rgb;
                thisNorm = normalize(thisNorm * 2.0- 1.0);
                thisNorm = normalize(v_TBN * thisNorm);
                thisNorm = mix(v_normal, thisNorm, (n-0.3)/0.7);
                dirt = true;

				// transition between bumpy ground and grass based on steepness
				// flatter ground has sharper bumps
				if (theta < PI / 4) 
					thisNorm = mix(thisNorm, v_normal, min(0.3, 1-(theta - PI / 7) / (PI/4-PI/7)));
            }

        }
		// sharp cliff
		else 
		{
            thisNorm = texture(raggedStoneNormalMap, v_texCoord * normalScale).rgb;
            thisNorm = normalize(thisNorm * 2.0- 1.0);
            thisNorm = normalize(v_TBN * thisNorm);
        }

		// transition between beach and grass
		norm = mix(norm, thisNorm, min((v_position.y - beachLine)/10.0, 1.0));
    }

	if (v_position.y > treeLine) 
	{
        vec3 thisNorm = texture(stoneNormalMap, v_texCoord * normalScale).rgb;
        thisNorm = normalize(thisNorm * 2.0- 1.0);
        thisNorm = normalize(v_TBN * thisNorm);
        norm = mix(norm, thisNorm, min((v_position.y - treeLine)/100.0, 1.0));
    }

	if (v_position.y > snowLine) 
	{
		// flat ground
        if (theta > PI / 4) 
		{
			// chance of rocks
            if (n > 0.5)
			{
                norm = texture(rocksNormalMap, v_texCoord * normalScale).rgb;
                norm = normalize(norm * 2.0- 1.0);
                norm = normalize(v_TBN * norm);
                norm = mix(v_normal, norm, (n - 0.5)/0.5);
            }

		}
		// rock face
		else
		{
            norm = texture(stoneNormalMap, v_texCoord * normalScale).rgb;
            norm = normalize(norm * 2.0- 1.0);
            norm = normalize(v_TBN * norm);
            snow = false;
        }
	}
	
	theta = atan(norm.y/sqrt(pow(norm.x, 2) + pow(norm.z, 2)));
    if (v_position.y > beachLine) 
	{
		// flat ground
        if (theta > PI / 7) 
		{
			// mix patches of pale and deep green
			vec3 savannah = vec3(0.733333333,0.631372549,0.31372549);
            vec3 forest = vec3(0.121568627,0.239215686,0.0470588235);

            float n2 = snoise(v_position * vec3(0.001,0.001,0.001));

			// sea level tends to be savannah, tree line tends to be forest
            float heightBias = (v_position.y - beachLine - 10)/(treeLine - beachLine - 10) * 2.0 - 1.0;
            float blend = 0.2;
            float n3 = (min(blend, max(-blend, n2 + heightBias)) + blend)/(blend*2);

            vec3 grassColor = mix(savannah, forest, n3);
            grassColor = mix(texGrass, grassColor, 0.5);

			// give slopes browner grass
            if (theta < PI / 4) color = mix(grassColor, vec3(0.51, 0.33, 0.06), min(0.3, 1-(theta - PI / 7) / (PI/4-PI/7)));
            else color = grassColor;
        }

		else 
		{
            if (!dirt) color = texLimeStone;
            else color = vec3(0.4863, 0.3686, 0.2558);
        }

		color = mix(texSand, color, min((v_position.y - beachLine)/10.0, 1.0));
    }

	if (v_position.y > treeLine) 
	{
        color = mix(color, texDarkStone, min((v_position.y - treeLine)/100.0, 1.0));
    }

	if (v_position.y > snowLine) 
	{
        if (theta > PI / 4) 
		{
            if (snow) color = texSnow;
            else color = texDarkStone;
        }

		else color = texDarkStone;
	}
	//color = vec3(0.5,0.5,0.5);
	//norm = v_normal;
	
	// ------------------ Calculate lighting ------------------------- //
	vec3 lighting;
	float specularStrength = 0.0;
    float ambientStrength = 0.35;
    float shininess = 0.0;
    vec3 lightDir = normalize(-v_lightDirection);
    vec3 ambient = ambientStrength * color;
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * color;
    vec3 viewDir = normalize(v_viewPos - v_position);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * color;
	lighting = ambient + diffuse + specular;
	// --------------------------------------------------------------- //

	float shadow = 0.0;

	if (!withoutShadow)
	{
		vec3 projCoords = v_fragPosLightSpace.xyz / v_fragPosLightSpace.w;
		projCoords = projCoords * 0.5 + 0.5;
		float closestDepth = texture(shadowMap, projCoords.xy).r; 
		float currentDepth = projCoords.z;
		float bias = max(0.001 * (1.0 - dot(norm, lightDir)), 0.0001);  
		vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
		for(int x = -2; x <= 2; ++x)
			for(int y = -2; y <= 2; ++y)
			{
				float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
				shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
			}
		shadow /= 25.0;
		if(projCoords.z > 1.0)
			shadow = 0.0;
	}
	
	lighting += min(0.15, 1.0 - shadow);
    vec3 result = lighting * color;

    float margin1 = 10.0;
    float margin2 = 20.0;
    float startTransparency = 1.0;

	// ------------- Add water tint for deeper levels --------------- //
    if (v_renderMode == REFRACTION || v_renderMode == FINAL_PASS) 
	{
        vec3 res = mix(lakeColorLight, lakeColorDark, min(1, max(0, -(v_position.y + margin1)/margin2)));
        result = mix(res,result,min(1, max(0, (-startTransparency/margin1)*-v_position.y+startTransparency)));
    }
	// ------------------------------------------------------------- //

	// ----------------- Calculate fog ------------------------------- //
	float linearDistance = length(v_position.xz * v_scale - v_viewPos.xz * v_scale);
	linearDistance = abs(linearDistance);
	float trueFogFactor = max(min((viewDistance - linearDistance) / (viewDistance - viewDistance / 5), 1), 0);
    fragColor = mix(fogColor, vec4(result, 1.0), trueFogFactor);;
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
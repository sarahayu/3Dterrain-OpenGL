#version 330 core												 
out vec4 fragColor;		

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

uniform vec3 lightColor;


uniform vec4 fogColor;
uniform float viewDistance;
uniform vec4 waterColor;

uniform sampler2D sand;
uniform sampler2D grass;
uniform sampler2D limeStone;
uniform sampler2D darkStone;
uniform sampler2D snow;

uniform sampler2D sandNormalMap;
uniform sampler2D rocksNormalMap;
uniform sampler2D stoneNormalMap;
uniform sampler2D raggedStoneNormalMap;
uniform float texScale;

uniform sampler2D refractionTex;
uniform sampler2D depthTex;
uniform sampler2D reflectionTex;
uniform vec2 windowSize;
uniform float time;

uniform sampler2D water_normalmap;
uniform sampler2D water_dudvmap;

float rand(vec2 co)
{
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

float snoise(vec3 v);
											 
void main()													 
{
	float linearDistance = length(vec2(position.x, position.z)-vec2(view_pos.x, view_pos.z));
	linearDistance = abs(linearDistance);		
	float foam = 1-max(0.0, min(1.0, abs(position.y)/1.0));
	float foamShift = pow((snoise(vec3(position.xz/10, time * 50))+1)/2 ,1)* foam;

	vec3 newPosition = position;
	if (render_mode == 0.0 && position.y <= 0.0) discard;
	else if (render_mode == 1.0 && position.y > 5.0) discard;
	else if (render_mode == 2.0 && position.y <= 0.0) 
	{
		float waterScale = 60.0;
        vec2 waterTex1 = texCoord + vec2(0.0, -time/5);
        vec2 waterTex2 = texCoord + vec2(0.0, time/5);


     const vec4 sca = vec4(0.005, 0.005, 0.005, 0.005);
        const vec4 sca2 = vec4(0.02, 0.02, 0.02, 0.02);
        const vec4 tscale = vec4(0.25, 0.25, 0.25, 0.25);
        const vec4 two = vec4(2.0, 2.0, 2.0, 1.0);
        const vec4 mone = vec4(-1.0, -1.0, -1.0, 1.0);
        const vec4 ofive = vec4(0.5,0.5,0.5,1.0);
        const float exponent = 1.0;
        vec3 lightTS = normalize(vec3(waterTex0));
        vec3 viewt = normalize(vec3(waterTex4));
        vec3 disdis = texture(water_dudvmap, (waterTex2*waterScale) * vec2(tscale)).rgb;
        vec3 dist = texture(water_dudvmap, (waterTex1*waterScale) + vec2(disdis*vec3(sca2))).rgb;
        vec3 fdist = dist;
        fdist = fdist * vec3(two + mone);
        fdist = normalize(fdist);
        fdist *= vec3(sca);
        //load normalmap

     vec3 nmap = texture(water_normalmap, waterTex1*waterScale + vec2(disdis*vec3(sca2))).rgb;
		vec3 nmap2 = nmap;
		//nmap.z *= 0.1;
		//nmap = normalize(nmap);
		//nmap.zxy = nmap.xyz;
        nmap = (nmap-vec3(ofive)) * vec3(two);
        nmap2 = (nmap2-vec3(ofive)) * vec3(two);
        vec3 vNorm = nmap;
        vNorm = normalize(nmap);
		vec3 vNorm2 = normalize(nmap2);
		//vec3 vNorm2 = vNorm;
        //get projective texcoords

     vec3 tmp = vec3(1.0 / waterTex3.w);
        vec3 temp = tmp;
        vec3 projCoord = vec3(gl_FragCoord.xy/windowSize, 0.0);
		//vec3 projCoord = vec3(waterTex3) * tmp;
        //projCoord += vec3(1.0);
        //projCoord *= vec3(0.5);
        tmp = projCoord + fdist;
		tmp = clamp(tmp,0.001,0.999);
        //tmp = clamp(tmp, 0.001, 0.999);
        //load reflection,refraction and depth texture

     vec3 refTex = texture(reflectionTex, vec2(tmp)).rgb;
        vec3 refl = refTex;
        vec3 refr = texture(refractionTex, vec2(tmp)).rgb;
		vec3 a = refl;
		//if (refr == vec3(1.0,1.0,1.0)) refr = texture(refractionTex, gl_FragCoord.xy/windowSize).rgb;
		float zNear = 0.1;    // TODO: Replace by the zNear of your perspective projection
		float zFar  = 500.0; // TODO: Replace by the zFar  of your perspective projection
		float depth = texture(depthTex, gl_FragCoord.xy / windowSize).r;
		float z_e = (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));
        vec3 wdepth = vec3(max(0.0,min(1.0,z_e)));
        wdepth = vec3(pow(wdepth.x, 4.0));
        vec3 invdepth = 1.0 - wdepth;
        //calculate specular highlight

     vec3 vRef = normalize(reflect(-lightTS, vNorm));
        float stemp =max(0, dot(viewt, vRef) );
        float spec1 = pow(stemp, 1);
        float spec2 = pow(stemp, 64);
        float spec3 = pow(stemp, 128);
        //vec3 specular = vec3(stemp);
        //calculate fresnel and inverted fresnel  

     vec3 invfres = vec3( dot(vNorm2, viewt) );
        vec3 fres = vec3(1.0) -invfres ;
        //calculate reflection and refraction

     refr *= invfres;
        refr *= invdepth;
     //   temp = vec3(waterColor) * wdepth * invfres;
     //   refr += temp;
		refl = mix(refl, vec3(0.0), 0.9); 
        refl *= fres;

        //add reflection and refraction

     tmp = refr + refl;
        //vec3 res = mix(tmp, vec3(waterColor), waterColor.w) + specular;
	if (abs(position.y) <= 1) tmp = mix(tmp, vec3(1.0), foamShift);
        vec3 res = tmp + vec3((spec1 + spec2 * 2 + spec3 * 2)/5);
        //vec3 res = mix(refl, vec3(waterColor), waterColor.w) + specular * 0.5/* * 0.4*/;
		//fragColor = vec4(texture(water_dudvmap, gl_FragCoord.xy / windowSize * waterScale).rgb, 1.0);
  //      vec3 color = texture(refractionTex, (gl_FragCoord.xy-vec2(0.5,0.5)) / windowSize).rgb;
		//float alpha = 1.0;
		//if (color == vec3(1.0,1.0,1.0))
		//{
		//	alpha = 0.0;
		//	color = vec3(1.0,0.0,0.0);
		//}
		//fragColor = vec4(color, alpha);
	
	//float fogFactor = max( min(linearDistance/ (viewDistance),1), 0.05);
	float trueFogFactor = max( min((viewDistance-(linearDistance ))/ (viewDistance - viewDistance/5),1), 0);
	//fragColor = mix(vec4(1.0,1.0,1.0,1.0),fogColor,0.0);
	fragColor = mix(fogColor,vec4(res, 1.0),trueFogFactor);
		return;
	}
	//if (position.y < 0.0) discard;

	float normalScale = 6.0;
	// refer to https://stackoverflow.com/questions/1110844/multiple-texture-images-blended-together-onto-3d-ground?rq=1
	//vec3 texSand = vec3(0.761, 0.698, 0.502);
	//vec3 texGrass = vec3(0.043, 0.4, 0.137);
	//vec3 texLimeStone = vec3(0.4863, 0.3686, 0.2558);
	//vec3 texSnow = vec3(1.0, 1.0, 1.0);
	//vec3 texDarkStone = vec3(0.208, 0.282, 0.369);

	
	vec3 texSand = texture(sand, texCoord * texScale).rgb;
	vec3 texGrass = texture(grass, texCoord * texScale).rgb;
	vec3 texDarkStone = texture(darkStone, texCoord * texScale).rgb;
	vec3 texLimeStone = texture(limeStone, texCoord * texScale).rgb;
	vec3 texSnow = texture(snow, texCoord * texScale).rgb;

	//texSand *= mmColor.r;
	//texGrass = mix(texSand, texGrass, mmColor.g);
	//texStone = mix(texGrass, texStone, mmColor.b);
	//vec3 color = mix(texStone, texSnow, mmColor.a);
	//vec3 color = vec3(1.0,1.0,1.0);

	vec3 color = texSand;
	
	float snowLine = snoise(newPosition * vec3(0.01,0.01,0.01)) * 100.0 + 500.0;
	float treeLine = snoise(newPosition * vec3(0.006,0.006,0.006)) * 50.0 + 300.0;
	float beachLine = snoise(newPosition * vec3(0.01,0.01,0.01)) * 5.0;


	int normCount = 1;

	float PI = 3.1415926535;
	float theta = atan(normal.y/sqrt(pow(normal.x, 2) + pow(normal.z, 2)));
	
	bool dirt = false;
	bool snow = true;

	float n = snoise(newPosition * vec3(0.01,0.01,0.01));	
	n = (n + 1.0)/2.0;
	vec3 norm = texture(sandNormalMap, texCoord * normalScale).rgb;
	norm = normalize(norm * 2.0- 1.0);
	norm = normalize(TBN * norm);

	//if (n > 0.5)
	//{
	//	norm = normalize(norm * 2.0- 1.0);
	//	norm.z *= 1-(n - 0.5)/0.5;
	//	norm = normalize(TBN * norm);
	//}

	if (newPosition.y > beachLine) 
	{
		vec3 thisNorm = normal;
		if (theta > PI / 7) 
		{
			vec3 rocksNorm = texture(rocksNormalMap, texCoord * normalScale).rgb;
			rocksNorm = normalize(rocksNorm * 2.0- 1.0);
			//rocksNorm.z *= 0.5;
			rocksNorm = normalize(TBN * rocksNorm);
			if (n > 0.3)
			{
				thisNorm = texture(rocksNormalMap, texCoord * normalScale).rgb;
				thisNorm = normalize(thisNorm * 2.0- 1.0);
				thisNorm.z *= 1.0;
				thisNorm = normalize(TBN * thisNorm);
				thisNorm = mix(normal, thisNorm, (n-0.3)/0.7);
				dirt = true;
			}
			if (theta < PI / 4) thisNorm = mix(thisNorm, normal, min(0.3, 1-(theta - PI / 7) / (PI/4-PI/7)));

		}
		else 
		{
			thisNorm = texture(raggedStoneNormalMap, texCoord * normalScale).rgb;
			thisNorm = normalize(thisNorm * 2.0- 1.0);
			thisNorm = normalize(TBN * thisNorm);
		}

		//norm = thisNorm;
		norm = mix(norm, thisNorm, min((newPosition.y - beachLine)/10.0, 1.0));
	}
	if (newPosition.y > treeLine) 
	{
		vec3 thisNorm = texture(stoneNormalMap, texCoord * normalScale).rgb;
		thisNorm = normalize(thisNorm * 2.0- 1.0);
		thisNorm = normalize(TBN * thisNorm);
		norm = mix(norm, thisNorm, min((newPosition.y - treeLine)/100.0, 1.0));
	}
	if (newPosition.y > snowLine) 
	{
		if (theta > PI / 4) 
		{
			if (n > 0.5)
			{
				norm = texture(rocksNormalMap, texCoord * normalScale).rgb;
				norm = normalize(norm * 2.0- 1.0);
				norm = normalize(TBN * norm);
				norm = mix(normal, norm, (n - 0.5)/0.5);
			}
		}
		else
		{
			norm = texture(stoneNormalMap, texCoord * normalScale).rgb;
			norm = normalize(norm * 2.0- 1.0);
			norm = normalize(TBN * norm);
			snow = false;
		}
	
		//color = mix(texStone, color, min((position.y - snowLine)/100.0, 1.0));
	}
	
	theta = atan(norm.y/sqrt(pow(norm.x, 2) + pow(norm.z, 2)));
	if (newPosition.y > beachLine) 
	{
		if (theta > PI / 7) 
		{
			
			float n2 = snoise(newPosition * vec3(0.001,0.001,0.001));	
			//n2 = (n2 + 1.0)/2.0;
			vec3 savannah = vec3(0.733333333,0.631372549,0.31372549);
			vec3 forest = vec3(0.121568627,0.239215686,0.0470588235);

			float heightBias = ((newPosition.y - beachLine - 10) - (treeLine - beachLine - 10)/2)/(treeLine - beachLine - 10);

			float blend = 0.2;
			float n3 = (min(blend, max(-blend, n2 + heightBias)) + blend)/(blend*2);

			vec3 grassColor = mix(savannah, forest, n3);
			grassColor = (texGrass + grassColor)/2;

			if (theta < PI / 4) color = mix(grassColor, vec3(0.51, 0.33, 0.06), min(0.3, 1-(theta - PI / 7) / (PI/4-PI/7)));
			else color = grassColor;

		}
		else 
		{
			if (!dirt) color = texLimeStone;
			else color = vec3(0.4863, 0.3686, 0.2558);
		}


		color = mix(texSand, color, min((newPosition.y - beachLine)/10.0, 1.0));
	}
	if (newPosition.y > treeLine) 
	{
		color = mix(color, texDarkStone, min((newPosition.y - treeLine)/100.0, 1.0));
	}
	if (newPosition.y > snowLine) 
	{
		if (theta > PI / 4) 
		{
			if (true) color = texSnow;
			else color = texDarkStone;
		}
		else color = texDarkStone;

		//color = mix(texStone, color, min((position.y - snowLine)/100.0, 1.0));
	}
	//color = vec3(0.5,0.5,0.5);
	//norm = normal;
	
	float specularStrength = 0.0;
    float ambientStrength = 0.6;
	float shininess = 0.0;

	vec3 lightDir = normalize(-light_direction);

    vec3 ambient = ambientStrength * color;

	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor * color;

	vec3 viewDir = normalize(view_pos - position);
	vec3 reflectDir = reflect(-lightDir, norm);

	float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
	vec3 specular = specularStrength * spec * color;
	
	float e = 1.569034853;
    vec3 result = (ambient + diffuse + specular) * color;
	//vec3 lakeColor = vec3(0, 0.553, 0.592);
	vec3 lakeColorLight = vec3(0.098, 0.286, 0.298);
	vec3 lakeColorDark = vec3(0, 0.188, 0.20);

	float startTransparency = 0.5;
	float margin1 = 3.0;
	float margin2 = 10.0;
	if (render_mode == 1.0) 
	{
		vec3 res = mix(lakeColorLight,lakeColorDark,min(1, max(0, -(position.y + margin1)/margin2)));
		result = mix(res,result,min(1, max(0, (-startTransparency/margin1)*-position.y+startTransparency)));
	}
	else if (position.y <= 1) result = mix(result, vec3(1.0), foamShift);
	//float fogFactor = max( min((viewDistance-(gl_FragCoord.z / gl_FragCoord.w ))/ (viewDistance - viewDistance/5),1), 0);

	
	//float fogFactor = max( min(linearDistance/ (viewDistance),1), 0.05);
	float trueFogFactor = max( min((viewDistance-(linearDistance ))/ (viewDistance - viewDistance/5),1), 0);
	//fragColor = mix(vec4(1.0,1.0,1.0,1.0),fogColor,0.0);
	vec4 trueColorWFog = mix(fogColor,vec4(result, 1.0),trueFogFactor);
	
	//if (render_mode == 0.0)
	//{
 //       vec4 wateredColor = mix(trueColorWFog, vec4(vec3(waterColor),1.0), waterColor.w);
 //       float perceivedReflDist = view_pos.y * linearDistance / (view_pos.y + position.y);
	//	perceivedReflDist = abs(perceivedReflDist);
 //       float distanceFogFactor = max( min((viewDistance-(perceivedReflDist ))/ (viewDistance - viewDistance/5),1), 0);
 //       fragColor = mix(fogColor,wateredColor,distanceFogFactor);
 //   }		
	//else
	fragColor = trueColorWFog;
	//fragColor = color;
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
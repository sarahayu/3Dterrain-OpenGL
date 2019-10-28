#version 330 core												 
out vec4 fragColor;

varying vec3 position;
varying vec2 texCoord;

uniform vec3 lightColor;
uniform vec3 lightDirection;
uniform vec3 viewPos;
uniform vec4 fogColor;
uniform vec4 waterColor;
uniform float viewDistance;
uniform samplerCube skybox;

uniform bool reflection;

uniform sampler2D water;
uniform float texScale;

uniform float timeOffset;
											 
void main()													 
{					
	vec3 norm = vec3(0.0,0.0,1.0);
	
	// refer to https://stackoverflow.com/questions/1110844/multiple-texture-images-blended-together-onto-3d-ground?rq=1
	//vec3 waterColor = texture2D(water, (texCoord + timeOffset) * texScale).rgb;
  
	//float specularStrength = 0.0;	// spcular doesn't work with directional lighting :/'
 //   float ambientStrength = 0.6;
	//float shininess = 1.0;

	//vec3 lightDir = normalize(-lightDirection);

 //   vec3 ambient = ambientStrength * waterColor;

	//float diff = max(dot(norm, lightDir), 0.0);
	//vec3 diffuse = diff * lightColor * waterColor;

	//vec3 viewDir = normalize(viewPos - position);
	//vec3 reflectDir = reflect(-lightDir, norm);

	//float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
	//vec3 specular = specularStrength * spec * waterColor;
	
 //   vec3 result = (ambient + diffuse + specular) * waterColor;
	float linearDistance = length(vec2(position.x, position.z)-vec2(viewPos.x, viewPos.z));
	linearDistance = abs(linearDistance);
	//float fogFactor = max( min(linearDistance/ (viewDistance),1), 0.05);

	float fogFactor = max( min((viewDistance-(linearDistance ))/ (viewDistance - viewDistance/5),1), 0);

	if (reflection)
	{
		vec3 I = normalize(position - viewPos);
		vec3 R = reflect(I, vec3(0.0,1.0,0.0));
		vec4 color = vec4(texture(skybox, R).rgb, 1.0);
		color = mix(color, vec4(vec3(waterColor),1.0), waterColor.w);
		fragColor = mix(fogColor,color,fogFactor);
	}
	else
	{
		fragColor = mix(fogColor,vec4(0.0,0.0,0.0,0.7),fogFactor);
	}

	//fragColor = mix(vec4(1.0,1.0,1.0,1.0),fogColor,0.0);
	
	//fragColor = mix(color,fogColor,fogFactor);
	//fragColor = color;
} 																 
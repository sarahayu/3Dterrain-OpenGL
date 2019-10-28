#version 330 core												 
out vec4 FragColor;		
				
in vec3 FragPos;
in vec3 WorldPos;

uniform vec3 lightColor;
uniform vec3 lightDirection;
uniform vec3 viewPos;
uniform float terrainDistance;
											 
void main()													 
{																 
   //FragColor = vec4(ourColor,0.25);											 
   vec3 blue = vec3(0.0,0.5,1.0);
   
    float ambientStrength = 0.18;
    vec3 ambient = ambientStrength * lightColor;


	vec3 norm = normalize(vec3(0.0,1.0,0.0));
	vec3 lightDir = normalize(-lightDirection);

	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;
	

	float specularStrength = 0.5;
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);

	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 128);
	vec3 specular = specularStrength * spec * blue;
	
    vec3 result = (ambient + diffuse + specular) * blue;
	float alpha = (terrainDistance - max(WorldPos.x,terrainDistance / 2.7))/terrainDistance*1.3;

   FragColor = vec4(result,min(alpha,0.8));
} 																 
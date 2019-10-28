#version 330 core												 
out vec4 FragColor;		
									 
in vec3 ourColor;
in vec3 Normal;
in vec3 FragPos;
in vec3 worldPos;

uniform vec3 lightColor;
uniform vec3 lightDirection;
uniform vec3 viewPos;
uniform float seaLevel;
uniform float snowLevel;
uniform float terrainDistance;

float rand(vec2 co)
{
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}
											 
void main()													 
{						
	vec3 norm = normalize(Normal);
											 
   //FragColor = vec4(ourColor,0.25);	
   									 
  //vec3 landColor = vec3(0.761,0.698,0.502);	
  //if (worldPos.y >= snowLevel)
  //{
  //	   if (rand(vec2(norm.y,norm.y)) / 2.0 + 0.5 <= min(20.0, worldPos.y - snowLevel) / 20.0)
	//   {
  //		   if (norm.y > 0.423) landColor = vec3(1.0,1.0,1.0);
	//	   else landColor = vec3(0.208,0.282,0.369);
	//   }
	//   else
	//   {
  //		   if (norm.y > 0.342) landColor = vec3(0.082,0.467,0.157);
	//	   else landColor = vec3(0.4863,0.3686,0.2558);
	//   }
  //}
  //else if (worldPos.y > seaLevel + 2.0) 
  //{
  //	   if (norm.y > 0.342) landColor = vec3(0.082,0.467,0.157);
	//   else landColor = vec3(0.4863,0.3686,0.2558);
  //}
  
    float ambientStrength = 0.18;
    vec3 ambient = ambientStrength * lightColor;


	vec3 lightDir = normalize(-lightDirection);

	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;
	

	float specularStrength = 0.5;
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);

	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = specularStrength * spec * ourColor;
	
    vec3 result = (ambient + diffuse + specular) * ourColor;
	float alpha = min((terrainDistance - max(worldPos.x,terrainDistance / 2.7))/terrainDistance*1.3,0.8);

   //FragColor = vec4(result,alpha);
   FragColor = vec4(result,alpha);
} 																 
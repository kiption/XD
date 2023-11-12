#version 330 core

in vec3 fragPos;
in vec3 normal;
in vec2 textCoord;

out vec4 FragColor;

uniform vec3 objColor;
uniform sampler2D textureC;


uniform vec3 LightColor;
uniform vec3 LightPos;
uniform mat4 LightTransform;
uniform vec3 cameraPos;
uniform int Blendcheck;
void main()
{
//ambient
	float ambientLight = 0.3f;
	vec3 ambient = ambientLight * LightColor;
//diffuse
	vec3 MyLightPos = vec3(LightTransform * vec4(LightPos, 1.0f));
	vec3 normalDir = normalize(normal);
	vec3 lightDir = normalize(MyLightPos - fragPos);
	float diffuseLight = max(dot(lightDir, normalDir), 0.0f);
	vec3 diffuse = diffuseLight * LightColor;
//specular
	int shininess = 128;
	vec3 viewDir = normalize(cameraPos - fragPos);
	vec3 reflectDir = reflect(-lightDir , normalDir);//¿‘ªÁ ∫§≈Õ π›ªÁ ∫§≈Õ // -lightDir???
	float specularLight = max(dot(viewDir, reflectDir), 0.0f);
	specularLight = pow(specularLight, shininess);
	vec3 specular = specularLight * LightColor;

	vec3 re = ambient + diffuse + specular;


	if(Blendcheck==0)
	{
		vec3 result = vec3(re.x * objColor.x, re.y * objColor.y, re.z * objColor.z);
	FragColor = vec4(result,1.0f);
	}
	if(Blendcheck==1)
	{
		vec3 result = vec3(re.x * objColor.x, re.y * objColor.y, re.z * objColor.z);
	FragColor = vec4(result,0.5f);
	}
	if(Blendcheck==2)
	{
		vec3 result = vec3(re.x * texture(textureC, textCoord).x, re.y * texture(textureC, textCoord).y, re.z * texture(textureC, textCoord).z);
	FragColor = vec4(result, 1.0f);
	}

}
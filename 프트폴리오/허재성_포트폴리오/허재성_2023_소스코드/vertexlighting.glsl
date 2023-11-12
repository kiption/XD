#version 330 core

layout(location = 0)in vec3 vertex;
layout(location = 1)in vec3 vertexNormal;
	
uniform mat4 modeltransform;
uniform mat4 viewtransform;
uniform mat4 projectiontransform;
uniform mat4 normaltransform;

out vec3 fragPos;
out vec3 normal;

void main()
{
	gl_Position = projectiontransform * viewtransform * modeltransform * vec4(vertex, 1.0f);
	fragPos = vec3(modeltransform * vec4(vertex, 1.0f));
	normal = vec3(normaltransform * vec4(vertexNormal, 1.0f));
}
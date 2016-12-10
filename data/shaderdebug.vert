#version 150 core

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 modelMatrix;

// TODO: normals/uvs
in vec3 position;
in vec3 normal;
in vec2 uv;

out vec4 cameraSpaceNormal;
out vec4 worldPosition;
out vec4 cameraSpacePos;
out vec2 texCoord;

void main(void)
{  		 
	worldPosition = (modelMatrix * vec4(position, 1.0));

	texCoord = vec2(uv.x, 1.0-uv.y);
  
	cameraSpaceNormal = normalize(modelViewMatrix*vec4(normal,0.0));
	
	cameraSpacePos = (modelViewMatrix * vec4(position, 1.0));
	gl_Position = projectionMatrix * cameraSpacePos;
}

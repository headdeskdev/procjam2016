#version 150 core

uniform mat3 screenViewMatrix;

in vec2 position;
in vec2 uv;
in vec4 colour;

out vec2 texCoord;
smooth out vec4 vertexColour;

void main(void)
{  		 
	texCoord = vec2(uv.x, uv.y);
	vertexColour = colour;  
	gl_Position = vec4(screenViewMatrix*vec3(position,1.0),1.0);
}
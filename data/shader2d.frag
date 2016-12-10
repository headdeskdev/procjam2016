#version 150 core

uniform sampler2D textureMap;

in vec2 texCoord;
in vec4 vertexColour;

out vec4 out_Color;

void main(void)
{	
	vec4 textureData = texture(textureMap,texCoord);
  	out_Color = vertexColour*textureData;
}
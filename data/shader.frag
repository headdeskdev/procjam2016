#version 150 core



uniform float textureScale;
uniform sampler2D textureMap;
uniform vec3 cameraSpaceLightDirection; // TODO: replace with better lighting
//uniform vec3 ambientLight;
//uniform vec4 lightingInfo[4];

in vec4 cameraSpaceNormal;
in vec4 worldPosition;
in vec4 cameraSpacePos;
in vec2 texCoord;

out vec4 out_Color;

void main(void)
{		
	float x = length(cameraSpacePos);
	float invfog = exp(-0.025 * x) * 1.0;
	float light = 0.65 + 0.35*dot(normalize(vec4(-cameraSpaceLightDirection,0.0)),cameraSpaceNormal);

	vec4 textureData = texture(textureMap,texCoord);
	float textureValue = (1.0-step(textureData.r, 0.5))*0.7 + 0.3;

	if (textureValue < 0.5) {
	 invfog = invfog*0.5+0.5;
	}

    float bluepink = textureValue*light*invfog + 0.5*(1.0-invfog);
    float gamma = 2.2;  
	vec3 converted = vec3(pow(1.0-bluepink,gamma),pow(bluepink,gamma),1.0);
	
	float fogreduction = clamp(invfog*0.4 + 0.6,0.6,1.0);

    out_Color = vec4(converted*fogreduction,1.0);	
    //out_Color = vec4(vec3(fogreduction*bluepink),1.0);

}
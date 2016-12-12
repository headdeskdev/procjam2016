#version 150

uniform sampler2D mainTexture;

// Lighting uniforms
uniform vec3 ambientColor;
uniform vec3 directionalViewDirection[2];
uniform vec3 directionalColor[2];
uniform usamplerBuffer clusterBufferTexture;
uniform usamplerBuffer clusterItemBufferTexture;
uniform samplerBuffer lightBufferTexture;

uniform vec3 backgroundColor;

in vec3 viewPosition;
in vec4 frustumPosition;
in vec3 viewNormal;
in vec2 fragUV;

out vec4 out_Color;

const float samplesX = 16.0;
const float samplesY = 8.0;
const float samplesZ = 24.0;

vec3 calculateLighting(vec3 normal, vec3 clusterPosition, float specular) 
{
    vec3 lighting = ambientColor;
    //lighting += vec3(1.0);
    for (int i = 0; i < 2; i++) {
        float intensity = max(0.0,dot(directionalViewDirection[i],-normal));
        lighting += directionalColor[i]*intensity;
    }
    int x = int(clamp(floor(clusterPosition.x * samplesX),0.0,samplesX-1.0));
    int y = int(clamp(floor(clusterPosition.y * samplesY),0.0,samplesY-1.0));
    int z = int(clamp(floor(clusterPosition.z * samplesZ),0.0,samplesZ-1.0));
    int cluster = (x + y * 16 + z * 8 * 16) * 2;
    uint lightOffset = texelFetch(clusterBufferTexture, cluster).r;
    uint lightCount = texelFetch(clusterBufferTexture, cluster + 1).r;

    for (uint i = uint(0); i < lightCount; i++) {
        int offset = int(lightOffset + i);
        int lightNumber = int(texelFetch(clusterItemBufferTexture, offset).r);

        vec4 lightViewPositionRange = texelFetch(lightBufferTexture, lightNumber*3);
        vec4 lightViewDirectionDot = texelFetch(lightBufferTexture, lightNumber*3+1);
        vec4 lightColorW = texelFetch(lightBufferTexture, lightNumber*3+2);

        vec3 lightViewPosition = lightViewPositionRange.xyz;
        float lightRange = lightViewPositionRange.w;

        vec3 lightViewDirection = lightViewDirectionDot.xyz;
        float lightDot = lightViewDirectionDot.w;

        vec3 lightColor = lightColorW.xyz;
		
        vec3 lightRay = viewPosition-lightViewPosition;
        float length = length(lightRay);
        lightRay = lightRay / length;
		
        float cone = smoothstep(lightDot-0.05, lightDot+0.05, dot(lightRay, lightViewDirection));
        float denom = (1.0+length/(0.25*lightRange));
        float fade = max(0.0,(25.0/(denom*denom) - 1.0) / 24.0);
        float lightIntensity = lightColorW.w*cone*fade*max(0.0,dot(lightRay,-normal));

        lighting += lightColor*lightIntensity;
    }
    return lighting;
}

vec3 applyFog(vec3  input, float distance)
{
	float fogAmount = clamp(1.0 - exp( -distance*0.01 ), 0.0, 1.0);
    vec3 fogColor = pow(backgroundColor,vec3(2.2));
    return mix( input, fogColor, fogAmount );
}

void main(void)
{
    vec3 fragmentFrustum = frustumPosition.xyz;
    fragmentFrustum.z = 0.125662*log(fragmentFrustum.z + 0.25) + 0.131923;
    fragmentFrustum.xy = (fragmentFrustum.xy/frustumPosition.w)*0.5 + vec2(0.5);
    vec4 textureData = texture(mainTexture,fragUV);
	vec3 color = calculateLighting(normalize(viewNormal),fragmentFrustum,step(textureData.r, 0.5)) * vec3((step(textureData.r, 0.5))*0.9+0.05);	
	color = applyFog(color, length(viewPosition));
    out_Color = vec4(pow(color, vec3(1.0/2.2)),1.0);
}
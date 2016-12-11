#version 140

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 normalModelViewMatrix;

in vec3 position;
in vec3 normal;
in vec2 uv;

out vec3 viewPosition;
out vec4 frustumPosition;
out vec3 viewNormal;
out vec2 fragUV;

void main(void)
{        
    vec4 viewPositionH = (modelViewMatrix * vec4(position, 1.0));

    vec4 coordinate = projectionMatrix * viewPositionH;
    
    viewPosition = viewPositionH.xyz;    

    frustumPosition = vec4(coordinate.xy, -viewPositionH.z, coordinate.w);

    fragUV = vec2(uv.x, 1.0-uv.y);  
    viewNormal = (normalModelViewMatrix*vec4(normal, 0.0)).xyz;
    
    gl_Position = coordinate;
}

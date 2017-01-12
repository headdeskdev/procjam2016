#version 150

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 normalModelViewMatrix;

in vec3 position;
in vec3 normal;
in vec2 uv;

void main(void)
{        
    vec4 viewPositionH = (modelViewMatrix * vec4(position, 1.0));

    vec4 coordinate = projectionMatrix * viewPositionH;
    
    gl_Position = coordinate;
}

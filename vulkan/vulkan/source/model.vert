#version 450
layout(location=0) in vec3 inPosition;
layout(location=1) in vec3 inColor;
layout(location=2) in vec2 inUV;

layout(location=0) out vec2 outUV;

layout(binding=0) uniform Matrices
{
    mat4 world;
    mat4 view;
    mat4 projection;        
};

void main()
{
    mat4 projectionViewWorld = projection * view * world;
    gl_Position = projectionViewWorld * vec4(inPosition, 1.0);
    outUV = inUV;
}
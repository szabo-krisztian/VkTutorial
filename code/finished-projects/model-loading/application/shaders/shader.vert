#version 450

layout(location = 0) in vec3 inVertex;
layout(location = 1) in vec3 inNormal;

layout(set = 0, binding = 0) uniform ModelTransform
{
    mat4 vertexTransform;
    mat4 normalTransform;
    mat4 view;
    mat4 proj;
} model;

layout(location = 0) out vec3 outVertex;
layout(location = 1) out vec3 outNormal;

void main()
{
    vec4 worldPosition = model.vertexTransform * vec4(inVertex, 1.0);
    gl_Position = model.proj * model.view * worldPosition;
    outVertex = vec3(worldPosition) / worldPosition.w;
    outNormal = normalize(vec3(model.normalTransform * vec4(inNormal, 0.0)));
}

#version 450

layout(location = 0) in vec3 inPosition; // Vertex position
layout(location = 1) in vec3 inNormal;   // Vertex normal

layout(set = 0, binding = 0) uniform CameraTransform {
    mat4 view;  // View matrix
    mat4 proj;  // Projection matrix
} camera;

layout(set = 1, binding = 0) uniform ModelTransform {
    mat4 vertex; // Model matrix for vertex positions
    mat4 normal; // Model matrix for normals
} model;

layout(location = 0) out vec3 fragVertex; // Fragment position
layout(location = 1) out vec3 fragNormal; // Fragment normal

void main() {
    vec4 worldPosition = model.vertex * vec4(inPosition, 1.0);
    gl_Position = camera.proj * camera.view * worldPosition;

    // Pass fragment position and normal to the fragment shader
    fragVertex = vec3(worldPosition) / worldPosition.w; // Perspective divide
    fragNormal = vec3(model.normal * vec4(normalize(inNormal), 0.0));

}

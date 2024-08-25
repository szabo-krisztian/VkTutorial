#version 450

layout(location = 0) in vec3 inPosition;

layout(set = 0, binding = 0) uniform CameraTransform
{
    mat4 view;
    mat4 proj;
} camera;

layout( push_constant ) uniform constants
{
	mat4 transform;
    vec3 color;
} pushConstant;

layout(location = 0) out vec3 fragColor;

void main()
{
    gl_Position = camera.proj * camera.view * pushConstant.transform * vec4(inPosition, 1.0);
    fragColor = pushConstant.color;
}


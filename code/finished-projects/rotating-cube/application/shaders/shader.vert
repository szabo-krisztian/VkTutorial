#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

mat4 myMatrix = mat4(
        vec4(-1.7755, -0.0911186, -0.192835, -0.19245),
        vec4(0.0, 2.36908, -0.192835, -0.19245),
        vec4(-0.3551, 0.455593, 0.964177, 0.96225),
        vec4(-2.15848e-07, 0.0, 10.2129, 10.3923)
    );

void main() {
    gl_Position = myMatrix * vec4(inPosition, 1.0);
    fragColor = inColor;
}
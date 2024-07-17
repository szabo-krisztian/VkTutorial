#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

mat4 myMatrix = mat4(
        vec4(1.81066,  0,       0,       0),
        vec4(0,        -2.41421, 0,       0),
        vec4(0,        0,       1.002,   1),
        vec4(0,        0,       2.80581, 3)
    );

void main() {
    gl_Position = myMatrix * vec4(inPosition, 1.0);
    fragColor = inColor;
}
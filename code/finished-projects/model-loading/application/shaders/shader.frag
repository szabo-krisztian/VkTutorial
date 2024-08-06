#version 450

layout(location = 0) in vec3 fragVertex; // Fragment position in world space
layout(location = 1) in vec3 fragNormal; // Fragment normal in world space

layout(set = 2, binding = 0) uniform Material {
    float shininess;
    vec3 ambientColor;
    vec3 diffuseColor;
    vec3 specColor;
    vec3 emissive;
    float alpha;
} material;

layout(set = 1, binding = 1) uniform Light {
    vec3 lightPos;
    vec3 lightColor;
    float lightPower; // Light intensity
} light;

layout(set = 1, binding = 2) uniform Camera {
    vec3 position;
} camera;

const float screenGamma = 2.2;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 normal = normalize(fragNormal);
    vec3 lightDir = light.lightPos - fragVertex;
    float distance = length(lightDir);
    
    if (distance < 0.0001) {
        discard; // Prevent division by zero
    }
    
    float distanceSquared = distance * distance;
    lightDir = normalize(lightDir);

    // Lambertian reflectance
    float lambertian = max(dot(lightDir, normal), 0.0);
    
    // Blinn-Phong specular calculation
    vec3 viewDir = normalize(camera.position - fragVertex);
    vec3 halfDir = normalize(lightDir + viewDir);
    float specAngle = max(dot(halfDir, normal), 0.0);
    float specular = pow(specAngle, material.shininess);
    
    // Combine components
    vec3 colorLinear = material.ambientColor +
                       material.diffuseColor * lambertian * light.lightColor * light.lightPower / distanceSquared +
                       material.specColor * specular * light.lightColor * light.lightPower / distanceSquared +
                       material.emissive;
                       
    // Apply gamma correction
    vec3 colorGammaCorrected = pow(colorLinear, vec3(1.0 / screenGamma));
    
    // Final color output
    outColor = vec4(colorGammaCorrected, material.alpha);
}

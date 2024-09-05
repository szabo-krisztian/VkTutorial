#version 450

precision mediump float;

layout(location = 0) in vec3 normalInterp;
layout(location = 1) in vec3 vertPos;

layout(set = 0, binding = 1) uniform Light
{
    vec3 position;
    vec3 color;
    float power;
} light;

layout(set = 0, binding = 2) uniform Camera
{
    vec3 position;
} camera;

layout(set = 1, binding = 0) uniform Material
{
    float shininess;
    vec3 ambientColor;
    vec3 diffuseColor;
    vec3 specColor;
    vec3 emissive;
    float alpha;
} material;

const float screenGamma = 2.2;

layout(location = 0) out vec4 outColor;

/*
 *  Blinn-Phong model: https://en.wikipedia.org/wiki/Blinn–Phong_reflection_model
 */

void main()
{
    vec3 normal = normalize(normalInterp);
    vec3 lightDir = light.position - vertPos;
    float distance = length(lightDir);
    distance = distance * distance; // square the distance
    lightDir = normalize(lightDir);

    float lambertian = max(dot(lightDir, normal), 0.0);
    float specular = 0.0;

    if (lambertian > 0.0)
    {
        vec3 viewDir = normalize(camera.position - vertPos);

        // Blinn-Phong
        vec3 halfDir = normalize(lightDir + viewDir);
        float specAngle = max(dot(halfDir, normal), 0.0);
        specular = pow(specAngle, material.shininess);

        // Phong (for comparison)
        if (material.shininess < 0.0)
        {
            vec3 reflectDir = reflect(-lightDir, normal);
            specAngle = max(dot(reflectDir, viewDir), 0.0);
            specular = pow(specAngle, material.shininess / 4.0);
        }
    }

    vec3 ambient = material.ambientColor;
    vec3 diffuse = material.diffuseColor * lambertian * light.color * light.power / distance;
    vec3 spec = material.specColor * specular * light.color * light.power / distance;

    vec3 colorLinear = diffuse + spec;

    vec3 colorGammaCorrected = pow(colorLinear, vec3(1.0 / screenGamma));
    outColor = vec4(colorGammaCorrected, material.alpha);
}

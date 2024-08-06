#version 450

layout(location = 0) in vec3 worldPosition;
layout(location = 1) in vec3 worldNormal;

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

void main()
{
  vec3 lightDirection = light.position - worldPosition;
  float distance = pow(length(lightDirection), 2);
  lightDirection = normalize(lightDirection);

  float lambertian = max(dot(lightDirection, worldNormal), 0.0);
  float specular = 0.0;

  if (lambertian > 0.0)
  {
    vec3 viewDirection = normalize(worldPosition - camera.position);
    vec3 halfDirection = normalize(lightDirection + viewDirection);
    float specularAngle = max(dot(halfDirection, worldNormal), 0.0);
    specular = pow(specularAngle, material.shininess);   
  }

  vec3 colorLinear = vec3(1,0,0) * lambertian * vec3(1,1,1) * 10.0 / distance;
                     vec3(1,0,0) * specular * vec3(1,1,1) * 10.0 / distance;
  
  vec3 colorGammaCorrected = pow(colorLinear, vec3(1.0 / screenGamma));
  outColor = vec4(colorGammaCorrected, 1.0);
}

#version 330 core

layout(location=0) out vec4 fragmentColor;

uniform vec3 diffuseComponent;

in vec4 normal;
in vec4 lightDir;

void main(void)
{
    vec4 N = normalize(normal);
    vec4 L = normalize(lightDir);

    float val = max(0.0, dot(N, L));

    vec3 diffuseShading = vec3(
        diffuseComponent.r * val,
        diffuseComponent.g * val,
        diffuseComponent.b * val
    );

    fragmentColor = vec4(diffuseShading, 1.0);
}
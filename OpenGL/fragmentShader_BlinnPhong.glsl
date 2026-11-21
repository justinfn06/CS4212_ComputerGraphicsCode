#version 330 core

layout(location=0) out vec4 fragmentColor;

uniform vec3 diffuseComponent;
uniform vec3 specularComponent;
uniform float phongExponent;
uniform vec4 viewDirWorld;

in vec4 normal;
in vec4 lightDir;

void main(void)
{
    vec4 N = normalize(normal);
    vec4 L = normalize(lightDir);
    vec4 V = normalize(viewDirWorld);

    float diffuseVal = max(0.0, dot(N, L));

    float specularVal = 0.0;

    if (diffuseVal > 0.0)
    {
        vec4 H = normalize(L + V);

        specularVal = pow(
            max(0.0, dot(N, H)),
            phongExponent
        );
    }

    vec3 diffuseShading = diffuseComponent * diffuseVal;
    vec3 specularShading = specularComponent * specularVal;

    fragmentColor = vec4(
        diffuseShading + specularShading,
        1.0
    );
}
#version 330 core

layout(location=0) out vec4 fragmentColor;

in vec4 normal;

void main(void)
{
    vec3 N = normalize(normal.xyz);

    fragmentColor = vec4(
        0.5 * (N + vec3(1.0)),
        1.0
    );
}
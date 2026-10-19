#pragma once

#include "scene.h"
#include "shader.h"

class Mirror : public Shader
{
public:
    explicit Mirror(vec3 albedo = vec3(1.0, 1.0, 1.0))
        : albedo(albedo) {}

    vec3 rayColor(
        const HitStruct& h,
        const std::vector<LightSample>&,
        const Scene& scene,
        int depth
    ) override
    {
        vec3 incoming = -h.viewDir;

        vec3 reflected = reflect(incoming, h.normal);
        ray reflectedRay(h.point, reflected);

        return albedo * scene.computeRayColor(reflectedRay, depth);
    }

private:
    vec3 albedo;
};
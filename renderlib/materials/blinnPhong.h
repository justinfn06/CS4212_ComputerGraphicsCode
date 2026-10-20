#pragma once
#include <algorithm>
#include <cmath>
#include "shader.h"

class BlinnPhong : public Shader
{
public:
    BlinnPhong(vec3 diffuseColor, vec3 specularColor, double exponent)
        : kd(diffuseColor),
          ks(specularColor),
          phongExp(exponent) {}

    vec3 rayColor(
        const HitStruct& h,
        const std::vector<LightSample>& lights,
        const Scene&,
        int
    ) override
    {
        vec3 result(0, 0, 0);

        for (const auto& light : lights) {
            double nDotL =
                std::max(0.0, dot(h.normal, light.direction));

            if (nDotL <= 0.0) {
                continue;
            }

            vec3 diffuse = kd * nDotL;
            vec3 specular(0, 0, 0);

            vec3 halfSum = light.direction + h.viewDir;

            if (halfSum.length_squared() >= 1e-12) {
                vec3 halfVector = unit_vector(halfSum);

                double nDotH =
                    std::max(0.0, dot(h.normal, halfVector));

                specular = ks * std::pow(nDotH, phongExp);
            }

            result += (diffuse + specular) * light.intensity;
        }
        return result;
    }

private:
    vec3 kd;
    vec3 ks;
    double phongExp;
};
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

    vec3 rayColor(const HitStruct& h) override
    {
        vec3 toLight = lightPosition - h.point; // toLight is the vector from the hit point to the light source

        // if light source is at same position as hit point, return black. 
        if (toLight.length_squared() == 0.0) {
            return vec3(0, 0, 0);
        }

        vec3 lightDir = unit_vector(toLight);
        double nDotL = std::max(0.0, dot(h.normal, lightDir)); // nDotL is the cosine of angle between surface normal and light direction

        // no direct illumination when light is behind the surface
        if (nDotL <= 0.0) {
            return vec3(0, 0, 0);
        }

        vec3 diffuse = kd * nDotL;
        vec3 halfSum = lightDir + h.viewDir;

        if (halfSum.length_squared() < 1e-12) {
            return diffuse;
        }

        vec3 halfVector = unit_vector(halfSum);
        double nDotH =
            std::max(0.0, dot(h.normal, halfVector));

        vec3 specular = ks * std::pow(nDotH, phongExp);

        return diffuse + specular;
    }

private:
    vec3 kd;
    vec3 ks;
    double phongExp;
    vec3 lightPosition = vec3(0, 10, 5);
};
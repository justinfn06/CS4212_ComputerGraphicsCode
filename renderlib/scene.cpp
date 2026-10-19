#include "scene.h"

#include <limits>

bool Scene::isShadowed(
    const point3& point,
    const PointLight& light
) const
{
    constexpr float epsilon = 0.001f;

    ray shadowRay(point, light.position - point);

    float tMax = 1.0f;
    HitStruct shadowHit;

    for (const auto& shape : shapes) {
        if (shape->intersect(
                shadowRay, epsilon, tMax, shadowHit)) {
            return true;
        }
    }

    return false;
}

color Scene::computeRayColor(
    const ray& r,
    int depth
) const
{
    if (depth <= 0) {
        return color(0, 0, 0);
    }

    constexpr float tMin = 0.001f;
    float tMax = std::numeric_limits<float>::max();

    HitStruct hit;
    bool hitShape = false;

    for (const auto& shape : shapes) {
        if (shape->intersect(r, tMin, tMax, hit)) {
            hitShape = true;
        }
    }

    if (!hitShape) {
        return color(0.5, 0.7, 1.0);
    }

    std::vector<LightSample> visibleLights;

    for (const auto& light : lights) {
        vec3 toLight = light.position - hit.point;

        if (toLight.length_squared() <= 1e-12) {
            continue;
        }

        if (isShadowed(hit.point, light)) {
            continue;
        }

        visibleLights.push_back(
            LightSample{unit_vector(toLight), light.intensity}
        );
    }

    return hit.shape->shade(
        hit,
        visibleLights,
        *this,
        depth - 1
    );
}
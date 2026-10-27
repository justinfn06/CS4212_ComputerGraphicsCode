#include "scene.h"
#include "../geometry/BoundingVolumeNode.h"

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

    return bvhRoot &&
           bvhRoot->intersect(
               shadowRay, epsilon, tMax, shadowHit
           );
}

void Scene::buildBVH()
{
    if (shapes.empty()) {
        bvhRoot.reset();
        return;
    }

    bvhRoot = std::make_shared<BoundingVolumeNode>(shapes, 0);
}

color Scene::computeRayColor(const ray& r, int depth) const
{
    if (depth <= 0) {
        return color(0, 0, 0);
    }

    constexpr float tMin = 0.001f;
    float tMax = std::numeric_limits<float>::max();

    HitStruct hit;
    const bool hitShape = bvhRoot && bvhRoot->intersect(r, tMin, tMax, hit);

    if (!hitShape) {
        return backgroundColor;
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
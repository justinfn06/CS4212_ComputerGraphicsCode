#pragma once
#include <memory>
#include <vector>
#include "../geometry/shape.h"
#include "../lighting/pointLight.h"
#include "../math/color.h"

struct Scene
{
    std::vector<std::unique_ptr<Shape>> shapes;
    std::vector<PointLight> lights;

    bool isShadowed(
        const point3& point,
        const PointLight& light
    ) const;

    color computeRayColor(
        const ray& r,
        int depth
    ) const;
};
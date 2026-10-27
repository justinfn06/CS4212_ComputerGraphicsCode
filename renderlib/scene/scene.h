#pragma once
#include <memory>
#include <vector>
#include <unordered_map>
#include <string>

#include "../geometry/shape.h"
#include "../cameras/perspectiveCamera.h"
#include "../lighting/pointLight.h"
#include "../materials/shader.h"
#include "../math/color.h"

struct Scene
{

    std::unordered_map<std::string, std::unique_ptr<Shader>> shaders;

    std::unique_ptr<PerspectiveCamera> camera;

    std::vector<std::unique_ptr<Shape>> shapes;
    std::vector<PointLight> lights;

    color backgroundColor{0.0, 0.0, 0.0};

    bool isShadowed(
        const point3& point,
        const PointLight& light
    ) const;

    color computeRayColor(
        const ray& r,
        int depth
    ) const;
};
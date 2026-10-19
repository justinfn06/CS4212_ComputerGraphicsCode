#pragma once

#include "scene.h"
#include "shader.h"
// not working yet

// class Diffuse : public Shader
// {
// public:
//     vec3 rayColor(
//         const HitStruct& h,
//         const std::vector<LightSample>&,
//         const Scene& scene,
//         int depth
//     ) override
//     {
//         vec3 direction = random_on_hemisphere(h.normal);
//         ray scatteredRay(h.point, direction);

//         return 0.5 * scene.computeRayColor(scatteredRay, depth);
//     }
// };
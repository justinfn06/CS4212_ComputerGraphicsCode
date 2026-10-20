#pragma once
#include "../core/vec3.h"

struct PointLight
{
    point3 position;
    vec3 intensity;

    PointLight(point3 position, vec3 intensity)
        : position(position), intensity(intensity) {}
};
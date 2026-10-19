#pragma once
#include "vec3.h"

struct LightSample
{
    vec3 direction;  // vector from surface toward light
    vec3 intensity;  // rgb strength reaching surface
};
#pragma once
#include "vec3.h"

class Shape;

struct HitStruct
{
    float t;
    vec3 point;
    vec3 normal;
    vec3 viewDir;
    const Shape* shape = nullptr;
};
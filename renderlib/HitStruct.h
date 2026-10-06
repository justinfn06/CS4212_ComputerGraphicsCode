#include "vec3.h"

class Shape;

struct HitStruct
{
    float t;
    vec3 point;
    const Shape* shape = nullptr;
};
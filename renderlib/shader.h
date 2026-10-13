#pragma once
#include "HitStruct.h"

class Shader
{
public:
    virtual ~Shader() = default;
    virtual vec3 rayColor(const HitStruct& h) = 0;
};
#pragma once

#include "camera.h"

class PerspectiveCamera : public Camera
{
public:
    PerspectiveCamera();
    PerspectiveCamera(int pixel_nx, int pixel_ny);
    PerspectiveCamera(vec3 origin, vec3 viewDir, float focal_length, float image_plane_width, float image_plane_height, int pixel_nx, int pixel_ny);

    ray generateRay(double i, double j) override;

private:
    float left, right, top, bottom;
};
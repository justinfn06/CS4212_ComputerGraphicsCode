#include "PerspectiveCamera.h"

PerspectiveCamera::PerspectiveCamera()
: Camera()
{
    left = -imagePlaneWidth / 2.0;
    right = imagePlaneWidth / 2.0;
    bottom = -imagePlaneHeight / 2.0;
    top = imagePlaneHeight / 2.0;
}

PerspectiveCamera::PerspectiveCamera(int pixel_nx, int pixel_ny)
: Camera(pixel_nx, pixel_ny)
{
    left = -imagePlaneWidth / 2.0;
    right = imagePlaneWidth / 2.0;
    bottom = -imagePlaneHeight / 2.0;
    top = imagePlaneHeight / 2.0;
} 
  
PerspectiveCamera::PerspectiveCamera(vec3 origin, vec3 viewDir, float focal_length, float image_plane_width, float image_plane_height, int pixel_nx, int pixel_ny)
: Camera(origin, viewDir, vec3(0, 1, 0), focal_length, image_plane_width, image_plane_height, pixel_nx, pixel_ny)
{
    left = -imagePlaneWidth / 2.0;
    right = imagePlaneWidth / 2.0;
    bottom = -imagePlaneHeight / 2.0;
    top = imagePlaneHeight / 2.0;
}

ray PerspectiveCamera::generateRay(int i, int j)
{
    float u = left + (right - left) * (i + 0.5) / (float)nx;
    float v = bottom + (top - bottom) * (j + 0.5) / (float)ny;
    vec3 rayDir = -focalLength * W + u * U + v * V;

    return ray(pos, rayDir);
}
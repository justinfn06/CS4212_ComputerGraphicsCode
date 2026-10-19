#pragma once

#include "ray.h"

class Camera
{
public:
  Camera();
  Camera(int pixel_nx, int pixel_ny);
  Camera(vec3 position, vec3 viewDir, vec3 upDir, float focal_length, float image_plane_width, float image_plane_height, int pixel_nx, int pixel_ny);

  virtual ray generateRay(int i, int j) = 0;

protected:
  // position of the camera
  vec3 pos;

  // the camera's basis vectors
  vec3 U, V, W;

  // focal length
  float focalLength;

  // imageplane dimensions
  float imagePlaneWidth, imagePlaneHeight;

  // number of pixels in x and y direction
  int nx, ny;
};
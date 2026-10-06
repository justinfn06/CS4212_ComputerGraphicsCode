#include <iostream>
#include <vector>
#include <memory>
#include <limits>
#include "Framebuffer.h"
#include "PerspectiveCamera.h"
#include "ray.h"
#include "Shape.h"
#include "Sphere.h"
#include "Triangle.h"

vec3 computeRayColor(const ray &r, const std::vector<std::shared_ptr<Shape>> &shapes)
{
  float t_min = 0.001f;
  float t_max = std::numeric_limits<float>::max();

  HitStruct closestHit;
  closestHit.t = t_max;
  bool hitAnything = false;

  // Check intersection with all shapes, find closest
  for (const auto &shape : shapes) {
    HitStruct tempHit;
    if (shape->intersect(r, t_min, t_max, tempHit)) {
      if (tempHit.t < closestHit.t) {
        closestHit = tempHit;
        hitAnything = true;
        t_max = tempHit.t;
      }
    }
  }

  if (hitAnything) {
    return closestHit.shape->getColor();
  }

  // Background color
  vec3 unit_direction = unit_vector(r.direction());
  auto a = 0.5 * (unit_direction.y() + 1.0);
  return (1.0 - a) * vec3(1.0, 1.0, 1.0) + a * vec3(0.5, 0.7, 1.0);
}

int main(int argc, char *argv[])
{
  Framebuffer fb(200, 200);

  // Default Camera: focal length = 1.0, image plane width = 0.5
  PerspectiveCamera cam(vec3(0, 0, 0), vec3(0, 0, -1), 1.0, 0.5, 0.5, 200, 200);

  // Create scene with three triangles
  std::vector<std::shared_ptr<Shape>> shapes;

  // Red Triangle 1
  shapes.push_back(std::make_shared<Triangle>(
    vec3(-1.2, -0.2, -7), vec3(0.8, -0.5, -5), vec3(0.9, 0, -5),
    vec3(1.0, 0.0, 0.0)));

  // Green Triangle 2
  shapes.push_back(std::make_shared<Triangle>(
    vec3(0.773205, -0.93923, -7), vec3(0.0330127, 0.94282, -5), vec3(-0.45, 0.779423, -5),
    vec3(0.0, 1.0, 0.0)));

  // Blue Triangle 3
  shapes.push_back(std::make_shared<Triangle>(
    vec3(0.426795, 1.13923, -7), vec3(-0.833013, -0.44282, -5), vec3(-0.45, -0.779423, -5),
    vec3(0.0, 0.0, 1.0)));

  // SPHERE RENDERING (COMMENTED OUT)
  // Default Camera: focal length = 0.7, image plane width = 0.5
  // PerspectiveCamera cam(vec3(0, 0, 0), vec3(0, 0, -1), 0.7, 0.5, 0.5, 200, 200);
  //
  // Yellow Sphere 1: center (0, -1.25, -7), radius 0.3
  // shapes.push_back(std::make_shared<Sphere>(vec3(0, -1.25, -7), 0.3f, vec3(0.992, 0.863, 0.239)));
  //
  // Blue Sphere 2: center (0, 0, -10), radius 3
  // shapes.push_back(std::make_shared<Sphere>(vec3(0, 0, -10), 3.0f, vec3(0.149, 0.451, 0.698)));
  //
  // Yellow Sphere 3: center (0, 0, -15), radius 5
  // shapes.push_back(std::make_shared<Sphere>(vec3(0, 0, -15), 5.0f, vec3(0.992, 0.863, 0.239)));



  for (int x = 0; x < 200; x++) {
    for (int y = 0; y < 200; y++) {
      ray r = cam.generateRay(x, y);
      vec3 pixelColor = computeRayColor(r, shapes);
      fb.setPixelColor(x, y, pixelColor);
    }
  }

  fb.exportToPNG("triangles.png");

  return 0;
}
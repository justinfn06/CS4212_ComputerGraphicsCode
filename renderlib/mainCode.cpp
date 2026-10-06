#include "camera.h"
#include "Framebuffer.h"
#include "sphere.h"

#include <iostream>

int main() {
    const int image_width = 300;
    const int image_height = 200;

    const point3 camera_position(0, 0, 0);
    const point3 view_direction(0, 0, -1);
    const double image_plane_width = 3.0;
    const double image_plane_height = 2.0;
    const double focal_length = 1.0;

    PerspectiveCamera camera(
        camera_position,
        view_direction,
        image_plane_width,
        image_plane_height,
        focal_length,
        image_width,
        image_height);

    Framebuffer fb(image_width, image_height);

    Sphere sphere(point3(0, 0, -3), 1.5);

    const color sphere_color(1.0, 0.0, 0.0); // red sphere
    const color background_color(1.0, 1.0, 1.0); // white background

    for (int j = 0; j < image_height; ++j) {
        for (int i = 0; i < image_width; ++i) {
            ray r = camera.generateRay(i, j);
            
            if (sphere.intersect(r)) {
                fb.setPixelColor(i, j, sphere_color);
            }
            else {
                fb.setPixelColor(i, j, background_color);
            }
        }
    }

    fb.exportToPNG("rendered_scene.png");

    return 0;
}
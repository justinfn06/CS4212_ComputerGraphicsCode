#include "camera.h"
#include "Framebuffer.h"
#include "sphere.h"
#include "triangle.h"

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

    Sphere big_sphere(point3(0, 0, -3), 1.5);
    Sphere medium_sphere(point3(0, 0, -3), 1);
    Sphere small_sphere(point3(0, 0, -3), 0.5);
    Triangle triangle(point3(-1, -1, -2), point3(1, -1, -2), point3(0, 1, -2));
    

    const color red(1.0, 0.0, 0.0);
    const color white(1.0, 1.0, 1.0);
    const color green(0.0, 1.0, 0.0);
    const color background_color(1.0, 1.0, 1.0); // white background

    for (int j = 0; j < image_height; ++j) {
        for (int i = 0; i < image_width; ++i) {
            ray r = camera.generateRay(i, j);
            
            if (triangle.intersect(r)) {
                fb.setPixelColor(i, j, green);
            }
            else if (small_sphere.intersect(r)) {
                fb.setPixelColor(i, j, red);
            }
            else if (medium_sphere.intersect(r)) {
                fb.setPixelColor(i, j, white);
            }
            else if (big_sphere.intersect(r)) {
                fb.setPixelColor(i, j, red);
            }
            else {
                fb.setPixelColor(i, j, background_color);
            }
        }
    }

    fb.exportToPNG("rendered_scene.png");

    return 0;
}
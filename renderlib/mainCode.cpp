#include "camera.h"
#include "Framebuffer.h"

#include <iostream>

int main() {
    const int image_width = 200;
    const int image_height = 200;

    const point3 camera_position(0, 0, 0);
    const point3 view_direction(1, 3, 5);

    const double image_plane_width = 2.0;
    const double image_plane_height = 2.0;
    const double focal_length = 1.5;

    PerspectiveCamera camera(
        camera_position,
        view_direction,
        image_plane_width,
        image_plane_height,
        focal_length,
        image_width,
        image_height);

    Framebuffer fb(image_width, image_height);

    for (int j = 0; j < image_height; ++j) {
        for (int i = 0; i < image_width; ++i) {
            ray r = camera.generateRay(i, j);
            color pixel_color = ray_color(r);
            fb.setPixelColor(i, j, pixel_color);
        }
    }

    fb.exportToPNG("rendered_scene.png");

    return 0;
}
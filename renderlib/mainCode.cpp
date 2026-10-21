#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <vector>
#include <omp.h>
#include <random>

#include "output/Framebuffer.h"
#include "cameras/perspectiveCamera.h"
#include "geometry/sphere.h"
#include "geometry/triangle.h"
#include "materials/lambertian.h"
#include "materials/blinnPhong.h"
#include "math/color.h"
#include "math/ray.h"
#include "materials/normal.h"
#include "scene/scene.h"
#include "materials/mirror.h"
#include "materials/diffuse.h"
#include "materials/constantColor.h"

double pi = 3.14159265358979323846; // pi. yum.

// helper function to add new triangle
void triangle(Scene& scene, point3 a, point3 b, point3 c, Shader& material) {
    scene.shapes.push_back(std::make_unique<Triangle>(a, b, c, material));
}

// helper function to add new sphere
void sphere(Scene& scene, point3 center, float radius, Shader& material) {
    scene.shapes.push_back(std::make_unique<Sphere>(center, radius, material));
}


// entry point for code.
int main() {
     
// 1080 * 810
const int width = 1000*4;
const int height = 1000*4;
Framebuffer fb(width, height);
PerspectiveCamera cam(vec3(0, 10, 100), vec3(0, -0.4, -1), 0.4f, 0.5f, 0.5f, width, height);

  
  
  

// COLOR/SHADER =====================================================================

Scene scene;
Lambertian grey(color(.6, .6, .6));
Lambertian yellow(color(.9, .8, .1));
Lambertian green(color(.2, .8, .2));
Lambertian purple(color(.6, .2, .8));
ConstantColor temp(color(.2, .8, .2));
BlinnPhong red(color(.8, .1, .1), color(1, 1, 1), 64);
BlinnPhong blue(color(.1, .2, .8), color(1, 1, 1), 16);
BlinnPhong orange(color(.95, .5, .1), color(1, 1, 1), 32);

  

Normal normal;

Mirror redMirror(color(1, 0, 0));
Mirror blueMirror(color(.7, .7, 1));
Mirror mirror(color(1, 1, 1));

// SHAPES ==========================================================================
triangle(scene, point3(-50, 0, 50), point3(50, 0, 50), point3(50, 0, -50), grey); // ground 1
triangle(scene, point3(-50, 0, 50), point3(50, 0, -50), point3(-50, 0, -50), grey); // ground 2
triangle(scene, point3(-4, 0, -6), point3(4, 0, -6), point3(4, 4, -6), mirror); // wall 1
triangle(scene, point3(-4, 0, -6), point3(4, 4, -6), point3(-4, 4, -6), mirror); // wall 2
triangle(scene, point3(.9, 1.5, -0.6), point3(1.9, 1.4, -0.6), point3(1.4, 2.2, -1.1), yellow);

sphere(scene, point3(-2, 0.8, -3), .8, green);
sphere(scene, point3(-1.1, 0.6, -1.2), 0.6, red);
sphere(scene, point3(0, 0.9, -3.5), 0.9, normal);
sphere(scene, point3(0.3, 0.45, -1.6), .45, blue);
sphere(scene, point3(2, 1, -3), 1, redMirror);
sphere(scene, point3(1.5, 0.5, 0.3), .5, blueMirror);
sphere(scene, point3(-1.3, 0.45, 0.4), 0.45, temp);
sphere(scene, point3(0.3, 1, 5), 1, orange);
sphere(scene, point3(2.4, 0.8, 4.8), 0.8, purple);

scene.lights.emplace_back(point3(-5, 8, 5), vec3(.7, .6, .45));
scene.lights.emplace_back(point3(6, 6, 2), vec3(.35, .45, .7));

    // MAIN LOOP =======================================================================

    // divide each pixel into grid: samplesPerAxis x samplesPerAxis
    const int samplesPerAxis = 4;
    const int samplesPerPixel = samplesPerAxis * samplesPerAxis;
    const double pixelSamplesScale = 1.0 / samplesPerPixel;


    std::clog << "\033[?25l"; // lock the cursor because it was being a stupid idiot. DO NOT TOUCH THIS LINE.
    #pragma omp parallel for schedule(dynamic, 1)
    for (int y = 0; y < height; ++y) {
        std::mt19937 generator(
            static_cast<std::mt19937::result_type>(y) + 1u
        );
        std::uniform_real_distribution<double> distribution(0.0, 1.0);

        auto random_double = [&]() {
            return distribution(generator);
        };

        int percentage = (((height - y) * 100) / height);
        
        if (omp_get_thread_num() == 0) {
            std::clog << "\rpercentage: [%" << percentage << "]   " << std::flush;
        }

        for (int x = 0; x < width; ++x) {
            
            color pixelColor(0, 0, 0);

            for (int p = 0; p < samplesPerAxis; ++p) {
                for (int q = 0; q < samplesPerAxis; ++q) {
                    // One random sample inside each grid cell.
                    // Subtract 0.5 because generateRay already adds 0.5.
                    double offsetX =
                        (p + random_double()) / samplesPerAxis - 0.5;
                    double offsetY =
                        (q + random_double()) / samplesPerAxis - 0.5;

                    ray r = cam.generateRay(
                        x + offsetX,
                        y + offsetY
                    );

                    pixelColor += scene.computeRayColor(r, 4);
                }
            }

            fb.setPixelColor(
                x, y,
                pixelSamplesScale * pixelColor
            );
        }
    }
    fb.exportToPNG("output.png");
    std::cout << "\nSaved output.png\n";
    return 0;
}

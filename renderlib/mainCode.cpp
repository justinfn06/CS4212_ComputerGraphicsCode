#include <cmath>
#include <iostream>
#include <memory>
#include <vector>
#include <random>
#include <fstream>
#include <string>
#include <chrono>

#include "loading/SceneLoader.h"
#include "loading/SceneParser_JSON.h"
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
    scene.shapes.push_back(std::make_shared<Triangle>(a, b, c, material));
}

// helper function to add new sphere
void sphere(Scene& scene, point3 center, float radius, Shader& material) {
    scene.shapes.push_back(std::make_shared<Sphere>(center, radius, material));
}


// entry point for code.
int main(int argc, char* argv[]) {
    if (argc <2 || argc > 3) {
        std::cerr << "Usage: mainCode.exe <scene.json> [output.png]\n";
        return 1;
    }

    const std::string inputFilename = argv[1];
    const std::string outputFilename = argc == 3 ? argv[2] : "output.png";

    std::ifstream input(inputFilename);
    json sceneFile;
    input >> sceneFile;

    const auto& sceneData =
        sceneFile.contains("scene") ? sceneFile["scene"] : sceneFile;

    int width = 1000;
    int height = 1000;

    if (sceneData.contains("image")) {
        width = sceneData["image"].value("width", 1000)*4;
        height = sceneData["image"].value("height", 1000)*4;
    }

    Scene scene;

    auto loader = std::make_shared<SceneLoader>(scene, width, height);

    SceneParser_JSON parser(loader);
    parser.parseFileData(inputFilename);
    scene.buildBVH();

    std::cout
        << "Loaded " << scene.shapes.size() << " shapes, "
        << scene.shaders.size() << " shaders, and "
        << scene.lights.size() << " lights.\n";

    Framebuffer fb(width, height);
    PerspectiveCamera& cam = *scene.camera;

    // MAIN LOOP =======================================================================

    // divide each pixel into grid: samplesPerAxis x samplesPerAxis
    // this is used for antialiasing
    const int samplesPerAxis = 4;
    const int samplesPerPixel = samplesPerAxis * samplesPerAxis;
    const double pixelSamplesScale = 1.0 / samplesPerPixel;

    const auto renderStart = std::chrono::steady_clock::now();
    int completedRows = 0;
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

        for (int x = 0; x < width; ++x) {
            
            color pixelColor(0, 0, 0);

            for (int p = 0; p < samplesPerAxis; ++p) {
                for (int q = 0; q < samplesPerAxis; ++q) {
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

        // eta calculations
        #pragma omp critical
        {
            ++completedRows;

            const int percentage = completedRows * 100 / height;

            const double elapsed =
                std::chrono::duration<double>(
                    std::chrono::steady_clock::now() - renderStart
                ).count();

            const double secondsPerRow = elapsed / completedRows;

            const int remainingSeconds = static_cast<int>(
                std::ceil(secondsPerRow * (height - completedRows))
            );

            std::clog
                << "\rProgress: " << percentage << "%"
                << " | ETA: " << remainingSeconds / 60 << "m "
                << remainingSeconds % 60 << "s"
                << "          "
                << std::flush;
        }
    }
    fb.exportToPNG(outputFilename);
    std::cout << "\nSaved " << outputFilename << '\n';
    return 0;
}

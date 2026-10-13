#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <vector>
#include <random>
#include <fstream>
#include <string>
#include "triangleList.h"
#include "Framebuffer.h"
#include "perspectiveCamera.h"
#include "sphere.h"
#include "triangle.h"
#include "lambertian.h"
#include "blinnPhong.h"
#include "color.h"
#include "ray.h"

// pasted code from github
void readFloatsFromFile(const std::string& filename, std::vector<float> &allFloats)
{
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Failed to open file\n";
        return;
    }

    float value;
    char c;

    while (file >> value) {
        allFloats.push_back(value);

        if (file.peek() == ',') file.get(c);
    }
}

using Scene = std::vector<std::unique_ptr<Shape>>; // scene holds all shapes

// helper function to add new triangle
void triangle(Scene& scene, point3 a, point3 b, point3 c, Shader& material) {
    scene.push_back(std::make_unique<Triangle>(a, b, c, material));
}

// main ray tracing function
// returns the color of pixel based on ray intersection with scene objects
color computeRayColor(const ray& r, const Scene& scene) {
    HitStruct h;
    float tmin = 0.001f;
    float tmax = std::numeric_limits<float>::max();
    float localTmax = tmax; // so we can change it

    color bgColor = color(0.9, .9, .95);

    bool hitShape = false;
    for (int idx = 0; idx < scene.size(); ++idx) {
        if (scene[idx]->intersect(r, tmin, localTmax, h)) {
            hitShape = true;
        }
    }

    if (hitShape) {
        // use shader belonging to the closest shape
        color c = h.shape->shade(h);
        return c;
    } else {
        return bgColor;
    }
}


// entry point for code.
int main() {
    const int width = 600;
    const int height = 600;
    Framebuffer fb(width, height);
    PerspectiveCamera cam(vec3(0, 0, 0), vec3(0, 0, -1), 0.9f, .9f, 0.9f, width, height);

    BlinnPhong blue(color(0.04, 0.42, 0.85), color(0.7, 0.8, 1.0), 80.0);
    Lambertian red(color(1.0, .8, .8));
    Scene scene;

    std::vector<float> allFloats;
    readFloatsFromFile("C:/Justin/School/UMD Computer Graphics/general repo/CS4212_ComputerGraphicsCode/renderlib/trilist.dat", allFloats);

    std::cout << "Triangles: " << allFloats.size() / 9 << '\n';

    scene.push_back(std::make_unique<TriangleList>(allFloats, blue, 1.75, vec3(0, 0, -5)));
    
    #pragma omp parallel for schedule(dynamic, 1)
    for (int y = 0; y < height; ++y) {
        std::clog << "\rScanlines remaining: " << (height - y) << ' ' << std::flush;
        for (int x = 0; x < width; ++x)
            fb.setPixelColor(x, y, computeRayColor(cam.generateRay(x, y), scene));
    }
    fb.exportToPNG("output.png");
    std::cout << "\nSaved output.png\n";
    return 0;
}

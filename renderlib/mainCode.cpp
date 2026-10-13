#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <vector>
#include <random>

#include "Framebuffer.h"
#include "perspectiveCamera.h"
#include "sphere.h"
#include "triangle.h"
#include "lambertian.h"
#include "blinnPhong.h"
#include "color.h"
#include "ray.h"

double pi = 3.14159265358979323846; // pi. yum.
using Scene = std::vector<std::unique_ptr<Shape>>; // scene holds all shapes

// helper function to add new triangle
void triangle(Scene& scene, point3 a, point3 b, point3 c, Shader& material) {
    scene.push_back(std::make_unique<Triangle>(a, b, c, material));
}

// helper function to add new sphere
void sphere(Scene& scene, point3 center, float radius, Shader& material) {
    scene.push_back(std::make_unique<Sphere>(center, radius, material));
}

// main ray tracing function
// returns the color of pixel based on ray intersection with scene objects
color computeRayColor(const ray& r, const Scene& scene) {
    HitStruct h;
    float tmin = 0.001f;
    float tmax = std::numeric_limits<float>::max();
    float localTmax = tmax; // so we can change it

    color bgColor = color(0.009, 0.014, 0.045);

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
    // 1080 * 810
    const int width = 1080;
    const int height = 810;
    Framebuffer fb(width, height);
    PerspectiveCamera cam(vec3(0, 0, 0), vec3(0, 0, -1),
                          0.9f, 1.2f, 0.9f, width, height);

    // definitions of colors for easier calling in the future.
    Lambertian orange(color(0.69, 0.42, 0.23));
    Lambertian ringLight(color(0.85, 0.66, 0.40));
    Lambertian ringDark(color(0.47, 0.30, 0.22));
    BlinnPhong water(color(0.22, 0.42, 0.57), color(.5, .5, .5), 512);
    Lambertian star(color(0.63, 0.78, 1.0));
    Lambertian warmStar(color(1.0, 0.78, 0.43));
    Lambertian red(color(.5, .2, .2));
    Lambertian green(color(.2, .4, .2));
    Lambertian rocketWhite(color(0.88, 0.88, 0.82));
    Lambertian rocketRed(color(0.85, 0.08, 0.05));
    Lambertian rocketFlame(color(1.0, 0.55, 0.08));
    BlinnPhong rocketGlass(color(0.04, 0.42, 0.85), color(0.7, 0.8, 1.0), 80.0);
    Scene scene;

    // planet and rings

    const int numBands = 3; // control the number of bands
    const double planetSize = 2.0f; // size of planet
    const point3 planetCenter(-2.2, 2.6, -16.0); // planet position
    const double distanceFromPlanet = 2.5; // distance interior ring starts from the planet edge
    const double neighboringDistance = .3f; // distance between neighboring rings
    const double bandWidth = .25f; // the width o5 each individual ring
    const int darkRing = 1;
    const double pitchDegrees = 120.0; // the rings pitch in degrees
    const double yawDegrees = 40.0; // the rings yaw in degrees
    const int sections = 64; // sections to reach full ring circle. higher = smoother. (must be multiple of 4)


    const double pitch = pitchDegrees * pi / 180.0;
    const double yaw = yawDegrees * pi / 180.0;
    sphere(scene, planetCenter, planetSize, orange); // planet sphere
    const vec3 ringU(std::cos(yaw), 0.0, -std::sin(yaw));
    const vec3 ringV(std::sin(yaw) * std::sin(pitch), std::cos(pitch), std::cos(yaw) * std::sin(pitch));

    // loops over ring logic for each individual band (currently 3)
    for (int band = 0; band < numBands; ++band) {
        double inner = distanceFromPlanet + band * neighboringDistance;
        double outer = inner + bandWidth;
        Shader& material = band == darkRing ? static_cast<Shader&>(ringDark) : ringLight; // determine which ring number is what color
        auto ringPoint = [&](double radius, double angle) {
            return planetCenter + radius * (std::cos(angle) * ringU + std::sin(angle) * ringV);
        };

        // 3 bands * 64 sections * 2 triangles = 384 triangles
        for (int i = 0; i < sections; ++i) {
            double a = 2 * pi * i / sections, b = 2 * pi * (i + 1) / sections;
            triangle(scene, ringPoint(inner, a), ringPoint(inner, b), ringPoint(outer, a), material);
            triangle(scene, ringPoint(outer, a), ringPoint(inner, b), ringPoint(outer, b), material);
        }
    }
    
    sphere(scene, point3(-50.5, 25, -80), 40.0f, red); // big red

    
    sphere(scene, point3(-3.8, -3, -10), 1.2f, water); // water
    
    sphere(scene, point3(-3, -3.2, -8), .2f, green); // green
    sphere(scene, point3(-2, -3.2, -10), .2f, green); // green
    sphere(scene, point3(-6.3, -3.2, -12), .2f, green); // green
    


    // stars
    std::mt19937 generator(1);
    std::uniform_real_distribution<double> randomX(-70.0, 70.0);
    std::uniform_real_distribution<double> randomY(-50.0, 50.0);
    std::uniform_real_distribution<float> randomRadius(0.05f, 0.25f);
    std::bernoulli_distribution isWarmStar(0.5); // % chance of warm star
    const int numberStars = 100; // the number of stars to generate


    for (int i = 0; i < numberStars; ++i) {
        double x = randomX(generator);
        double y = randomY(generator);
        float radius = randomRadius(generator);

        Shader& material = isWarmStar(generator) ? warmStar : star;

        sphere(scene, point3(x, y, -100.0), radius, material);
    }


    // rocket
    const float rocketSize = 0.5f;
    const point3 rocketCenter(3.0, 0.0, -11.0); // moves the entire rocket around

    // conversion from degrees
    const double pitchR = 0.0 * pi / 180.0;   // tilt towards camera
    const double yawR   = 5.0 * pi / 180.0;   // turn to show sides
    const double rollR  = -35.0 * pi / 180.0; // lean left/right

    auto rocketPoint = [&](double x, double y, double z) {
        // pitch
        double y1 = y * std::cos(pitchR) - z * std::sin(pitchR);
        double z1 = y * std::sin(pitchR) + z * std::cos(pitchR);

        // yaw
        double x2 = x * std::cos(yawR) + z1 * std::sin(yawR);
        double z2 = -x * std::sin(yawR) + z1 * std::cos(yawR);

        // roll
        double x3 = x2 * std::cos(rollR) - y1 * std::sin(rollR);
        double y3 = x2 * std::sin(rollR) + y1 * std::cos(rollR);

        // Scale the rotated local point, then position the rocket.
        return rocketCenter + rocketSize * vec3(x3, y3, z2);
    };
    // four corners in two triangles to make a square (a and c are shared by both but b and d are unique)
    auto rocketFace = [&](point3 a, point3 b, point3 c, point3 d) {
        triangle(scene, a, b, c, rocketWhite);
        triangle(scene, a, c, d, rocketWhite);
    };

    // box corners: front/back, bottom/top, left/right.
    point3 frontBottomRight = rocketPoint( 1, -1,  0.4);
    point3 frontTopRight    = rocketPoint( 1,  1,  0.4);
    point3 frontTopLeft     = rocketPoint(-1,  1,  0.4);
    point3 frontBottomLeft  = rocketPoint(-1, -1,  0.4);
    point3 backBottomLeft   = rocketPoint(-1, -1, -0.4);
    point3 backBottomRight  = rocketPoint( 1, -1, -0.4);
    point3 backTopRight     = rocketPoint( 1,  1, -0.4);
    point3 backTopLeft      = rocketPoint(-1,  1, -0.4);

    rocketFace(frontBottomLeft, frontBottomRight, frontTopRight, frontTopLeft);
    rocketFace(backBottomRight, backBottomLeft, backTopLeft, backTopRight);
    rocketFace(backBottomLeft, frontBottomLeft, frontTopLeft, backTopLeft);
    rocketFace(frontBottomRight, backBottomRight, backTopRight, frontTopRight);
    rocketFace(frontTopLeft, frontTopRight, backTopRight, backTopLeft);
    rocketFace(backBottomLeft, backBottomRight, frontBottomRight, frontBottomLeft);

    // porthole (embeded sphere)
    sphere(scene, rocketPoint(0, 0.16, 0.1), 0.5f * rocketSize, rocketGlass);

    // four triangles for nose
    point3 noseTip = rocketPoint(0, 2.4, 0);
    triangle(scene, frontTopLeft, frontTopRight, noseTip, rocketRed);
    triangle(scene, frontTopRight, backTopRight, noseTip, rocketRed);
    triangle(scene, backTopRight, backTopLeft, noseTip, rocketRed);
    triangle(scene, backTopLeft, frontTopLeft, noseTip, rocketRed);

    // four triangles for fins
    triangle(scene, rocketPoint(-1.0, -0.13, 0), rocketPoint(-1.8, -1.4, 0), rocketPoint(-1.0, -1.0, 0), rocketRed);
    triangle(scene, rocketPoint(1.0, -0.13, 0), rocketPoint(1.0, -1.0, 0), rocketPoint(1.8, -1.4, 0), rocketRed);
    triangle(scene, rocketPoint(0, -0.13, 0.4), rocketPoint(0, -1.0, 0.4), rocketPoint(0, -1.4, 1.2), rocketRed);
    triangle(scene, rocketPoint(0, -0.13, -0.4), rocketPoint(0, -1.4, -1.2), rocketPoint(0, -1.0, -0.4), rocketRed);

    // orange triangle for flame
    triangle(scene, rocketPoint(-0.43, -1.0, 0), rocketPoint(0, -2.87, 0), rocketPoint(0.43, -1.0, 0), rocketFlame);
    
    


    for (int y = 0; y < height; ++y) {
        std::clog << "\rScanlines remaining: " << (height - y) << ' ' << std::flush;
        for (int x = 0; x < width; ++x)
            fb.setPixelColor(x, y, computeRayColor(cam.generateRay(x, y), scene));
    }
    fb.exportToPNG("output.png");
    std::cout << "\nSaved output.png\n";
    return 0;
}

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <vector>
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
        const int width = 1200*2;
    const int height = 900*2;
    Framebuffer fb(width, height);

    const point3 eye(1.2, 1.8, 10);
    const point3 target(-0.3, 2.6, -23);

    PerspectiveCamera cam(
        eye, target - eye,
        0.65f, 0.82f, 0.615f,
        width, height
    );

    Scene scene;

    // ============================================================
    // MATERIALS
    // ============================================================

    Lambertian asphalt(color(0.12, 0.14, 0.18));
    Lambertian concrete(color(0.48, 0.51, 0.57));
    Lambertian curb(color(0.67, 0.68, 0.68));

    Lambertian cream(color(0.76, 0.70, 0.58));
    Lambertian terracotta(color(0.65, 0.27, 0.19));
    Lambertian sage(color(0.34, 0.49, 0.39));
    Lambertian ochre(color(0.73, 0.51, 0.24));
    Lambertian dustyBlue(color(0.30, 0.42, 0.56));
    Lambertian rose(color(0.63, 0.35, 0.34));

    Lambertian trim(color(0.79, 0.77, 0.69));
    Lambertian charcoal(color(0.055, 0.065, 0.085));
    Lambertian wood(color(0.28, 0.14, 0.075));
    Lambertian leafGreen(color(0.13, 0.39, 0.15));
    Lambertian leafLight(color(0.24, 0.49, 0.18));
    Lambertian awningRed(color(0.62, 0.12, 0.11));
    Lambertian roadPaint(color(0.74, 0.70, 0.54));

    // Glazed ceramic planters.
    BlinnPhong planter(
        color(0.48, 0.22, 0.12),
        color(0.45, 0.32, 0.23),
        24
    );

    // Dark blue painted metal.
    BlinnPhong lampMetal(
        color(0.045, 0.075, 0.13),
        color(0.55, 0.62, 0.75),
        80
    );

    // Glossy red cafe doors.
    BlinnPhong lacquerRed(
        color(0.48, 0.035, 0.045),
        color(0.75, 0.55, 0.50),
        64
    );

    Mirror windowGlass(color(0.62, 0.73, 0.83));
    Mirror shopGlass(color(0.76, 0.80, 0.84));
    Mirror chrome(color(0.87, 0.89, 0.93));

    Normal sculptureNormal;

    ConstantColor lampGlow(color(1.0, 0.76, 0.37));
    ConstantColor duskSky(color(0.075, 0.095, 0.18));

    // ============================================================
    // GEOMETRY HELPERS
    // ============================================================

    auto quad = [&](point3 a, point3 b, point3 c, point3 d,
                    Shader& mat) {
        triangle(scene, a, b, c, mat);
        triangle(scene, a, c, d, mat);
    };

    auto box = [&](point3 lo, point3 hi, Shader& mat) {
        double x0 = lo.x(), y0 = lo.y(), z0 = lo.z();
        double x1 = hi.x(), y1 = hi.y(), z1 = hi.z();

        // Top.
        quad(point3(x0,y1,z0), point3(x0,y1,z1),
             point3(x1,y1,z1), point3(x1,y1,z0), mat);

        // Bottom.
        quad(point3(x0,y0,z1), point3(x0,y0,z0),
             point3(x1,y0,z0), point3(x1,y0,z1), mat);

        // Front.
        quad(point3(x0,y0,z1), point3(x1,y0,z1),
             point3(x1,y1,z1), point3(x0,y1,z1), mat);

        // Back.
        quad(point3(x1,y0,z0), point3(x0,y0,z0),
             point3(x0,y1,z0), point3(x1,y1,z0), mat);

        // Left.
        quad(point3(x0,y0,z0), point3(x0,y0,z1),
             point3(x0,y1,z1), point3(x0,y1,z0), mat);

        // Right.
        quad(point3(x1,y0,z1), point3(x1,y0,z0),
             point3(x1,y1,z0), point3(x1,y1,z1), mat);
    };

    // Local facade coordinates.
    auto facadePoint = [&](point3 origin, vec3 along, vec3 outward,
                           double u, double y, double depth) {
        return origin
            + along * u
            + vec3(0, y, 0)
            + outward * depth;
    };

    auto panel = [&](point3 origin, vec3 along, vec3 outward,
                     double u, double y, double w, double h,
                     double depth, Shader& mat) {
        point3 a = facadePoint(
            origin, along, outward, u - w / 2, y, depth
        );
        point3 b = facadePoint(
            origin, along, outward, u + w / 2, y, depth
        );

        quad(a, b, b + vec3(0, h, 0), a + vec3(0, h, 0), mat);
    };

    // ============================================================
    // T-SHAPED STREET
    // ============================================================

    box(point3(-35, -0.30, -43),
        point3( 35, -0.04,  20), concrete);

    // Main road.
    box(point3(-3.2, -0.035, -25),
        point3( 3.2,  0.000,  18), asphalt);

    // Cross street meets the main road without overlapping it.
    box(point3(-35, -0.035, -31.5),
        point3( 35,  0.000, -25.0), asphalt);

    // Sidewalks.
    box(point3(-12, 0, -25),
        point3(-3.2, 0.16, 18), concrete);

    box(point3(3.2, 0, -25),
        point3(12, 0.16, 18), concrete);

    box(point3(-35, 0, -43),
        point3( 35, 0.16, -31.5), concrete);

    // Main street curbs.
    box(point3(-3.38, 0, -25),
        point3(-3.20, 0.19, 18), curb);

    box(point3(3.20, 0, -25),
        point3(3.38, 0.19, 18), curb);

    // Cross street curbs.
    box(point3(-35, 0, -31.7),
        point3( 35, 0.19, -31.5), curb);

    box(point3(-35, 0, -25.0),
        point3(-3.2, 0.19, -24.8), curb);

    box(point3(3.2, 0, -25.0),
        point3(35, 0.19, -24.8), curb);

    // Dashed main road line, ending before the crossing.
    for (int i = 0; i < 7; ++i) {
        double z = 8.0 - 4.5 * i;

        box(point3(-0.05, 0.002, z - 1.0),
            point3( 0.05, 0.008, z + 1.0), roadPaint);
    }

    // Cross street markings.
    for (int i = -7; i <= 7; ++i) {
        double x = i * 4.5;

        box(point3(x - 1.0, 0.002, -28.30),
            point3(x + 1.0, 0.008, -28.20), roadPaint);
    }

    // Pedestrian crossing.
    for (int i = 0; i < 8; ++i) {
        double x = -2.95 + i * 0.77;

        box(point3(x,        0.002, -24.0),
            point3(x + 0.40, 0.009, -22.5), trim);
    }

    box(point3(-2.95, 0.002, -21.7),
        point3( 2.95, 0.009, -21.5), trim);

    // ============================================================
    // DETAILED BUILDING HELPER
    // ============================================================

    auto building = [&](point3 origin, vec3 along, vec3 outward,
                        double frontage, double depth,
                        int floors, Shader& wall, bool cafe) {
        const double base = 0.16;
        const double floorHeight = 2.65;
        const double roofY = base + floors * floorHeight;

        point3 corners[4] = {
            origin - along * (frontage / 2),
            origin + along * (frontage / 2),
            origin - along * (frontage / 2) - outward * depth,
            origin + along * (frontage / 2) - outward * depth
        };

        double xmin = corners[0].x(), xmax = corners[0].x();
        double zmin = corners[0].z(), zmax = corners[0].z();

        for (int i = 1; i < 4; ++i) {
            xmin = std::min(xmin, corners[i].x());
            xmax = std::max(xmax, corners[i].x());
            zmin = std::min(zmin, corners[i].z());
            zmax = std::max(zmax, corners[i].z());
        }

        box(point3(xmin, base, zmin),
            point3(xmax, roofY, zmax), wall);

        auto face = [&](double u, double y, double w, double h,
                        double offset, Shader& mat) {
            panel(origin, along, outward,
                  u, y, w, h, offset, mat);
        };

        // Stone base molding.
        face(0, base, frontage, 0.30, 0.025, charcoal);

        // Horizontal trim between floors.
        for (int floor = 1; floor < floors; ++floor) {
            face(
                0, base + floor * floorHeight - 0.10,
                frontage, 0.16, 0.035, trim
            );
        }

        // Projecting roof cornice.
        point3 ca = origin
            - along * (frontage / 2 + 0.14)
            - outward * 0.10;
        point3 cb = origin
            + along * (frontage / 2 + 0.14)
            + outward * 0.28;

        box(
            point3(std::min(ca.x(), cb.x()), roofY - 0.12,
                   std::min(ca.z(), cb.z())),
            point3(std::max(ca.x(), cb.x()), roofY + 0.14,
                   std::max(ca.z(), cb.z())),
            trim
        );

        int columns = (frontage > 12.0) ? 6 : 3;
        double spacing = frontage / columns;

        // Upper-floor reflective windows.
        for (int floor = 1; floor < floors; ++floor) {
            double y = base + floor * floorHeight + 0.44;

            for (int col = 0; col < columns; ++col) {
                double u = -frontage / 2
                    + spacing * (col + 0.5);

                double w = spacing * 0.54;
                double h = 1.46;

                // Pale frame.
                face(u, y - 0.09, w + 0.20, h + 0.18,
                     0.040, trim);

                // Reflective pane.
                face(u, y, w, h, 0.055, windowGlass);

                // Vertical and horizontal mullions.
                face(u, y, 0.045, h, 0.070, charcoal);
                face(u, y + 0.76, w, 0.045, 0.075, charcoal);

                // Window sill.
                face(u, y - 0.15, w + 0.32, 0.12,
                     0.090, trim);
            }
        }

        // Ground-floor storefront windows.
        for (int col = 0; col < columns; ++col) {
            double u = -frontage / 2
                + spacing * (col + 0.5);

            face(u, base + 0.22, spacing * 0.80, 1.98,
                 0.040, charcoal);

            face(u, base + 0.36, spacing * 0.68, 1.66,
                 0.055, shopGlass);

            face(u, base + 0.36, 0.055, 1.66,
                 0.075, trim);
        }

        // Glossy cafe doors; wooden doors elsewhere.
        Shader& doorMaterial = cafe
            ? static_cast<Shader&>(lacquerRed)
            : static_cast<Shader&>(wood);

        face(0, base + 0.02, 0.88, 2.12,
             0.090, doorMaterial);

        face(0, base + 0.85, 0.62, 1.02,
             0.105, shopGlass);

        sphere(
            scene,
            facadePoint(origin, along, outward,
                        0.29, 1.08, 0.15),
            0.045f, chrome
        );

        // Blank shop sign.
        face(0, 2.40, frontage * 0.84, 0.30, 0.10, trim);
        face(0, 2.46, frontage * 0.78, 0.18, 0.12,
             cafe ? awningRed : charcoal);

        if (cafe) {
            // Red and cream striped awning.
            for (int stripe = 0; stripe < 12; ++stripe) {
                double ua = -frontage / 2 + 0.25
                    + stripe * (frontage - 0.5) / 12.0;
                double ub = -frontage / 2 + 0.25
                    + (stripe + 1) * (frontage - 0.5) / 12.0;

                Shader& mat = (stripe % 2 == 0)
                    ? static_cast<Shader&>(awningRed)
                    : static_cast<Shader&>(trim);

                point3 a = facadePoint(
                    origin, along, outward, ua, 2.34, 1.00
                );
                point3 b = facadePoint(
                    origin, along, outward, ub, 2.34, 1.00
                );
                point3 c = facadePoint(
                    origin, along, outward, ub, 2.67, 0.16
                );
                point3 d = facadePoint(
                    origin, along, outward, ua, 2.67, 0.16
                );

                quad(a, b, c, d, mat);

                face((ua + ub) / 2, 2.18, ub - ua,
                     0.16, 1.00, mat);
            }
        }

        // Rooftop chimney.
        point3 chimney = origin
            + along * (frontage * 0.28)
            - outward * 1.6;

        box(
            point3(chimney.x() - 0.25, roofY,
                   chimney.z() - 0.28),
            point3(chimney.x() + 0.25, roofY + 0.80,
                   chimney.z() + 0.28),
            wall
        );
    };

    // ============================================================
    // BUILDINGS ALONG THE MAIN STREET
    // ============================================================

    building(point3(-5.5, 0, 5.0),
             vec3(0,0,-1), vec3(1,0,0),
             9.8, 6.0, 3, terracotta, true);

    building(point3(-5.5, 0, -5.0),
             vec3(0,0,-1), vec3(1,0,0),
             9.8, 6.0, 4, sage, false);

    building(point3(-5.5, 0, -15.0),
             vec3(0,0,-1), vec3(1,0,0),
             9.8, 6.0, 3, ochre, false);

    building(point3(-5.5, 0, -22.3),
             vec3(0,0,-1), vec3(1,0,0),
             4.4, 6.0, 4, dustyBlue, false);

    building(point3(5.5, 0, 5.0),
             vec3(0,0,1), vec3(-1,0,0),
             9.8, 6.0, 4, dustyBlue, false);

    building(point3(5.5, 0, -5.0),
             vec3(0,0,1), vec3(-1,0,0),
             9.8, 6.0, 3, cream, true);

    building(point3(5.5, 0, -15.0),
             vec3(0,0,1), vec3(-1,0,0),
             9.8, 6.0, 4, rose, false);

    building(point3(5.5, 0, -22.3),
             vec3(0,0,1), vec3(-1,0,0),
             4.4, 6.0, 3, sage, false);

    // ============================================================
    // BUILDINGS OPPOSITE THE T INTERSECTION
    // ============================================================

    building(point3(0, 0, -34.0),
             vec3(1,0,0), vec3(0,0,1),
             15.0, 7.0, 4, terracotta, true);

    building(point3(-13.0, 0, -34.0),
             vec3(1,0,0), vec3(0,0,1),
             10.6, 7.0, 3, cream, false);

    building(point3(13.0, 0, -34.0),
             vec3(1,0,0), vec3(0,0,1),
             10.6, 7.0, 5, sage, false);

    building(point3(-24.0, 0, -34.0),
             vec3(1,0,0), vec3(0,0,1),
             10.6, 7.0, 4, dustyBlue, false);

    building(point3(24.0, 0, -34.0),
             vec3(1,0,0), vec3(0,0,1),
             10.6, 7.0, 3, ochre, false);

    // ============================================================
    // TREES IN GLAZED CERAMIC PLANTERS
    // ============================================================

    auto tree = [&](double x, double z, bool lighter) {
        box(point3(x - 0.42, 0.16, z - 0.42),
            point3(x + 0.42, 0.48, z + 0.42), planter);

        // Slightly wider planter rim.
        box(point3(x - 0.46, 0.43, z - 0.46),
            point3(x + 0.46, 0.52, z + 0.46), planter);

        box(point3(x - 0.095, 0.52, z - 0.095),
            point3(x + 0.095, 2.05, z + 0.095), wood);

        Shader& leaves = lighter
            ? static_cast<Shader&>(leafLight)
            : static_cast<Shader&>(leafGreen);

        sphere(scene, point3(x, 2.65, z), 0.78f, leaves);
        sphere(scene, point3(x - 0.38, 2.38, z + 0.08),
               0.52f, leaves);
        sphere(scene, point3(x + 0.34, 2.40, z - 0.15),
               0.54f, leaves);
    };

    tree(-4.25,   1.0, false);
    tree( 4.25,  -3.0, true);
    tree(-4.25,  -9.0, true);
    tree( 4.25, -13.0, false);
    tree(-4.25, -19.0, false);
    tree( 4.25, -21.0, true);

    tree(-8.0, -32.6, true);
    tree( 8.0, -32.6, false);

    // ============================================================
    // BENCHES
    // ============================================================

    auto bench = [&](double x, double z, int side) {
        box(point3(x - 0.27, 0.60, z - 0.85),
            point3(x + 0.27, 0.72, z + 0.85), wood);

        box(point3(x - 0.21, 0.16, z - 0.65),
            point3(x + 0.21, 0.60, z - 0.52), lampMetal);

        box(point3(x - 0.21, 0.16, z + 0.52),
            point3(x + 0.21, 0.60, z + 0.65), lampMetal);

        double backX = x + side * 0.25;

        box(point3(backX - 0.045, 0.72, z - 0.85),
            point3(backX + 0.045, 1.18, z + 0.85), wood);
    };

    bench(-4.35, -4.0, -1);
    bench( 4.35, -8.0,  1);


        // ============================================================
    // GLOSSY PAINTED FIRE HYDRANT — BLINN-PHONG DEMONSTRATION
    // ============================================================

    BlinnPhong hydrantRed(
        color(0.72, 0.035, 0.02),
        color(1.0, 0.95, 0.90),
        20
    );

    BlinnPhong hydrantMetal(
        color(0.28, 0.30, 0.34),
        color(0.85, 0.90, 1.0),
        40
    );

    // Near the curb, clear of the trees and benches.
    const double hx = 3.85;
    const double hz = 1.0;

    // Footing.
    box(
        point3(hx - 0.28, 0.16, hz - 0.28),
        point3(hx + 0.28, 0.27, hz + 0.28),
        hydrantMetal
    );

    // Painted upright.
    box(
        point3(hx - 0.16, 0.27, hz - 0.16),
        point3(hx + 0.16, 0.88, hz + 0.16),
        hydrantRed
    );

    // Rounded shoulder and domed cap catch broad highlights.
    sphere(
        scene, point3(hx, 0.86, hz),
        0.23f, hydrantRed
    );

    // Top operating nut.
    box(
        point3(hx - 0.065, 1.06, hz - 0.065),
        point3(hx + 0.065, 1.16, hz + 0.065),
        hydrantMetal
    );

    // Side outlets.
    box(
        point3(hx - 0.34, 0.57, hz - 0.10),
        point3(hx + 0.34, 0.76, hz + 0.10),
        hydrantRed
    );

    sphere(
        scene, point3(hx - 0.34, 0.665, hz),
        0.13f, hydrantRed
    );

    sphere(
        scene, point3(hx + 0.34, 0.665, hz),
        0.13f, hydrantRed
    );

    // Front outlet faces the camera.
    sphere(
        scene, point3(hx, 0.63, hz + 0.19),
        0.16f, hydrantRed
    );

    // Small metallic nut on its cap.
    box(
        point3(hx - 0.045, 0.585, hz + 0.32),
        point3(hx + 0.045, 0.675, hz + 0.37),
        hydrantMetal
    );

    // ============================================================
    // NORMAL-SHADED PUBLIC ART
    // ============================================================

    const double artZ = -32.7;

    box(
        point3(-1.05, 0.16, artZ - 0.55),
        point3( 1.05, 0.38, artZ + 0.55),
        charcoal
    );

    box(
        point3(-0.78, 0.38, artZ - 0.43),
        point3( 0.78, 1.05, artZ + 0.43),
        cream
    );

    box(
        point3(-0.88, 1.05, artZ - 0.49),
        point3( 0.88, 1.17, artZ + 0.49),
        trim
    );

    box(
        point3(-0.07, 1.17, artZ - 0.07),
        point3( 0.07, 1.53, artZ + 0.07),
        lampMetal
    );

    // Curved surface shows smoothly changing normal colors.
    sphere(
        scene,
        point3(0, 2.16, artZ),
        0.68f,
        sculptureNormal
    );

    // Flat fins show constant colors for each triangle normal.
    triangle(
        scene,
        point3(-0.72, 1.17, artZ + 0.10),
        point3(-0.33, 1.17, artZ + 0.36),
        point3(-0.98, 2.66, artZ - 0.16),
        sculptureNormal
    );

    triangle(
        scene,
        point3(0.33, 1.17, artZ + 0.36),
        point3(0.72, 1.17, artZ + 0.10),
        point3(0.98, 2.66, artZ + 0.26),
        sculptureNormal
    );

    // Reflective plaque.
    quad(
        point3(-0.30, 0.64, artZ + 0.435),
        point3( 0.30, 0.64, artZ + 0.435),
        point3( 0.30, 0.84, artZ + 0.435),
        point3(-0.30, 0.84, artZ + 0.435),
        chrome
    );

    // ============================================================
    // STREET LIGHTS
    // ============================================================

    auto streetLamp = [&](double x, double z, int side) {
        double bulbX = x - side * 0.65;

        box(point3(x - 0.14, 0.16, z - 0.14),
            point3(x + 0.14, 0.38, z + 0.14), lampMetal);

        box(point3(x - 0.05, 0.38, z - 0.05),
            point3(x + 0.05, 4.15, z + 0.05), lampMetal);

        box(
            point3(std::min(x, bulbX) - 0.05, 4.04, z - 0.05),
            point3(std::max(x, bulbX) + 0.05, 4.15, z + 0.05),
            lampMetal
        );

        box(point3(bulbX - 0.22, 3.96, z - 0.22),
            point3(bulbX + 0.22, 4.05, z + 0.22), lampMetal);

        sphere(
            scene, point3(bulbX, 3.83, z),
            0.14f, lampGlow
        );

        scene.lights.emplace_back(
            point3(bulbX, 3.60, z),
            color(0.11, 0.065, 0.025)
        );
    };

    streetLamp(-3.65,   4.0, -1);
    streetLamp( 3.65,  -3.0,  1);
    streetLamp(-3.65, -11.0, -1);
    streetLamp( 3.65, -18.0,  1);

    // Opposite-sidewalk lamp.
    box(point3(-2.45, 0.16, -32.7),
        point3(-2.35, 4.05, -32.6), lampMetal);

    box(point3(-2.65, 3.96, -32.9),
        point3(-2.15, 4.08, -32.4), lampMetal);

    sphere(
        scene, point3(-2.40, 3.81, -32.65),
        0.15f, lampGlow
    );

    scene.lights.emplace_back(
        point3(-2.40, 3.57, -32.65),
        color(0.11, 0.065, 0.025)
    );

    // Glossy painted bollards with reflective caps.
    for (int side = -1; side <= 1; side += 2) {
        double x = side * 3.65;

        box(point3(x - 0.065, 0.16, -23.4),
            point3(x + 0.065, 0.77, -23.27), lampMetal);

        sphere(
            scene, point3(x, 0.80, -23.335),
            0.09f, chrome
        );
    }

    // ============================================================
    // BLUE-HOUR LIGHTING
    // ============================================================

    scene.lights.emplace_back(
        point3(-25, 45, -15),
        color(0.16, 0.21, 0.36)
    );

    scene.lights.emplace_back(
        point3(20, 30, 15),
        color(0.075, 0.105, 0.18)
    );

    scene.lights.emplace_back(
        point3(-18, 16, -24),
        color(0.10, 0.055, 0.045)
    );

    // ============================================================
    // DUSK BACKDROP
    // Hides the default daytime background without changing Scene.
    // Open overhead so it doesn't block the scene lights.
    // ============================================================

    const double skyExtent = 300.0;
    const double skyBottom = -50.0;
    const double skyTop = 200.0;

    quad(
        point3(-skyExtent, skyBottom, -skyExtent),
        point3( skyExtent, skyBottom, -skyExtent),
        point3( skyExtent, skyTop,    -skyExtent),
        point3(-skyExtent, skyTop,    -skyExtent),
        duskSky
    );

    quad(
        point3( skyExtent, skyBottom, skyExtent),
        point3(-skyExtent, skyBottom, skyExtent),
        point3(-skyExtent, skyTop,    skyExtent),
        point3( skyExtent, skyTop,    skyExtent),
        duskSky
    );

    quad(
        point3(-skyExtent, skyBottom,  skyExtent),
        point3(-skyExtent, skyBottom, -skyExtent),
        point3(-skyExtent, skyTop,    -skyExtent),
        point3(-skyExtent, skyTop,     skyExtent),
        duskSky
    );

    quad(
        point3(skyExtent, skyBottom, -skyExtent),
        point3(skyExtent, skyBottom,  skyExtent),
        point3(skyExtent, skyTop,     skyExtent),
        point3(skyExtent, skyTop,    -skyExtent),
        duskSky
    );






    // MAIN LOOP =======================================================================

    // divide each pixel into grid: samplesPerAxis x samplesPerAxis
    const int samplesPerAxis = 4;
    const int samplesPerPixel = samplesPerAxis * samplesPerAxis;
    const double pixelSamplesScale = 1.0 / samplesPerPixel;


    #pragma omp parallel for schedule(dynamic, 1)
    for (int y = 0; y < height; ++y) {
        std::mt19937 generator(
            static_cast<std::mt19937::result_type>(y) + 1u
        );
        std::uniform_real_distribution<double> distribution(0.0, 1.0);

        auto random_double = [&]() {
            return distribution(generator);
        };

        std::clog << "\rScanlines remaining: "
                  << (height - y) << ' ' << std::flush;

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

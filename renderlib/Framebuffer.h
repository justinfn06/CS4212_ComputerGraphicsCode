#pragma once

#include <vector>
#include "vec3.h"
#include "color.h"

class Framebuffer {
    public:
    Framebuffer();
    Framebuffer(int width, int height);

    void clearToColor(const color& c);
    void clearToGradient(const color& c1, const color& c2);

    void setPixelColor(int i, int j, const color& c);
    void setPixelColor(int idx, const color& c);

    void exportToPNG(const std::string& filename);

    private:
    int width, height;
    std::vector<color> fbStorage;
};
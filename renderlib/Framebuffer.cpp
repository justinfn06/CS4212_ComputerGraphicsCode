#include "Framebuffer.h"
#include "png++/png.hpp"

Framebuffer::Framebuffer() : width(100), height(100), fbStorage(width * height) {}

Framebuffer::Framebuffer(int width, int height) : width(width), height(height), fbStorage(width * height) {}

// Clear the framebuffer to a single color.
void Framebuffer::clearToColor(const color &c)
{
  for (auto idx = 0; idx < fbStorage.size(); idx++) {
    setPixelColor(idx, c);
  }
}

// Clear the framebuffer to a horizontal gradient between two colors, using linear interpolation.
void Framebuffer::clearToGradient(const color &c1, const color &c2)
{
  for (auto x = 0; x < width; x++) {
    for (auto y = 0; y < height; y++) {
      auto t = double(y) / (height);

      color c = (1 - t) * c1 + t * c2;

      setPixelColor(x, y, c);
    }
  }
}

// Set the color of a pixel at (i, j) in the framebuffer.
void Framebuffer::setPixelColor(int i, int j, const color &c)
{
  fbStorage[j * width + i] = c;
}

// Set the color of a pixel at the given index in the framebuffer.
void Framebuffer::setPixelColor(int idx, const color &c)
{
  fbStorage[idx] = c;
}

// Export the framebuffer to a PNG file with the given filename.
void Framebuffer::exportToPNG(const std::string &filename)
{
  png::image<png::rgb_pixel> imData(width, height);

  for (int j = 0; j < height; ++j) {
    for (int i = 0; i < width; ++i) {
        // int flipped_j = (height-1) - j;

        // vec3 color = fbStorage[flipped_j * width + i];
        vec3 color = fbStorage[j*width + i];

        png::byte r = static_cast<png::byte>(color.x() * 255.0);
        png::byte g = static_cast<png::byte>(color.y() * 255.0);
        png::byte b = static_cast<png::byte>(color.z() * 255.0);

        imData[j][i] = png::rgb_pixel(r, g, b);
    }
  }

  imData.write(filename);
}
#ifndef CAMERA_H
#define CAMERA_H

#include "color.h"
#include "ray.h"
#include "vec3.h"

#include <algorithm>
#include <cmath>

namespace {
constexpr double kPi = 3.14159265358979323846;
}

// Camera Class ===========================================================================

// The camera class is an abstract base class that defines the way rays are generated
// for a given pixel (i, j) in the image plane. It provides an interface for generating rays based on pixel coordinates.
class Camera {
public:
    virtual ray generateRay(int i, int j) const = 0;

    virtual void generateRay(int i, int j, ray& r) const {
        r = generateRay(i, j);
    }
};



// Perspective Camera Class ===========================================================================

// The perspective camera has ways to generate rays that simulate a pinhole camera model.
// It takes into account the camera's position, orientation, field of view, aspect ratio,
// and focal length to compute the rays that pass through each pixel in the image plane
class PerspectiveCamera : public Camera {
public:
    // default constructor initializes the camera with default parameters
    PerspectiveCamera()
        : PerspectiveCamera(point3(0.0, 0.0, 0.0),
                           point3(0.0, 0.0, -1.0),
                           vec3(0.0, 1.0, 0.0),
                           90.0,
                           16.0 / 9.0,
                           400,
                           225,
                           1.0) {}

    // constructor that initializes the camera with specified parameters
    PerspectiveCamera(point3 camera_position,
                      point3 view_direction,
                                            vec3 vup = vec3(0.0, 1.0, 0.0),
                                            double vfov = 90.0,
                                            double aspect_ratio = 16.0 / 9.0,
                                            int image_width = 400,
                                            int image_height = 225,
                                            double focal_length = 1.0)
                : origin_(camera_position),
                    image_width_(image_width),
                    image_height_(image_height),
                    aspect_ratio_(aspect_ratio),
                    vfov_(vfov),
                    focal_length_(focal_length),
                    vup_(vup),
                    view_direction_(view_direction) {
                auto theta = degrees_to_radians(vfov_);
                auto h = std::tan(theta / 2.0);
                auto viewport_height = 2.0 * h * focal_length_;
                auto viewport_width = viewport_height * aspect_ratio_;

                auto w = unit_vector(origin_ - view_direction_);
                auto u = unit_vector(cross(vup_, w));
                auto v = cross(w, u);

                viewport_u_ = viewport_width * u;
                viewport_v_ = -viewport_height * v; // switched to negative to flip image vertically
                lower_left_corner_ = origin_ - viewport_u_ / 2.0 - viewport_v_ / 2.0 - focal_length_ * w;
                pixel_delta_u_ = viewport_u_ / static_cast<double>(image_width_);
                pixel_delta_v_ = viewport_v_ / static_cast<double>(image_height_);
        }

        // Additional constructor to match mainCode.cpp usage: specify image plane width/height directly
        PerspectiveCamera(point3 camera_position,
                                            point3 view_direction,
                                            double image_plane_width,
                                            double image_plane_height,
                                            double focal_length,
                                            int image_width,
                                            int image_height,
                                            vec3 vup = vec3(0.0, 1.0, 0.0))
                : origin_(camera_position),
                    image_width_(image_width),
                    image_height_(image_height),
                    vup_(vup),
                    view_direction_(view_direction),
                    focal_length_(focal_length) {
                // compute aspect ratio from provided image resolution
                aspect_ratio_ = static_cast<double>(image_width_) / static_cast<double>(image_height_);

                // use provided physical image plane dimensions directly instead of vfov
                double viewport_width = image_plane_width;
                double viewport_height = image_plane_height;

                auto w = unit_vector(origin_ - view_direction_);
                auto u = unit_vector(cross(vup_, w));
                auto v = cross(w, u);

                viewport_u_ = viewport_width * u;
                viewport_v_ = -viewport_height * v; // flip vertically to match raster orientation
                lower_left_corner_ = origin_ - viewport_u_ / 2.0 - viewport_v_ / 2.0 - focal_length_ * w;
                pixel_delta_u_ = viewport_u_ / static_cast<double>(image_width_);
                pixel_delta_v_ = viewport_v_ / static_cast<double>(image_height_);
        }

    const point3& origin() const { return origin_; }
    const vec3& viewport_u() const { return viewport_u_; }
    const vec3& viewport_v() const { return viewport_v_; }
    const point3& lower_left_corner() const { return lower_left_corner_; }

    // Override the base class method to generate a ray for pixel (i, j)
    ray generateRay(int i, int j) const override {
        auto pixel_center = lower_left_corner_
            + (static_cast<double>(i) + 0.5) * pixel_delta_u_
            + (static_cast<double>(j) + 0.5) * pixel_delta_v_;
        return ray(origin_, pixel_center - origin_);
    }

    // Override the base class method to fill in the ray reference
    void generateRay(int i, int j, ray& r) const override {
        r = generateRay(i, j);
    }

private:
    // convert degrees to radians
    static double degrees_to_radians(double degrees) {
        return degrees * kPi / 180.0;
    }

    point3 origin_; // camera position
    point3 view_direction_; // point the camera is looking at
    vec3 vup_;     // up vector for the camera
    double vfov_ = 90.0; // vertical field of view. controls the height of the viewport
    double aspect_ratio_ = 16.0 / 9.0; // aspect ratio
    int image_width_ = 400; // image width
    int image_height_ = 225; // image height
    double focal_length_ = 1.0; // focal length
    vec3 viewport_u_ = vec3(0.0, 0.0, 0.0); // viewport horizontal vector
    vec3 viewport_v_ = vec3(0.0, 0.0, 0.0); // viewport vertical vector
    point3 lower_left_corner_ = point3(0.0, 0.0, 0.0); // lower left corner of the viewport
    vec3 pixel_delta_u_ = vec3(0.0, 0.0, 0.0); // this means the change in the u direction per pixel
    vec3 pixel_delta_v_ = vec3(0.0, 0.0, 0.0); // this means the change in the v direction per pixel
};

// Utility function to compute the color of a ray based on its direction
inline color ray_color(const ray& r) {
    vec3 unit_direction = unit_vector(r.direction());
    return color(0.5 * (unit_direction.x() + 1.0),
                 0.5 * (unit_direction.y() + 1.0),
                 0.5 * (unit_direction.z() + 1.0));
}

#endif

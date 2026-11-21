#pragma once

#include <stdexcept>

#include "ISceneLoader.h"
#include "../scene/scene.h"
#include "../geometry/sphere.h"
#include "../geometry/triangle.h"
#include "../materials/lambertian.h"
#include "../materials/blinnPhong.h"
#include "../materials/mirror.h"
#include "../materials/normal.h"
#include "../materials/constantColor.h"
#include "model_obj.h"
#include <algorithm>
#include <limits>

class SceneLoader : public ISceneLoader
{
    Scene& scene;
    int width, height;

    static vec3 convert(const vec& v) {
        return vec3(v.x, v.y, v.z);
    }

public:
    SceneLoader(Scene& scene, int width, int height) : scene(scene), width(width), height(height) {}

    void reserveCameras(size_t) override {}
    void reserveLights(size_t) override {}
    void reserveShapes(size_t) override {}
    void reserveShaders(size_t) override {}
    void reserveTextures(size_t) override {}

    void setSceneParams(const SceneParams& params) override {
        scene.backgroundColor = convert(params.bgColor);
    }

    void addCamera(const std::string&, const std::string& type, const vec& position, const vec& viewDir, float focalLength, float imagePlaneWidth) override
    {
        if (type != "perspective")
            throw std::runtime_error("Unsupported camera: " + type);

        float imagePlaneHeight =
            imagePlaneWidth * static_cast<float>(height) / width;

        scene.camera = std::make_unique<PerspectiveCamera>(
            convert(position), convert(viewDir),
            focalLength, imagePlaneWidth, imagePlaneHeight,
            width, height
        );
    }

    void addPointLight(const vec& position, const vec& intensity) override {
        scene.lights.emplace_back(convert(position), convert(intensity));
    }

    void addShader(const ShaderDesc& desc) override
    {
        if (desc.type == "Lambertian" || desc.type == "Diffuse") {
            scene.shaders[desc.name] =
                std::make_unique<Lambertian>(
                    convert(desc.diffuse.data)
                );
        }
        else if (desc.type == "BlinnPhong") {
            scene.shaders[desc.name] =
                std::make_unique<BlinnPhong>(
                    convert(desc.diffuse.data),
                    convert(desc.specular.data),
                    desc.phongExp
                );
        }
        else if (desc.type == "Mirror") {
            scene.shaders[desc.name] =
                std::make_unique<Mirror>(
                    convert(desc.specular.data)
                );
        }
        else if (desc.type == "Normal") {
            scene.shaders[desc.name] =
                std::make_unique<Normal>();
        }
        else if (desc.type == "ConstantColor" || desc.type == "Emitter") {
            scene.shaders[desc.name] =
                std::make_unique<ConstantColor>(
                    convert(desc.emission.data)
                );
        }
    }

    void addShape(const ShapeDesc& desc) override {
        Shader& shader =
            *scene.shaders.at(desc.shaderNameReference);

        if (desc.type == "sphere") {
            scene.shapes.push_back(std::make_shared<Sphere>(convert(desc.center), desc.radius, shader));
        }
        else if (desc.type == "triangle") {
            scene.shapes.push_back(std::make_shared<Triangle>(convert(desc.v0), convert(desc.v1), convert(desc.v2), shader));
        }
        else if (desc.type == "mesh") {
            ModelOBJ model;

            const int count = model.getNumberOfIndices();

            // get bounding box and scale to fit in -1 to 1 cube
            // the reason for this is to make sure its not too large/small for scene (also helps with positioning)
            auto position = [&](int index) {
                const auto& p = model.getVertex(index).position;
                return vec3(p[0], p[1], p[2]);
            };

            // 
            const double infinity = std::numeric_limits<double>::infinity(); // initialize to infinity (any vertex will be smaller)
            vec3 minimum(infinity, infinity, infinity); // same thing as above
            vec3 maximum(-infinity, -infinity, -infinity); // same but negative infinity (any vertex is bigger)

            // iterate through all vertices and find min and max for each axis (x y and z)
            // these values are used for bounding box
            for (int i = 0; i < model.getNumberOfVertices(); ++i) {
                const vec3 p = position(i);
                for (int axis = 0; axis < 3; ++axis) {
                    minimum[axis] = std::min(minimum[axis], p[axis]);
                    maximum[axis] = std::max(maximum[axis], p[axis]);
                }
            }

            const vec3 center = (minimum + maximum) / 2.0; // find center of bounding box
            const vec3 size = maximum - minimum; // find size of bounding box
            const double largest = std::max({size[0], size[1], size[2]}); // find which axis is biggest and use it for scaling (remember -1 to 1 cube)

            const double scale = 2.0 / largest; // do the actual scaling
            const int* indices = model.getIndexBuffer(); // the index buffer is just a list of indices that make up the triangles (3 per triangle)

            // go through all the triangles and add them to the scene (after scaling and centering)
            for (int i = 0; i < count; i += 3) {
                // scale and center the vertex positions
                const vec3 a = (position(indices[i]) - center) * scale; 
                const vec3 b = (position(indices[i + 1]) - center) * scale;
                const vec3 c = (position(indices[i + 2]) - center) * scale;

                // check if the triangle has no area (or its close to zero area). The reason for this is to improve performance
                if (cross(b - a, c - a).length_squared() <= 1e-20)
                    continue;

                // create triangle and add it to the scene
                scene.shapes.push_back(std::make_shared<Triangle>(a, b, c, shader));
            }
        }
        else {
            throw std::runtime_error(
                "Unsupported shape: " + desc.type
            );
        }
    }

    void addAreaLight(const vec&, const vec&, const vec&, float, float) override {
        throw std::runtime_error("Area light not supported.");
    }

    void addTexture(const std::string&, const std::string&, const std::string&) override {
        throw std::runtime_error("Textures not supported.");
    }
};
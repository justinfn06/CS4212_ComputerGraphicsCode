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

            if (!model.import(desc.meshFilePath.c_str())) {
                throw std::runtime_error(
                    "Could not load mesh: " + desc.meshFilePath);
            }

            const int count = model.getNumberOfIndices();
            if (count == 0) {
                throw std::runtime_error("Mesh contains no triangles");
            }

            auto position = [&](int index) {
                const auto& p = model.getVertex(index).position;
                return vec3(p[0], p[1], p[2]);
            };

            const double infinity = std::numeric_limits<double>::infinity();
            vec3 minimum(infinity, infinity, infinity);
            vec3 maximum(-infinity, -infinity, -infinity);

            for (int i = 0; i < model.getNumberOfVertices(); ++i) {
                const vec3 p = position(i);
                for (int axis = 0; axis < 3; ++axis) {
                    minimum[axis] = std::min(minimum[axis], p[axis]);
                    maximum[axis] = std::max(maximum[axis], p[axis]);
                }
            }

            const vec3 center = (minimum + maximum) / 2.0;
            const vec3 size = maximum - minimum;
            const double largest = std::max({size[0], size[1], size[2]});

            if (largest <= 0.0) {
                throw std::runtime_error("Mesh has no usable size");
            }

            const double scale = 2.0 / largest;
            const int* indices = model.getIndexBuffer();

            for (int i = 0; i < count; i += 3) {
                const vec3 a = (position(indices[i]) - center) * scale;
                const vec3 b = (position(indices[i + 1]) - center) * scale;
                const vec3 c = (position(indices[i + 2]) - center) * scale;

                if (cross(b - a, c - a).length_squared() <= 1e-20)
                    continue;

                scene.shapes.push_back(
                    std::make_shared<Triangle>(a, b, c, shader));
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
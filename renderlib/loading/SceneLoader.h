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
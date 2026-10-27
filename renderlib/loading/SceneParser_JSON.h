#pragma once

#include <memory>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "ISceneLoader.h"

class SceneParser_JSON {
public:
  SceneParser_JSON(std::shared_ptr<ISceneLoader> sceneLoader)
      : loader(sceneLoader) {}

  void parseStringData(const std::string &json_string);
  void parseFileData(const std::string &filename);

private:
  void parseJSONData(const json &data);

  void parseCamera(const json &camera);
  void parseLight(const json &light);
  void parseShader(const json &shader);
  void parseShape(const json &shape);
  void parseTexture(const json &shape);

  std::shared_ptr<ISceneLoader> loader;

  ISceneLoader::ShaderProperty
  extractShaderProperty(const json &shaderParam,
                        const std::string propertyName);
  glm::mat4 parseTransformData(const json &xformData);
};

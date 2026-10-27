#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stack>

#include "SceneParser_JSON.h"

//
// function that the json parser will pick up to load vecs
//
void from_json(const json &j, ISceneLoader::vec &p) {
  // case when the input matches
  // "0.0 0.0 0.0",
  if (j.is_string()) {
    std::string s = j.get<std::string>();
    std::stringstream ss(s);
    ss >> p.x >> p.y >> p.z;
  }

  // and when it is a JSON array
  // [0, 0, 0]
  else if (j.is_array() && j.size() == 3) {
    p.x = j[0];
    p.y = j[1];
    p.z = j[2];
  }

  else {
    // Throwing a json::type_error keeps it consistent with the library's own
    // errors
    throw json::type_error::create(
        302,
        "Expected a string or 3-element array for vec, got " +
            std::string(j.type_name()),
        &j);
  }
}

void SceneParser_JSON::parseStringData(const std::string &json_string) {
  std::cout << "Attempting to parse JSON String..." << std::endl;

  json json_data = json::parse(json_string);
  parseJSONData(json_data);
}

void SceneParser_JSON::parseFileData(const std::string &filename) {
  std::cout << "Attempting to parse: " << filename << std::endl;
  std::ifstream inputFileStream(filename);

  // ///////////////////////////////////////
  // open file and parse by json class
  json json_data;
  inputFileStream >> json_data;

  parseJSONData(json_data);
}

void SceneParser_JSON::parseJSONData(const json &json_data) {
  //
  // Parse the various components
  //

  // if "scene" exists, step inside. Otherwise, assume json_data IS the scene
  // level
  const json &json_context =
      json_data.contains("scene") ? json_data["scene"] : json_data;

  if (json_context.contains("sceneParameters")) {
    const json &sceneParam = json_context["sceneParameters"];

    ISceneLoader::SceneParams sParams;
    sParams.bgColor =
        sceneParam.value("bgColor", ISceneLoader::vec{0.0f, 0.0f, 0.0f});
    sParams.envMapFile = sceneParam.value("envMapVertCross", "");
    loader->setSceneParams(sParams);
  }

  // Cameras
  if (json_context.contains("camera")) {

    json debugCam;

    loader->reserveCameras(json_context["camera"].size());
    try {
      for (auto &camera : json_context["camera"]) {
        debugCam = camera;
        parseCamera(camera);
      }
    } catch (json::type_error &e) {
      std::cout << "Camera: " << debugCam.dump(2) << std::endl;
      std::cout << "JSON Type Error: " << e.what() << std::endl;
      std::cout << "ID: " << e.id << std::endl;
    }
  }

  // Lights
  if (json_context.contains("light")) {
    loader->reserveLights(json_context["light"].size());
    for (auto &light : json_context["light"]) {
      parseLight(light);
    }
  }

  // Shaders
  if (json_context.contains("shader")) {
    loader->reserveShaders(json_context["shader"].size());
    for (auto &shader : json_context["shader"]) {
      parseShader(shader);
    }
  }

  // Shapes
  if (json_context.contains("shape")) {
    loader->reserveShapes(json_context["shape"].size());
    for (auto &shape : json_context["shape"]) {
      parseShape(shape);
    }
  }

  // Textures
  if (json_context.contains("texture")) {
    loader->reserveTextures(json_context["texture"].size());
    for (auto &texture : json_context["texture"]) {
      parseTexture(texture);
    }
  }
}

void SceneParser_JSON::parseCamera(const json &camera) {
  ISceneLoader::vec position, viewDir;
  position = camera["position"];

  if (camera.contains("lookatPoint")) {
    ISceneLoader::vec lookAtPoint = camera["lookatPoint"];
    viewDir.x = lookAtPoint.x - position.x;
    viewDir.y = lookAtPoint.y - position.y;
    viewDir.z = lookAtPoint.z - position.z;
  } else {
    viewDir = camera["viewDir"];
  }

  float focalLength = camera.value("focalLength", 1.0f);
  float imagePlaneWidth = camera.value("imagePlaneWidth", 0.5f);

  std::string camType =
      camera.value("type", camera.value("_type", "perspective"));
  std::string camName = camera.value("name", camera.value("_name", "default"));

  loader->addCamera(camName, camType, position, viewDir, focalLength,
                    imagePlaneWidth);
}

void SceneParser_JSON::parseLight(const json &light) {
  std::string type = light.value("type", light.value("_type", "point"));
  ;

  ISceneLoader::vec position, radiantEnergy;
  position = light["position"];
  radiantEnergy = light["intensity"];

  if (type == "area") {
    position = light["position"];

    ISceneLoader::vec normal;
    normal = light["normal"];

    float width = 1.0, length = 1.0;
    width = light["width"];
    length = light["length"];

    loader->addAreaLight(position, radiantEnergy, normal, width, length);
  } else {
    loader->addPointLight(position, radiantEnergy);
  }
}

ISceneLoader::ShaderProperty
SceneParser_JSON::extractShaderProperty(const json &shaderParam,
                                        const std::string propertyName) {
  ISceneLoader::ShaderProperty result;

  if (!shaderParam.contains(propertyName))
    return result;

  const auto &prop = shaderParam[propertyName];

  // Handle complex objects that can specify more connected
  // parameters for texturing or maybe even procedural parameters??
  // Example:  "diffuse": { "tex": "...", "data": [...] }
  if (prop.is_object()) {
    if (prop.contains("data")) {
      // Using the custom from_json
      // will handle both "data": "1 1 1" and "data": [1, 1, 1]
      result.data = prop["data"].get<ISceneLoader::vec>();
    } else {
      // Default base color if they supply a texture but no data
      result.data = {1.0f, 1.0f, 1.0f};
    }

    if (prop.contains("tex")) {
      result.textureName = prop["tex"];
      result.hasTexture = true;
    }
  } else {
    // Handle simple arrays: "diffuse": [1, 0, 0]
    result.data = prop.get<ISceneLoader::vec>();
  }

  return result;
}

void SceneParser_JSON::parseShader(const json &shader) {
  ISceneLoader::ShaderDesc shaderDesc;

  shaderDesc.type = shader.value("type", shader.value("_type", "unknown"));
  shaderDesc.name = shader.value("name", shader.value("_name", "unknown"));

  if (shaderDesc.type == "Lambertian" || shaderDesc.type == "Diffuse") {
    shaderDesc.diffuse = extractShaderProperty(shader, "diffuse");
  }

  else if (shaderDesc.type == "BlinnPhong" || shaderDesc.type == "Phong") {

    shaderDesc.diffuse = extractShaderProperty(shader, "diffuse");
    shaderDesc.specular = extractShaderProperty(shader, "specular");

    shaderDesc.phongExp = shader.value("phongExp", 1.0f);
  }

  else if (shaderDesc.type == "Mirror") {
    shaderDesc.specular = extractShaderProperty(shader, "specular");
  }

  else if (shaderDesc.type == "Glaze") {
    shaderDesc.diffuse = extractShaderProperty(shader, "diffuse");
    shaderDesc.mirrorCoef = shader.value("mirrorCoef", 0.0f);
  }

  else if (shaderDesc.type == "Dielectric") {
    float refrIdx;
    ISceneLoader::vec attenCoef;
    shaderDesc.attenuationCoef =
        shader.value("attenuationCoef", ISceneLoader::vec{1.0f, 1.0f, 1.0f});
    shaderDesc.refractiveIndex = shader.value("refractiveIndex", 1.0f);
  }

  else if (shaderDesc.type == "BlinnPhongMirrored") {

    shaderDesc.diffuse = extractShaderProperty(shader, "diffuse");
    shaderDesc.specular = extractShaderProperty(shader, "specular");
    shaderDesc.phongExp = shader.value("phongExp", 1.0f);
    shaderDesc.mirrorCoef = shader.value("mirrorCoef", 0.0f);
    shaderDesc.roughness = shader.value("roughness", 0.0f);

  } else if (shaderDesc.type == "CoolToWarm") {
    shaderDesc.diffuse = extractShaderProperty(shader, "diffuse");
  }

  else if (shaderDesc.type == "Emitter") {
    shaderDesc.emission = extractShaderProperty(shader, "emission");
  }

  loader->addShader(shaderDesc);
}

glm::mat4 extractRotation(const json &rotInfo) {
  const float m_pi = 3.14159265359f;

  glm::mat4 tmpM(1.0f);
  float rot = rotInfo.value("amount", 0.0f);
  std::string axis = rotInfo.value("axis", "");

  if (axis == "X") {
    tmpM = glm::rotate(glm::mat4(1.0f), rot * m_pi / 180.0f,
                       glm::vec3(1.0f, 0.0f, 0.0f));
  } else if (axis == "Y") {
    tmpM = glm::rotate(glm::mat4(1.0f), rot * m_pi / 180.0f,
                       glm::vec3(0.0f, 1.0f, 0.0f));
  } else if (axis == "Z") {
    tmpM = glm::rotate(glm::mat4(1.0f), rot * m_pi / 180.0f,
                       glm::vec3(0.0f, 0.0f, 1.0f));
  }

  return tmpM;
}

glm::mat4 SceneParser_JSON::parseTransformData(const json &transformData) {
  glm::mat4 glM(1.0f);

  glm::mat4 tmpM(1.0f);
  std::stack<glm::mat4> transformStack;

  assert(transformData.is_array());
  for (auto xIdx = 0; xIdx < transformData.size(); xIdx++) {

    json xformInfo = transformData[xIdx];

    // Translation transform
    if (xformInfo["type"] == "translate") {
      ISceneLoader::vec translation;
      translation = xformInfo["amount"];
      tmpM = glm::translate(
          glm::mat4(1.0f),
          glm::vec3(translation.x, translation.y, translation.z));
      transformStack.push(tmpM);
    } else if (xformInfo["type"] == "scale") {
      ISceneLoader::vec scale;
      scale = xformInfo["amount"];
      tmpM = glm::scale(glm::mat4(1.0f), glm::vec3(scale.x, scale.y, scale.z));
      transformStack.push(tmpM);
    } else if (xformInfo["type"] == "rotation") {
      transformStack.push(extractRotation(xformInfo));
    }
  }

  while (!transformStack.empty()) {
    glm::mat4 m1 = transformStack.top();
    transformStack.pop();
    glM = m1 * glM;
  }

  return glM;
}

void SceneParser_JSON::parseShape(const json &shape) {
  ISceneLoader::ShapeDesc shapeDesc;

  shapeDesc.type = shape.value("type", shape.value("_type", "unknown"));
  shapeDesc.name = shape.value("name", shape.value("_name", "unknown"));

  // If there is a transform in the shape, we need to extract it. We
  // will store the final combined transform rather than the overall
  // set of matrices that it is composed of.
  if (shape.contains("xform")) {
    shapeDesc.hasXForm = true;
    shapeDesc.localXForm = parseTransformData(shape["xform"]);
  } else {
    shapeDesc.hasXForm = false;
    shapeDesc.localXForm = glm::mat4(1.0f); // identity matrix
  }

  // shader reference name here...
  if (shape.contains("shader") && shape["shader"].is_object()) {
    const auto &shaderObj = shape["shader"];

    if (shaderObj.contains("ref") || shaderObj.contains("_ref")) {
      shapeDesc.shaderNameReference =
          shaderObj.value("ref", shaderObj.value("_ref", ""));
    }
  }

  if (shapeDesc.type == "sphere") {
    shapeDesc.center = shape.value("center", ISceneLoader::vec{0, 0, 0});
    shapeDesc.radius = shape.value("radius", 1.0f);
    ;
  }

  else if (shapeDesc.type == "triangle") {

    shapeDesc.v0 = shape.value("v0", ISceneLoader::vec{0, 0, 0});
    shapeDesc.v1 = shape.value("v1", ISceneLoader::vec{1, 0, 0});
    shapeDesc.v2 = shape.value("v2", ISceneLoader::vec{0, 1, 0});
  }

  else if (shapeDesc.type == "box") {
    shapeDesc.minPt = shape.value("minPt", ISceneLoader::vec{-1, -1, -1});
    shapeDesc.maxPt = shape.value("maxPt", ISceneLoader::vec{1, 1, 1});
  }

  else if (shapeDesc.type == "mesh") {
    std::string meshFileName = shape.value("file", "");
    shapeDesc.meshFilePath = meshFileName;
  }

  else if (shapeDesc.type == "instance") {
    shapeDesc.instanceId =
        shape.value("id", shape.value("_id", "unknownInstanceId"));
  }

  loader->addShape(shapeDesc);
}

void SceneParser_JSON::parseTexture(const json &texture) {
  std::string type = texture.value("type", texture.value("_type", "unknown"));
  std::string name = texture.value("name", texture.value("_name", "unknown"));

  bool hasImage = false;
  std::string sourceFile = "";
  if (type == "image") {
    hasImage = true;
    sourceFile = texture.value("sourcefile", "");
    ;
  }

  loader->addTexture(type, name, sourceFile);
}

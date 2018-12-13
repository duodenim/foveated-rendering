#include "Map.h"
#include "Log.h"
#include "FileLoader.h"
#include "Components/MeshComponent.h"
#include <fstream>
#include <json.hpp>
#include "Components/BillboardComponent.h"

using json = nlohmann::json;

static Vec3 JsonToPositionVec3(const json& json) {
  Vec3 ret;
  ret.x = json.at("x").get<float>();
  ret.y = json.at("y").get<float>();
  ret.z = json.at("z").get<float>();

  return ret;
}

Map::Map() {
  mDirectionalLight = AddComponent<DirectionalLightComponent>();
}

Map::~Map() {

}

void Map::Update(float deltaTime) {
  GameObject::Update(deltaTime);
}

void Map::LoadMap(const std::string &mapPath, const bool relativeToDataFolder) {
  const std::string absoluteMapPath = relativeToDataFolder ? FileLoader::GetFilePath(mapPath) : mapPath;

  std::ifstream mapFile(absoluteMapPath);

  if (mapFile.fail()) {
    Log::LogFatal("Could not find map file");
    Log::LogFatal(absoluteMapPath);
    exit(1);
  }

  json mapJson;

  mapFile >> mapJson;

  auto entities = mapJson.at("entities").get<std::vector<json>>();

  for (auto &entity: entities) {
    const std::string entityType = entity.at("type").get<std::string>();
    if (entityType != "static") {
      Log::LogWarning("Dynamic entities are not supported yet, this entity will not be spawned");
    }
    std::shared_ptr<MeshComponent> mesh = AddComponent<MeshComponent>();
    mesh->LoadMeshData(entity.at("mesh").get<std::string>());
    auto position = entity.at("position").get<json>();
    auto rotation = entity.at("rotation").get<json>();
    auto scale = entity.at("scale").get<json>();
    mesh->SetPosition(JsonToPositionVec3(position));
    mesh->SetRotation(JsonToPositionVec3(rotation));
    mesh->SetScale(JsonToPositionVec3(scale));
    mesh->LoadShader("static_entity.vert", "static_entity.frag");
  }

  //Load lights
  if (mapJson.find("directionalLight") != mapJson.end()) {
    const auto light = mapJson.at("directionalLight").get<json>();

    const auto direction = light.at("direction").get<json>();
    const auto ambient = light.at("ambient").get<json>();
    const auto diffuse = light.at("diffuse").get<json>();
    const auto specular = light.at("specular").get<json>();

    float aR = ambient.at("r").get<float>();
    float aG = ambient.at("g").get<float>();
    float aB = ambient.at("b").get<float>();

    float difR = diffuse.at("r").get<float>();
    float difG = diffuse.at("g").get<float>();
    float difB = diffuse.at("b").get<float>();

    float sR = specular.at("r").get<float>();
    float sG = specular.at("g").get<float>();
    float sB = specular.at("b").get<float>();

    mDirectionalLight->SetDirection(JsonToPositionVec3(direction));
    mDirectionalLight->SetAmbientColor(Vec3(aR,aG,aB));
    mDirectionalLight->SetDiffuseColor(Vec3(difR,difG,difB));
    mDirectionalLight->SetSpecularColor(Vec3(sR,sG,sB));
  }

  //Load characters
  if (mapJson.find("characters") != mapJson.end()) {
    const auto characters = mapJson.at("characters").get<std::vector<json>>();

    for (const auto& character : characters) {
      auto billboard = AddComponent<BillboardComponent>();
      auto position = character.at("position").get<json>();
      auto rotation = character.at("rotation").get<json>();

      billboard->SetImage(character.at("image").get<std::string>());
      billboard->SetPosition(JsonToPositionVec3(position));
      billboard->SetRotation(JsonToPositionVec3(rotation));
    }
  }
  Log::LogInfo("Finished Parsing Map:" + mapPath);
}

#pragma once

#include "Components/DirectionalLightComponent.h"
#include "GameObject.h"

/*
 * This class contains all static geometry for the currently loaded map.
 * It is automatically spawned by the Engine at startup
 * Call LoadMap() to set the map to a specified file
 */
class Map : public GameObject {
public:
  Map();
  ~Map();

  /**
   * Functions inherited from GameObject
   */
  void Update(float deltaTime);

  /**
   * Parses a map file and transitions the game to that map
   * @param[in] mapPath The map file to load
   * @param[in] relativeToDataFolder - True if mapPath is relative to the data folder, false if relative to the executable
   */
  void LoadMap(const std::string &mapPath, const bool relativeToDataFolder);

private:
  std::shared_ptr<DirectionalLightComponent> mDirectionalLight;
};

#include <EngineCore.h>
#include <CommandArgs.h>
#include <Config.h>
#include <ctime>
#include "World/Player.h"

int main(int argc, char** argv){
  srand(time(0));
  Config::LoadFile("config.txt");
  CommandArgs::ParseArgs(argc, argv);
  EngineCore* core = EngineCore::GetEngine();

  core->SpawnObject<Player>();

  std::string mapToLoad = CommandArgs::GetMapToLoad();
  if (mapToLoad.empty()) {
    core->GetMap()->LoadMap("maps/spheres.map", true);
  } else {
    core->GetMap()->LoadMap(mapToLoad, false);
  }

  
  core->BeginGame();
  delete core;

  return 0;
}


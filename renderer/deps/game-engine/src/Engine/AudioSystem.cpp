#include "AudioSystem.h"
#include <SDL_mixer.h>
#include "Log.h"

namespace Audio {
  void Init() {
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) != 0) {
      Log::LogFatal("Could not initialize SDL Mixer");
      exit(1);
    }
  }

  void Shutdown() {
    Mix_CloseAudio();
  }
}

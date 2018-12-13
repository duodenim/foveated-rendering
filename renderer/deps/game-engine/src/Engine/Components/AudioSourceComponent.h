#ifndef AUDIO_SOURCE_COMPONENT_H
#define AUDIO_SOURCE_COMPONENT_H

#include "../Component.h"
#include "../AudioSystem.h"
#include <SDL_mixer.h>

class AudioSourceComponent : public Component {
public:
	AudioSourceComponent();
	~AudioSourceComponent();
	void LoadWAVFromFile(const std::string sndFilePath);
  void SetVolume(const float volume);
	void Play();
private:
  Mix_Chunk* mSample;
	float mVolume;
};
#endif
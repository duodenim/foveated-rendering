#include "AudioSourceComponent.h"
#include "../FileLoader.h"

AudioSourceComponent::AudioSourceComponent() {
  mSample = NULL;
  mVolume = 1;
}

AudioSourceComponent::~AudioSourceComponent() {
  if (mSample) {
    Mix_FreeChunk(mSample);
  }
}

void AudioSourceComponent::LoadWAVFromFile(const std::string sndFilePath) {
  if (mSample) {
    Mix_FreeChunk(mSample);
  }
  const std::string absoluteFilePath = FileLoader::GetFilePath(sndFilePath);
  mSample = Mix_LoadWAV(absoluteFilePath.c_str());
}

void AudioSourceComponent::Play() {
  if (mSample) {
    Mix_VolumeChunk(mSample, (int)(mVolume * 128));
    Mix_PlayChannel(-1, mSample, 0);
  }
}

void AudioSourceComponent::SetVolume(const float volume) {
  if (volume > 1.0f) {
    mVolume = 1.0f;
  } else if (volume < 0.0f) {
    mVolume = 0.0f;
  } else {
    mVolume = volume;
  }
}

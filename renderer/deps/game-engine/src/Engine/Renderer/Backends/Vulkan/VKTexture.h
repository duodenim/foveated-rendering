#pragma once

#include "VKImage.h"
#include "../../Texture.h"

class VKTexture : public Texture {
public:
  VKImage m_Image;
  VkDescriptorSet m_TextureDescriptorSet;
};
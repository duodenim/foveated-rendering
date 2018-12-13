#pragma once

#include "../../Backend.h"
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include "VKDevice.h"
#include "VKSurface.h"
#include "VKImage.h"
#include "VKBuffer.h"
#include "VKShader.h"
#include "VKFrameBuffer.h"
#include "VKTexture.h"

class VKBackend : public RenderBackend {
public:
  static bool IsUsable();
  void Init();
  void WindowInit(const std::string name, int width, const int height);
  void Shutdown();
  const Model LoadModel(const std::vector<Vertex> vertices, const std::vector<u32> indices);
  Texture* LoadTexture(const unsigned char* data, const int width, const int height, const int numChannels);
  Shader* CreateShader(const std::vector<char> vertexProgram, const std::vector<char> fragmentProgram, const DRAW_STAGE stage);
  void SetFrameBufferModel(const Model &model);
  void SetFrameBufferShader(Shader* shader, const DRAW_STAGE stage);

  void Draw(const Mat4 &viewMatrix, const Mat4 &projMatrix, const Mat4 &userData, const std::vector<Drawable> &scene, const std::vector<Drawable> &ui, const LightData& lights);

  void DeleteModel(Model &model);
  void DeleteTexture(Texture* tex);
  void DeleteShader(Shader* shader);

  std::string GetShaderFolderName();
  DEPTH_MODE GetDepthMode();

  std::string GetDeviceName();
  u64 GetUsedVRAM();

private:
  VKSurface m_Surface;

  VkInstance m_Instance;
  VKDevice m_Device;

  VmaAllocator m_MemAllocator;

  VKFrameBuffer m_WorldFB;
  VKFrameBuffer m_UIFB;
  VKFrameBuffer m_ShadowFB;
  VKFrameBuffer m_FoveatedFB;
  VkDescriptorSet m_WorldFBDescriptorSet;
  VkDescriptorSet m_UIFBDescriptorSet;
  VkDescriptorSet m_FoveatedDescriptorSet;
  Model m_FBModel;
  VKShader* m_UIFBShader;
  VKShader* m_AspectShader;
  VKShader* m_FoveatedClearShader;

  VkCommandBuffer m_WorldCmdBuffer;
  VkCommandBuffer m_FoveatedCmdBuffer;
  VkCommandBuffer m_UICmdBuffer;
  VkCommandBuffer m_PresentCmdBuffer;
  VkCommandBuffer m_ShadowCmdBuffer;

  VkSemaphore m_ImageAvailable;
  VkSemaphore m_RenderFinished;
  VkSemaphore m_ShadowToFoveated;

  VkDescriptorSetLayout m_PerFrameDescriptorSetLayout;
  VkDescriptorSetLayout m_PerObjectDescriptorSetLayout;
  VkDescriptorSet m_PerFrameDescriptorSet;
  VkPipelineLayout m_PipelineLayout;

  VKBuffer mCameraUBO;
  VKBuffer mLightUBO;
  VKBuffer mUsrDataUBO;

  VKBuffer m_StagingBuffer;

  VkSampler m_TextureSampler;
  VkSampler m_ShadowSampler;

  VKShader* m_ShadowShader;

  VKTexture* m_DummyImage;

  VkFence m_LastFrameFinished;

  u32 m_ShadowSize;

  VkPipeline CreateGraphicsPipeline(const VkShaderModule vertexModule, const VkShaderModule fragModule, const VkRenderPass renderpass, const VkExtent2D renderExtent);

  VkCommandBuffer MakeOneTimeBuffer();
  void SubmitOneTimeBuffer(VkQueue queue, VkCommandBuffer &command);

  void DrawModel(const Drawable &d, VkCommandBuffer cmdBfr);
  void DrawFrameBuffer(VkCommandBuffer cmdBfr, VkPipeline pipeline, VkDescriptorSet descSet);
};
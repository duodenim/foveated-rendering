#define VMA_IMPLEMENTATION
#include "VKRenderer.h"
#include "../../../Log.h"
#include <SDL_video.h>
#include "VKVertexBuffer.h"
#include "VKTexture.h"
#include "VKFrameBuffer.h"
#include "VKError.h"
#include <imgui.h>
#include "imgui_impl_vulkan.h"
#include "../../Frontend.h"
#include "../../../Config.h"
#include "GazePoint.h"

#include <gtc/matrix_transform.hpp>

u8 dummyImageData[] = {
  0x00, 0x00, 0x00, 0xff,
  0x00, 0x00, 0x00, 0xff,
  0x00, 0x00, 0x00, 0xff,
  0x00, 0x00, 0x00, 0xff
};

#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/euler_angles.hpp>
#include <gtc/type_ptr.hpp>

const int STAGING_BUFFER_SIZE = 64 * 1024 * 1024; //64MB should be plenty for a staging buffer

const std::vector<const char*> VALIDATION_LAYERS = {
  "VK_LAYER_LUNARG_standard_validation"
};

const std::vector<const char*> REQUIRED_DEVICE_EXTENSIONS = {
  VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
const bool ENABLE_VALIDATION_LAYERS = false;
#else
const bool ENABLE_VALIDATION_LAYERS = true;
#endif

const u32 SHADOW_SIZE = 4096;

bool VKBackend::IsUsable() {
  //Create a dummy SDL Vulkan window. 
  //If it works then we have vulkan support
  SDL_Window* test;
  test = SDL_CreateWindow("test", 0, 0, 1, 1, SDL_WINDOW_VULKAN);

  if (test == nullptr) {
    return false;
  }
  SDL_DestroyWindow(test);
  return true;
}
void VKBackend::Init() {
  //Init foveated rendering devices
  GazePointManager::InitDevices();
  GazePointManager::SelectDevice(0); //TODO - replace this index with actual number

}

void VKBackend::WindowInit(const std::string name, int width, const int height) {
  //Create SDL window
  const std::string windowName = name + "[Vulkan]";
  m_Surface.CreateWindow(windowName, width, height);

  //Log instance level extensions
  u32 instExtCount = 0;
  VKError::CheckResult(vkEnumerateInstanceExtensionProperties(nullptr, &instExtCount, nullptr), "Could not get instance extensions");
  std::vector<VkExtensionProperties> instExtensions(instExtCount);
  VKError::CheckResult(vkEnumerateInstanceExtensionProperties(nullptr, &instExtCount, instExtensions.data()), "Could not get instance extensions");

  Log::LogInfo("[VKBackend] Available instance extensions: ");
  for (const auto &ext : instExtensions) {
    Log::LogInfo("[VKBackend] " + std::string(ext.extensionName));
  }

  VkApplicationInfo appInfo = {};
  appInfo.pApplicationName = "FovRender";
  appInfo.applicationVersion = 1;
  appInfo.pEngineName = "CoolEngine";
  appInfo.engineVersion = 1;
  appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);
  VkInstanceCreateInfo createInfo = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};

  std::vector<const char*> extensions = m_Surface.GetInstanceExtensions();

  //Enable debugging
  if (ENABLE_VALIDATION_LAYERS) {
    createInfo.enabledLayerCount = (u32)VALIDATION_LAYERS.size();
    createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();

    extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
  } else {
    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = nullptr;
  }
  
  createInfo.enabledExtensionCount = (u32)extensions.size();
  createInfo.ppEnabledExtensionNames = extensions.data();
  createInfo.pApplicationInfo = &appInfo;

  VKError::CheckResult(vkCreateInstance(&createInfo, nullptr, &m_Instance), "Could not create instance");

  //Create debug message callback function
  if (ENABLE_VALIDATION_LAYERS) {
    VKError::Setup(m_Instance, VK_DEBUG_REPORT_ERROR_BIT_EXT);
  }

  //Create window drawing surface
  m_Surface.CreateSurface(m_Instance);

  m_Device.SetupDevice(m_Instance, REQUIRED_DEVICE_EXTENSIONS, m_Surface.GetSurface());

  m_Surface.CreateSwapchain(m_Device.GetPhysicalDevice(), m_Device.GetDevice());
  
  //Create memory allocator object
  VmaAllocatorCreateInfo allocatorInfo = {};
  allocatorInfo.device = m_Device.GetDevice();
  allocatorInfo.physicalDevice = m_Device.GetPhysicalDevice();

  VKError::CheckResult(vmaCreateAllocator(&allocatorInfo, &m_MemAllocator), "Could not create memory allocator");

  //Get supported swapchain properties
  VkSurfaceCapabilitiesKHR swapChainCapabilities = m_Surface.GetCapabilities();

  //Create framebuffers

  //World framebuffer
  std::vector<VkFormat> fbFormat(1);
  fbFormat[0] = m_Surface.GetDefaultFormat().format;
  m_WorldFB.Setup(swapChainCapabilities.minImageExtent.width, swapChainCapabilities.minImageExtent.height, fbFormat, VK_FORMAT_D32_SFLOAT, false, m_Device.GetDevice(), m_MemAllocator);

  //Foveated framebuffer
  m_FoveatedFB.Setup(swapChainCapabilities.minImageExtent.width, swapChainCapabilities.minImageExtent.height, fbFormat, VK_FORMAT_D32_SFLOAT, false, m_Device.GetDevice(), m_MemAllocator);

  //UI framebuffer
  m_UIFB.Setup(swapChainCapabilities.minImageExtent.width, swapChainCapabilities.minImageExtent.height, fbFormat, VK_FORMAT_UNDEFINED, false, m_Device.GetDevice(), m_MemAllocator);

  //Setup shadow resolution
  if (Config::OptionExists("ShadowResolution")) {
    m_ShadowSize = Config::GetOptionInt("ShadowResolution");
  } else {
    m_ShadowSize = SHADOW_SIZE;
  }
  m_ShadowFB.Setup(m_ShadowSize, m_ShadowSize, std::vector<VkFormat>(), VK_FORMAT_D32_SFLOAT, true, m_Device.GetDevice(), m_MemAllocator);

  //Allocate command buffers for world/ui/aspect passes
  std::vector<VkCommandBuffer> outBfrs = m_Device.AllocateCommandBuffers(VK_COMMAND_BUFFER_LEVEL_PRIMARY, 5);

  m_WorldCmdBuffer = outBfrs[0];
  m_UICmdBuffer = outBfrs[1];
  m_PresentCmdBuffer = outBfrs[2];
  m_ShadowCmdBuffer = outBfrs[3];
  m_FoveatedCmdBuffer = outBfrs[4];

  //Setup texture sampling info
  VkSamplerCreateInfo sampler = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
  sampler.magFilter = VK_FILTER_LINEAR;
  sampler.minFilter = VK_FILTER_LINEAR;
  sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  sampler.anisotropyEnable = VK_FALSE;
  sampler.maxAnisotropy = 1;
  sampler.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  sampler.unnormalizedCoordinates = VK_FALSE;
  sampler.compareEnable = VK_FALSE;
  sampler.compareOp = VK_COMPARE_OP_ALWAYS;
  sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  sampler.mipLodBias = 0.0f;
  sampler.minLod = 0.0f;
  sampler.maxLod = 0.0f;

  VKError::CheckResult(vkCreateSampler(m_Device.GetDevice(), &sampler, nullptr, &m_TextureSampler), "Could not create texture sampler");

  sampler.magFilter = VK_FILTER_LINEAR;
  sampler.minFilter = VK_FILTER_LINEAR;
  sampler.addressModeU = sampler.addressModeV = sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
  sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

  VKError::CheckResult(vkCreateSampler(m_Device.GetDevice(), &sampler, nullptr, &m_ShadowSampler), "Could not make shadow map sampler");

  //Setup uniform data descriptor info
  
  //Camera info
  VkDescriptorSetLayoutBinding cameraUBOBinding = {};
  cameraUBOBinding.binding = 0;
  cameraUBOBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  cameraUBOBinding.descriptorCount = 1;
  cameraUBOBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  //Light info
  VkDescriptorSetLayoutBinding lightBinding = {};
  lightBinding.binding = 1;
  lightBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  lightBinding.descriptorCount = 1;
  lightBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT;

  //User Data
  VkDescriptorSetLayoutBinding usrDataBinding = {};
  usrDataBinding.binding = 7;
  usrDataBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  usrDataBinding.descriptorCount = 1;
  usrDataBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

  //Shadow maps
  VkDescriptorSetLayoutBinding smBinding = {};
  smBinding.binding = 6;
  smBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  smBinding.descriptorCount = 1;
  smBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  VkDescriptorSetLayoutBinding bindings[] = { cameraUBOBinding, lightBinding, smBinding, usrDataBinding };

  VkDescriptorSetLayoutCreateInfo descSetLayout = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
  descSetLayout.bindingCount = 4;
  descSetLayout.pBindings = bindings;

  VKError::CheckResult(vkCreateDescriptorSetLayout(m_Device.GetDevice(), &descSetLayout, nullptr, &m_PerFrameDescriptorSetLayout), "Could not create per frame descriptor set layout");

  VkDescriptorSetLayoutBinding texDataBinding = {};
  texDataBinding.binding = 0;
  texDataBinding.descriptorCount = 1;
  texDataBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  texDataBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

  VkDescriptorSetLayoutCreateInfo objectSetLayout = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
  objectSetLayout.bindingCount = 1;
  objectSetLayout.pBindings = &texDataBinding;

  VKError::CheckResult(vkCreateDescriptorSetLayout(m_Device.GetDevice(), &objectSetLayout, nullptr, &m_PerObjectDescriptorSetLayout), "Could not create per object descriptor set layout");

  //Model matrices will be handled through a push constant
  VkPushConstantRange pushConstant = {};
  pushConstant.offset = 0;
  pushConstant.size = sizeof(Mat4);
  pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  //Create pipeline layout for uniform data
  VkDescriptorSetLayout descSetLayouts[] = { m_PerFrameDescriptorSetLayout, m_PerObjectDescriptorSetLayout };
  VkPipelineLayoutCreateInfo pipelineCreate = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
  pipelineCreate.setLayoutCount = 2;
  pipelineCreate.pSetLayouts = descSetLayouts;
  pipelineCreate.pushConstantRangeCount = 1;
  pipelineCreate.pPushConstantRanges = &pushConstant;
  VKError::CheckResult(vkCreatePipelineLayout(m_Device.GetDevice(), &pipelineCreate, nullptr, &m_PipelineLayout), "Could not create graphics pipeline layout");

  //Create buffers for uniform data
  mCameraUBO.Setup(2 * sizeof(Mat4), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY, m_MemAllocator);
  mUsrDataUBO.Setup(sizeof(Mat4), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY, m_MemAllocator);
  mLightUBO.Setup(sizeof(LightData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY, m_MemAllocator);

  //Create descriptor set
  VkDescriptorSetLayout descriptorSetLayouts[] = {m_PerFrameDescriptorSetLayout, m_PerObjectDescriptorSetLayout, m_PerObjectDescriptorSetLayout, m_PerObjectDescriptorSetLayout };
  VkDescriptorSetAllocateInfo descSetAllocInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
  descSetAllocInfo.descriptorPool = m_Device.GetDescriptorPool();
  descSetAllocInfo.descriptorSetCount = 4;
  descSetAllocInfo.pSetLayouts = descriptorSetLayouts;

  std::vector<VkDescriptorSet> outSets(4);
  VKError::CheckResult(vkAllocateDescriptorSets(m_Device.GetDevice(), &descSetAllocInfo, outSets.data()), "Could not allocate global descriptor sets");

  m_PerFrameDescriptorSet = outSets[0];
  m_WorldFBDescriptorSet = outSets[1];
  m_UIFBDescriptorSet = outSets[2];
  m_FoveatedDescriptorSet = outSets[3];

  //Associate descriptor sets with buffers
  VkDescriptorBufferInfo cameraBufferInfo;
  cameraBufferInfo.buffer = mCameraUBO.GetBuffer();
  cameraBufferInfo.offset = 0;
  cameraBufferInfo.range = 2 * sizeof(Mat4);

  VkDescriptorBufferInfo usrDataBufferInfo;
  usrDataBufferInfo.buffer = mUsrDataUBO.GetBuffer();
  usrDataBufferInfo.offset = 0;
  usrDataBufferInfo.range = sizeof(Mat4);

  VkDescriptorImageInfo worldFBInfo = m_WorldFB.GetColorImageInfos(m_TextureSampler)[0];

  VkDescriptorImageInfo uiFBInfo = m_UIFB.GetColorImageInfos(m_TextureSampler)[0];

  VkDescriptorImageInfo shadowMapInfo = m_ShadowFB.GetDepthImageInfo(m_ShadowSampler);

  VkDescriptorImageInfo forveatedInfo = m_FoveatedFB.GetColorImageInfos(m_TextureSampler)[0];

  VkWriteDescriptorSet cameraWrite = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
  cameraWrite.dstSet = m_PerFrameDescriptorSet;
  cameraWrite.dstBinding = 0;
  cameraWrite.dstArrayElement = 0;
  cameraWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  cameraWrite.descriptorCount = 1;
  cameraWrite.pBufferInfo = &cameraBufferInfo;

  VkWriteDescriptorSet usrWrite = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
  usrWrite.dstSet = m_PerFrameDescriptorSet;
  usrWrite.dstBinding = 7;
  usrWrite.dstArrayElement = 0;
  usrWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  usrWrite.descriptorCount = 1;
  usrWrite.pBufferInfo = &usrDataBufferInfo;

  VkWriteDescriptorSet lightWrite = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
  lightWrite.dstSet = m_PerFrameDescriptorSet;
  lightWrite.dstBinding = 1;
  lightWrite.dstArrayElement = 0;
  lightWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  lightWrite.descriptorCount = 1;
  VkDescriptorBufferInfo lightInfo = mLightUBO.GetBufferInfo();
  lightWrite.pBufferInfo = &lightInfo;

  VkWriteDescriptorSet worldFBWrite = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
  worldFBWrite.dstSet = m_WorldFBDescriptorSet;
  worldFBWrite.dstBinding = 0;
  worldFBWrite.dstArrayElement = 0;
  worldFBWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  worldFBWrite.descriptorCount = 1;
  worldFBWrite.pImageInfo = &worldFBInfo;

  VkWriteDescriptorSet uiFBWrite = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
  uiFBWrite.dstSet = m_UIFBDescriptorSet;
  uiFBWrite.dstBinding = 0;
  uiFBWrite.dstArrayElement = 0;
  uiFBWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  uiFBWrite.descriptorCount = 1;
  uiFBWrite.pImageInfo = &uiFBInfo;

  VkWriteDescriptorSet fovWrite = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
  fovWrite.dstSet = m_FoveatedDescriptorSet;
  fovWrite.dstBinding = 0;
  fovWrite.dstArrayElement = 0;
  fovWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  fovWrite.descriptorCount = 1;
  fovWrite.pImageInfo = &forveatedInfo;

  VkWriteDescriptorSet shadowMapWrite = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
  shadowMapWrite.dstSet = m_PerFrameDescriptorSet;
  shadowMapWrite.dstBinding = 6;
  shadowMapWrite.dstArrayElement = 0;
  shadowMapWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  shadowMapWrite.descriptorCount = 1;
  shadowMapWrite.pImageInfo = &shadowMapInfo;

  VkWriteDescriptorSet descWrites[] = { cameraWrite, lightWrite, usrWrite, worldFBWrite, uiFBWrite, shadowMapWrite, fovWrite };

  vkUpdateDescriptorSets(m_Device.GetDevice(), 7, descWrites, 0, nullptr);

  //Create semaphores
  VkSemaphoreCreateInfo semaCreate = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
  VKError::CheckResult(vkCreateSemaphore(m_Device.GetDevice(), &semaCreate, nullptr, &m_ImageAvailable), "Could not create image available semaphore");
  VKError::CheckResult(vkCreateSemaphore(m_Device.GetDevice(), &semaCreate, nullptr, &m_RenderFinished), "Could not create render finished semaphore");
  VKError::CheckResult(vkCreateSemaphore(m_Device.GetDevice(), &semaCreate, nullptr, &m_ShadowToFoveated), "Could not create shadow to foveated semaphore");

  //Create fence for syncing with last frame
  VkFenceCreateInfo fenceCreate = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
  fenceCreate.flags = VK_FENCE_CREATE_SIGNALED_BIT;
  VKError::CheckResult(vkCreateFence(m_Device.GetDevice(), &fenceCreate, nullptr, &m_LastFrameFinished), "Could not make frame sync fence");

  //Create staging buffer to use for model and texture uploads
  m_StagingBuffer.Setup(STAGING_BUFFER_SIZE, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, m_MemAllocator);

  //Map camera and user data ubo for faster writes in draw loop
  mCameraUBO.Map(m_MemAllocator);
  mUsrDataUBO.Map(m_MemAllocator);
  mLightUBO.Map(m_MemAllocator);

  //Init imgui
  ImGui::CreateContext();
  ImGui_ImplVulkan_InitInfo imguiInit = {};
  imguiInit.Instance = m_Instance;
  imguiInit.PhysicalDevice = m_Device.GetPhysicalDevice();
  imguiInit.Device = m_Device.GetDevice();
  imguiInit.Queue = m_Device.GetGraphicsQueue();
  imguiInit.DescriptorPool = m_Device.GetDescriptorPool();

  ImGui_ImplVulkan_Init(&imguiInit, m_Surface.GetRenderPass());

  VkCommandBuffer imguiFontCmd = MakeOneTimeBuffer();
  ImGui_ImplVulkan_CreateFontsTexture(imguiFontCmd);
  SubmitOneTimeBuffer(m_Device.GetGraphicsQueue(), imguiFontCmd);

  ImGuiIO& io = ImGui::GetIO();
  io.DisplaySize.x = swapChainCapabilities.minImageExtent.width;
  io.DisplaySize.y = swapChainCapabilities.minImageExtent.height;

  m_ShadowShader = static_cast<VKShader*>(RenderFrontend::LoadShader("shadowpass.vert", "shadowpass.frag", DRAW_STAGE::SHADOW));

  //Create dummy image
  m_DummyImage = static_cast<VKTexture*>(LoadTexture(dummyImageData, 2, 2, 4));

  m_FoveatedClearShader = static_cast<VKShader*>(RenderFrontend::LoadShader("fbo.vert", "fbo_foveated.frag", DRAW_STAGE::FOVEATED));
}

void VKBackend::Shutdown() {
  vkDeviceWaitIdle(m_Device.GetDevice());
  ImGui_ImplVulkan_Shutdown();
  ImGui::DestroyContext();
  DeleteTexture(m_DummyImage);
  mCameraUBO.UnMap(m_MemAllocator);
  mUsrDataUBO.UnMap(m_MemAllocator);
  mLightUBO.UnMap(m_MemAllocator);
  vkDestroySemaphore(m_Device.GetDevice(), m_ImageAvailable, nullptr);
  vkDestroySemaphore(m_Device.GetDevice(), m_RenderFinished, nullptr);
  vkDestroySemaphore(m_Device.GetDevice(), m_ShadowToFoveated, nullptr);
  vkDestroyFence(m_Device.GetDevice(), m_LastFrameFinished, nullptr);
  m_Device.FreeCommandBuffers({m_WorldCmdBuffer, m_UICmdBuffer, m_PresentCmdBuffer, m_ShadowCmdBuffer, m_FoveatedCmdBuffer});
  vkDestroySampler(m_Device.GetDevice(), m_TextureSampler, nullptr);
  vkDestroySampler(m_Device.GetDevice(), m_ShadowSampler, nullptr);
  mCameraUBO.Destroy(m_MemAllocator);
  mUsrDataUBO.Destroy(m_MemAllocator);
  mLightUBO.Destroy(m_MemAllocator);
  m_StagingBuffer.UnMap(m_MemAllocator);
  m_StagingBuffer.Destroy(m_MemAllocator);
  vkDestroyDescriptorSetLayout(m_Device.GetDevice(), m_PerFrameDescriptorSetLayout, nullptr);
  vkDestroyDescriptorSetLayout(m_Device.GetDevice(), m_PerObjectDescriptorSetLayout, nullptr);
  vkDestroyPipelineLayout(m_Device.GetDevice(), m_PipelineLayout, nullptr);
  m_WorldFB.Destroy(m_Device.GetDevice(), m_MemAllocator);
  m_UIFB.Destroy(m_Device.GetDevice(), m_MemAllocator);
  m_ShadowFB.Destroy(m_Device.GetDevice(), m_MemAllocator);
  m_FoveatedFB.Destroy(m_Device.GetDevice(), m_MemAllocator);
  vmaDestroyAllocator(m_MemAllocator);
  m_Surface.Destroy(m_Instance, m_Device.GetDevice());
  m_Device.DestroyDevice();
  if (ENABLE_VALIDATION_LAYERS) {
    VKError::Shutdown(m_Instance);
  }
  vkDestroyInstance(m_Instance, nullptr);

  GazePointManager::FreeDevices();
}

const Model VKBackend::LoadModel(const std::vector<Vertex> vertices, const std::vector<u32> indices) {
  Model m;
  VKVertexBuffer *vBuffer = new VKVertexBuffer;

  //Allocate memory for vertices
  VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
  bufferInfo.size = vertices.size() * sizeof(Vertex) + indices.size() * sizeof(u32);
  bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VmaAllocationCreateInfo allocInfo = {};
  allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

  vmaCreateBuffer(m_MemAllocator, &bufferInfo, &allocInfo,
                  &vBuffer->m_Buffer, &vBuffer->m_Allocation, nullptr);

  m.mVBuffer = vBuffer;
  vBuffer->m_IndexOffset = vertices.size() * sizeof(Vertex);

  //Copy over vertex and index data
  void* data = m_StagingBuffer.Map(m_MemAllocator);
  void* indexData;
  memcpy(data, vertices.data(), (size_t)(vertices.size() * sizeof(Vertex)));

  indexData = static_cast<char*>(data) + vBuffer->m_IndexOffset;

  memcpy(indexData, indices.data(), (size_t)(indices.size() * sizeof(u32)));

  VkCommandBuffer copyCommand = MakeOneTimeBuffer();

  VkBufferCopy bufferCopy = {};
  bufferCopy.size = bufferInfo.size;
  bufferCopy.srcOffset = 0;
  bufferCopy.dstOffset = 0;
  vkCmdCopyBuffer(copyCommand, m_StagingBuffer.GetBuffer(), vBuffer->m_Buffer, 1, &bufferCopy);
  SubmitOneTimeBuffer(m_Device.GetGraphicsQueue(), copyCommand);
  return m;
}

Texture* VKBackend::LoadTexture(const unsigned char* data, const int width, const int height, const int numChannels) {
  VKTexture* texture = new VKTexture;
  
  //Setup image info
  if (width > 0 && height > 0) {

    VkFormat imageFormat;

    switch (numChannels) {
    default:
      imageFormat = VK_FORMAT_R8_UNORM;
      break;
    case 2:
      imageFormat = VK_FORMAT_R8G8_UNORM;
      break;
    case 3:
      imageFormat = VK_FORMAT_R8G8B8_UNORM;
      break;
    case 4:
      imageFormat = VK_FORMAT_R8G8B8A8_UNORM;
      break;
    }
    texture->m_Image.Setup((u32)width, (u32)height, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, imageFormat, VK_IMAGE_ASPECT_COLOR_BIT, m_Device.GetDevice(), m_MemAllocator);
    //Use staging buffer to upload data


    //Copy image into staging buffer
    void* dst_data = m_StagingBuffer.Map(m_MemAllocator);
    memcpy(dst_data, data, width * height * numChannels);

    //Transition image to transfer destination layout
    VkCommandBuffer transitionCmd = MakeOneTimeBuffer();

    VkImageMemoryBarrier barrier = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.image = texture->m_Image.GetImage();
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    vkCmdPipelineBarrier(transitionCmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0,  nullptr, 0, nullptr, 1, &barrier);

    //Copy staging buffer to image
    VkBufferImageCopy bufferToImage = {};
    bufferToImage.bufferOffset = 0;
    bufferToImage.bufferRowLength = 0;
    bufferToImage.bufferImageHeight = 0;
    bufferToImage.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    bufferToImage.imageSubresource.mipLevel = 0;
    bufferToImage.imageSubresource.baseArrayLayer = 0;
    bufferToImage.imageSubresource.layerCount = 1;
    bufferToImage.imageOffset = {0, 0, 0};
    bufferToImage.imageExtent = {(u32)width, (u32)height, 1};

    vkCmdCopyBufferToImage(transitionCmd, m_StagingBuffer.GetBuffer(), texture->m_Image.GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferToImage);

    //Transition image to optimal shader read layout
    VkImageMemoryBarrier barrier2 = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    barrier2.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier2.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier2.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier2.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier2.image = texture->m_Image.GetImage();
    barrier2.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier2.subresourceRange.baseMipLevel = 0;
    barrier2.subresourceRange.levelCount = 1;
    barrier2.subresourceRange.baseArrayLayer = 0;
    barrier2.subresourceRange.layerCount = 1;
    barrier2.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier2.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(transitionCmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier2);

    SubmitOneTimeBuffer(m_Device.GetGraphicsQueue(), transitionCmd);

    //Create descriptor set
    VkDescriptorSetAllocateInfo descSetAlloc = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
    descSetAlloc.descriptorPool = m_Device.GetDescriptorPool();
    descSetAlloc.descriptorSetCount = 1;
    descSetAlloc.pSetLayouts = &m_PerObjectDescriptorSetLayout;

    VKError::CheckResult(vkAllocateDescriptorSets(m_Device.GetDevice(), &descSetAlloc, &texture->m_TextureDescriptorSet), "Could not allocate texture descriptor set");

    //Update descriptor set to point to texture
    VkDescriptorImageInfo descImageInfo = {};
    descImageInfo.sampler = m_TextureSampler;
    descImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    descImageInfo.imageView = texture->m_Image.GetImageView();

    VkWriteDescriptorSet texWrite = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    texWrite.dstSet = texture->m_TextureDescriptorSet;
    texWrite.dstBinding = 0;
    texWrite.dstArrayElement = 0;
    texWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    texWrite.descriptorCount = 1;
    texWrite.pImageInfo = &descImageInfo;

    vkUpdateDescriptorSets(m_Device.GetDevice(), 1, &texWrite, 0, nullptr);
  } else     {
    texture->m_TextureDescriptorSet = VK_NULL_HANDLE;
  }

  texture->mHeight = (u32)height;
  texture->mWidth = (u32)width;

  return texture;
}

Shader* VKBackend::CreateShader(const std::vector<char> vertexProgram, const std::vector<char> fragmentProgram, const DRAW_STAGE stage) {
  VKShader* shader = new VKShader;
  VkShaderModule vertModule;
  VkShaderModule fragModule;

  //Create shader modules
  VkShaderModuleCreateInfo create = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
  create.codeSize = vertexProgram.size();
  create.pCode = reinterpret_cast<const u32*>(vertexProgram.data());

  VKError::CheckResult(vkCreateShaderModule(m_Device.GetDevice(), &create, nullptr, &vertModule), "Could not create vertex shader module");

  create.codeSize = fragmentProgram.size();
  create.pCode = reinterpret_cast<const u32*>(fragmentProgram.data());

  VKError::CheckResult(vkCreateShaderModule(m_Device.GetDevice(), &create, nullptr, &fragModule), "Could not create fragment shader module");

  VkRenderPass rp;
  VkExtent2D extent = m_Surface.GetSwapchainExtent();

  switch (stage) {
  case DRAW_STAGE::WORLD:
    rp = m_WorldFB.GetRenderPass();
    extent = {m_WorldFB.GetWidth(), m_WorldFB.GetHeight()};
    break;
  case DRAW_STAGE::UI:
    rp = m_UIFB.GetRenderPass();
    break;
  case DRAW_STAGE::ASPECT:
    rp = m_Surface.GetRenderPass();
    break;
  case DRAW_STAGE::SHADOW:
    rp = m_ShadowFB.GetRenderPass();
    extent = {m_ShadowSize, m_ShadowSize};
    break;
  case DRAW_STAGE::FOVEATED:
    rp = m_FoveatedFB.GetRenderPass();
    break;
  }

  shader->m_Pipeline = CreateGraphicsPipeline(vertModule, fragModule, rp, extent);

  vkDestroyShaderModule(m_Device.GetDevice(), vertModule, nullptr);
  vkDestroyShaderModule(m_Device.GetDevice(), fragModule, nullptr);
  return shader;
}

void VKBackend::SetFrameBufferModel(const Model &model) {
  m_FBModel = model;
}

void VKBackend::SetFrameBufferShader(Shader* shader, const DRAW_STAGE stage) {
  switch (stage) {
  case DRAW_STAGE::UI:
    m_UIFBShader = static_cast<VKShader*>(shader);
    break;
  case DRAW_STAGE::ASPECT:
    m_AspectShader = static_cast<VKShader*>(shader);
    break;
  default:
    Log::LogFatal("[VKBackend] - DRAW STAGE DOES NOT HAVE AN ASSOCIATED FRAMEBUFFER");
    break;
  }
}

void VKBackend::DeleteModel(Model &model) {
  vkDeviceWaitIdle(m_Device.GetDevice());
  VKVertexBuffer* vBuffer = static_cast<VKVertexBuffer*>(model.mVBuffer);
  vmaDestroyBuffer(m_MemAllocator, vBuffer->m_Buffer, vBuffer->m_Allocation);
}
void VKBackend::DeleteTexture(Texture* tex) {
  VKTexture* t = static_cast<VKTexture*>(tex);
  vkDeviceWaitIdle(m_Device.GetDevice());

  t->m_Image.Destroy(m_Device.GetDevice(), m_MemAllocator);
  if (t->m_TextureDescriptorSet != VK_NULL_HANDLE) {
    vkFreeDescriptorSets(m_Device.GetDevice(), m_Device.GetDescriptorPool(), 1, &t->m_TextureDescriptorSet);
  }
}

void VKBackend::DeleteShader(Shader* shader) {
  vkDeviceWaitIdle(m_Device.GetDevice());
  VKShader* s = static_cast<VKShader*>(shader);
  vkDestroyPipeline(m_Device.GetDevice(), s->m_Pipeline, nullptr);
}

std::string VKBackend::GetShaderFolderName() {
  return "VK";
}

DEPTH_MODE VKBackend::GetDepthMode() {
  return DEPTH_MODE::ZERO_TO_ONE;
}

VkPipeline VKBackend::CreateGraphicsPipeline(const VkShaderModule vertexModule, const VkShaderModule fragModule, const VkRenderPass renderpass, const VkExtent2D renderExtent) {
  VkPipelineShaderStageCreateInfo vertShaderStage = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
  vertShaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStage.module = vertexModule;
  vertShaderStage.pName = "main";

  VkPipelineShaderStageCreateInfo fragShaderStage = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
  fragShaderStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStage.module = fragModule;
  fragShaderStage.pName = "main";

  VkPipelineShaderStageCreateInfo stages[] = { vertShaderStage, fragShaderStage };

  //Setup vertex buffer bindings/attributes
  VkVertexInputBindingDescription vertexBindingDesc = {};
  vertexBindingDesc.binding = 0;
  vertexBindingDesc.stride = sizeof(Vertex);
  vertexBindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  VkVertexInputAttributeDescription vPosAttributeDesc = {};
  vPosAttributeDesc.binding = 0;
  vPosAttributeDesc.location = 0;
  vPosAttributeDesc.format = VK_FORMAT_R32G32B32_SFLOAT;
  vPosAttributeDesc.offset = (u32)offsetof(Vertex, mPosition);

  VkVertexInputAttributeDescription vNormalAttributeDesc = {};
  vNormalAttributeDesc.binding = 0;
  vNormalAttributeDesc.location = 1;
  vNormalAttributeDesc.format = VK_FORMAT_R32G32B32_SFLOAT;
  vNormalAttributeDesc.offset = (u32)offsetof(Vertex, mNormal);

  VkVertexInputAttributeDescription vTexCoordAttributeDesc = {};
  vTexCoordAttributeDesc.binding = 0;
  vTexCoordAttributeDesc.location = 2;
  vTexCoordAttributeDesc.format = VK_FORMAT_R32G32_SFLOAT;
  vTexCoordAttributeDesc.offset = (u32)offsetof(Vertex, mTexCoord);

  VkVertexInputAttributeDescription vColorAttributeDesc = {};
  vColorAttributeDesc.binding = 0;
  vColorAttributeDesc.location = 3;
  vColorAttributeDesc.format = VK_FORMAT_R32G32B32_SFLOAT;
  vColorAttributeDesc.offset = (u32)offsetof(Vertex, mColor);

  VkVertexInputAttributeDescription attributeDescs[] = { vPosAttributeDesc, vNormalAttributeDesc,
                                                         vTexCoordAttributeDesc, vColorAttributeDesc };
  VkPipelineVertexInputStateCreateInfo vertexInputInfo = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
  vertexInputInfo.vertexBindingDescriptionCount = 1;
  vertexInputInfo.pVertexBindingDescriptions = &vertexBindingDesc;
  vertexInputInfo.vertexAttributeDescriptionCount = 4;
  vertexInputInfo.pVertexAttributeDescriptions = attributeDescs;

  VkPipelineInputAssemblyStateCreateInfo inputAssembly = {VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  VkViewport viewport = {};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float)renderExtent.width;
  viewport.height = (float)renderExtent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor = {};
  scissor.offset = {0,0};
  scissor.extent = renderExtent;

  VkPipelineViewportStateCreateInfo viewportState = {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
  viewportState.viewportCount = 1;
  viewportState.scissorCount = 1;
  viewportState.pViewports = &viewport;
  viewportState.pScissors = &scissor;

  VkPipelineRasterizationStateCreateInfo raster = {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
  raster.depthClampEnable = VK_FALSE;
  raster.rasterizerDiscardEnable = VK_FALSE;
  raster.polygonMode = VK_POLYGON_MODE_FILL;
  raster.lineWidth = 1.0f;
  raster.cullMode = VK_CULL_MODE_NONE;
  raster.frontFace = VK_FRONT_FACE_CLOCKWISE;
  raster.depthBiasEnable = VK_FALSE;

  VkPipelineMultisampleStateCreateInfo multisample = {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
  multisample.sampleShadingEnable = VK_FALSE;
  multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  VkPipelineColorBlendAttachmentState blendingAttachment = {};
  blendingAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  blendingAttachment.blendEnable = VK_TRUE;
  blendingAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  blendingAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  blendingAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  blendingAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  blendingAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
  blendingAttachment.colorBlendOp = VK_BLEND_OP_ADD;

  VkPipelineColorBlendStateCreateInfo blending = {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
  blending.logicOpEnable = VK_FALSE;
  blending.attachmentCount = 1;
  blending.pAttachments = &blendingAttachment;

  VkPipelineDepthStencilStateCreateInfo depth = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
  depth.depthTestEnable = VK_TRUE;
  depth.depthWriteEnable = VK_TRUE;
  depth.depthCompareOp = VK_COMPARE_OP_LESS;
  depth.depthBoundsTestEnable = VK_FALSE;
  depth.stencilTestEnable = VK_FALSE;

  VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
  VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
  dynamicStateCreateInfo.dynamicStateCount = 2;
  dynamicStateCreateInfo.pDynamicStates = dynamicStates;

  VkGraphicsPipelineCreateInfo createInfo = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
  createInfo.stageCount = 2;
  createInfo.pStages = stages;
  createInfo.pVertexInputState = &vertexInputInfo;
  createInfo.pInputAssemblyState = &inputAssembly;
  createInfo.pViewportState = &viewportState;
  createInfo.pRasterizationState = &raster;
  createInfo.pMultisampleState = &multisample;
  createInfo.pDepthStencilState = &depth;
  createInfo.pColorBlendState = &blending;
  createInfo.layout = m_PipelineLayout;
  createInfo.renderPass = renderpass;
  createInfo.pDynamicState = &dynamicStateCreateInfo;
  createInfo.subpass = 0;

  VkPipeline ret;
  vkCreateGraphicsPipelines(m_Device.GetDevice(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &ret);
  return ret;
}

VkCommandBuffer VKBackend::MakeOneTimeBuffer() {
  //Allocate the command buffer
  VkCommandBuffer ret = m_Device.AllocateCommandBuffers(VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1)[0];

  VkCommandBufferBeginInfo begin = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
  begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(ret, &begin);

  return ret;
}

void VKBackend::SubmitOneTimeBuffer(VkQueue queue, VkCommandBuffer &command) {
  //End and submit the command buffer
  vkEndCommandBuffer(command);

  VkSubmitInfo submit = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
  submit.commandBufferCount = 1;
  submit.pCommandBuffers = &command;

  vkQueueSubmit(queue, 1, &submit, VK_NULL_HANDLE);

  vkQueueWaitIdle(queue);

  m_Device.FreeCommandBuffers({command});
}

static auto vector_getter = [](void* vec, int idx, const char** out_text) {
  auto& vector = *static_cast<std::vector<std::string>*>(vec);
  if (idx < 0 || idx >= static_cast<int>(vector.size())) { return false; }
  *out_text = vector.at(idx).c_str();
  return true;
};

void VKBackend::Draw(const Mat4 &viewMatrix, const Mat4 &projMatrix, const Mat4 &userData, const std::vector<Drawable> &scene, const std::vector<Drawable> &ui, const LightData& lights) {
  //Wait for last frame to finish rendering
  vkWaitForFences(m_Device.GetDevice(), 1, &m_LastFrameFinished, VK_TRUE, std::numeric_limits<u64>::max());
  vkResetFences(m_Device.GetDevice(), 1, &m_LastFrameFinished);
  //Reset and begin command buffer

  VkClearValue clearColor = {lights.mDirectionalLight.m_AmbientColor.r,
                             lights.mDirectionalLight.m_AmbientColor.g,
                             lights.mDirectionalLight.m_AmbientColor.b, 1.0f};
  VkClearValue clearDepth = {1.0f, 0};

  //Overall foveated rendering control
  static bool enableFoveatedRendering = true;
  ImGui::Begin("Foveated Rendering Settings");

  bool newFoveatedRendering = enableFoveatedRendering;
  if (ImGui::Checkbox("Enable Foveated Rendering", &newFoveatedRendering)) {
    enableFoveatedRendering = newFoveatedRendering;
  }
  ImGui::End();

  //Base pass resolution control
  static float baseResScale = 1.0f;

  if (enableFoveatedRendering) {
    ImGui::Begin("Base pass settings");
    ImGui::Text("Base pass resolution %ux%u", m_WorldFB.GetWidth(), m_WorldFB.GetHeight());
    ImGui::Text("Base pass resolution scale:");
  }

  //Foveated device selection
  if (enableFoveatedRendering) {
    ImGui::Begin("Device Selection");

    int currentSelection = GazePointManager::GetCurrentIndex();
    int newSelection = currentSelection;

    std::vector<std::string> devices(GazePointManager::GetDeviceCount());

    for (int i = 0; i < GazePointManager::GetDeviceCount(); i++) {
      devices[i] = GazePointManager::GetDeviceName(i);
    }
    ImGui::Combo("", &newSelection, vector_getter, static_cast<void*>(&devices), devices.size());

    if (newSelection != currentSelection) {
      GazePointManager::SelectDevice(newSelection);
    }

    ImGui::End();
  }


  const float MIN_BASE_RES = 5.0;
  const float MAX_BASE_RES = 100.0;
  float newBaseResScale = baseResScale * 100.0f;

  if (!enableFoveatedRendering) {
    baseResScale = 1.0f;
  }

  if (enableFoveatedRendering) {
    if (ImGui::SliderFloat("", &newBaseResScale, MIN_BASE_RES, MAX_BASE_RES, "%f%%")) {
      if (newBaseResScale < MIN_BASE_RES) {
        newBaseResScale = MIN_BASE_RES;
      }

      if (newBaseResScale > MAX_BASE_RES) {
        newBaseResScale = MAX_BASE_RES;
      }
    }
  }

  if (enableFoveatedRendering) {
    ImGui::End();
  }


  if (baseResScale != newBaseResScale / 100.0f) {
    baseResScale = enableFoveatedRendering ? newBaseResScale / 100.0f : 1.0f;

    m_WorldFB.Destroy(m_Device.GetDevice(), m_MemAllocator);
    m_WorldFB.Setup(m_FoveatedFB.GetWidth() * baseResScale, m_FoveatedFB.GetHeight() * baseResScale, {m_Surface.GetDefaultFormat().format}, VK_FORMAT_D32_SFLOAT, false, m_Device.GetDevice(), m_MemAllocator);
    VkDescriptorImageInfo worldFBInfo = m_WorldFB.GetColorImageInfos(m_TextureSampler)[0];

    VkWriteDescriptorSet worldFBWrite = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    worldFBWrite.dstSet = m_WorldFBDescriptorSet;
    worldFBWrite.dstBinding = 0;
    worldFBWrite.dstArrayElement = 0;
    worldFBWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    worldFBWrite.descriptorCount = 1;
    worldFBWrite.pImageInfo = &worldFBInfo;

    vkUpdateDescriptorSets(m_Device.GetDevice(), 1, &worldFBWrite, 0, nullptr);
  }


  //Setup camera information
  Mat4 vkProj = projMatrix;
  vkProj[1][1] *= -1;
  Mat4 matrices[] = { viewMatrix, vkProj };
  void* data = mCameraUBO.Map(m_MemAllocator);
  memcpy(data, matrices, 2 * sizeof(Mat4));

  //Setup user data information
  data = mUsrDataUBO.Map(m_MemAllocator);
  Mat4 modUserData = userData;
  modUserData[3] = Vec4(lights.mDirectionalLight.m_AmbientColor.r, lights.mDirectionalLight.m_AmbientColor.g, lights.mDirectionalLight.m_AmbientColor.b, 1.0f);
  memcpy(data, &modUserData, sizeof(Mat4));

  //Copy lighting info
  LightData shadowedLightData = lights;
  Vec3 dLightPosition = -1.0f * Vec3(20.0f * lights.mDirectionalLight.m_Direction.x,
                             20.0f * lights.mDirectionalLight.m_Direction.y,
                             20.0f * lights.mDirectionalLight.m_Direction.z);

  Vec3 up = Vec3(0.0f, 1.0f, 0.0f);

  if (dLightPosition.x == 0.0f && dLightPosition.z == 0.0f && dLightPosition.y != 0.0f) {
    up = Vec3(1.0f, 0.0f, 0.0f);
  }
  Mat4 lightView = glm::lookAt(dLightPosition, Vec3(0.0f, 0.0f, 0.0f), up);
  Mat4 lightProjection = glm::orthoZO(-20.0f, 20.0f, 20.0f, -20.0f, 1.0f, 100.0f);
  shadowedLightData.mDirectionalLight.m_LightSpaceMatrix = lightProjection * lightView;
  data = mLightUBO.Map(m_MemAllocator);
  memcpy(data, &shadowedLightData, sizeof(LightData));

  VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  //Create shadow maps
  vkBeginCommandBuffer(m_ShadowCmdBuffer, &beginInfo);

  {
    VkViewport shadowViewport = {};
    shadowViewport.x = 0.0f;
    shadowViewport.y = 0.0f;
    shadowViewport.width = (float)(m_ShadowSize);
    shadowViewport.height = (float)(m_ShadowSize);
    shadowViewport.minDepth = 0.0f;
    shadowViewport.maxDepth = 1.0f;
    vkCmdSetViewport(m_ShadowCmdBuffer, 0, 1, &shadowViewport);

    VkRect2D shadowScissor = {};
    shadowScissor.offset = {0,0};
    shadowScissor.extent = {m_ShadowSize, m_ShadowSize};
    vkCmdSetScissor(m_ShadowCmdBuffer, 0, 1, &shadowScissor);
  }

  VkRenderPassBeginInfo shadowBegin = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
  shadowBegin.renderPass = m_ShadowFB.GetRenderPass();
  shadowBegin.framebuffer = m_ShadowFB.GetFramebuffer();
  shadowBegin.renderArea.offset = {0,0};
  shadowBegin.renderArea.extent = {m_ShadowSize, m_ShadowSize};
  shadowBegin.clearValueCount = 1;
  shadowBegin.pClearValues = &clearDepth;

  vkCmdBeginRenderPass(m_ShadowCmdBuffer, &shadowBegin, VK_SUBPASS_CONTENTS_INLINE);
  vkCmdBindPipeline(m_ShadowCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_ShadowShader->m_Pipeline);


  for (const auto& m : scene) {
    VKVertexBuffer * vBuffer = static_cast<VKVertexBuffer*>(m.mVBuffer);

    if (m.mTexture == nullptr) {
      vkCmdBindDescriptorSets(m_ShadowCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 1, 1, &m_DummyImage->m_TextureDescriptorSet, 0, nullptr);
    } else {
      VKTexture* texture = static_cast<VKTexture*>(m.mTexture);
      vkCmdBindDescriptorSets(m_ShadowCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 1, 1, &texture->m_TextureDescriptorSet, 0, nullptr);
    }
    Mat4 modelMatrix = lightProjection * lightView * m.mTransformMatrix;

    vkCmdPushConstants(m_ShadowCmdBuffer, m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Mat4), glm::value_ptr(modelMatrix));

    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(m_ShadowCmdBuffer, 0, 1, &vBuffer->m_Buffer, offsets);
    vkCmdBindIndexBuffer(m_ShadowCmdBuffer, vBuffer->m_Buffer, vBuffer->m_IndexOffset, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(m_ShadowCmdBuffer, m.mNumFaces * 3, 1, 0, 0, 0);
  }

  vkCmdEndRenderPass(m_ShadowCmdBuffer);
  vkEndCommandBuffer(m_ShadowCmdBuffer);

  VkSemaphore shadowSemaphore = m_ShadowFB.GetSemaphore();
  VkSemaphore shadowSignalSemaphores[] = {m_ShadowFB.GetSemaphore(), m_ShadowToFoveated};
  VkSubmitInfo shadowSubmit = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
  shadowSubmit.commandBufferCount = 1;
  shadowSubmit.pCommandBuffers = &m_ShadowCmdBuffer;
  shadowSubmit.signalSemaphoreCount = 2;
  shadowSubmit.pSignalSemaphores = shadowSignalSemaphores;

  vkQueueSubmit(m_Device.GetGraphicsQueue(), 1, &shadowSubmit, VK_NULL_HANDLE);

  vkBeginCommandBuffer(m_WorldCmdBuffer, &beginInfo);

  {
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)(m_WorldFB.GetWidth());
    viewport.height = (float)(m_WorldFB.GetHeight());
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(m_WorldCmdBuffer, 0, 1, &viewport);

    VkRect2D scissor = {};
    scissor.offset = {0,0};
    scissor.extent = {m_WorldFB.GetWidth(), m_WorldFB.GetHeight()};
    vkCmdSetScissor(m_WorldCmdBuffer, 0, 1, &scissor);
  }

  //Startup 1st renderpass for 3D world
  VkRenderPassBeginInfo worldBeginInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
  worldBeginInfo.renderPass = m_WorldFB.GetRenderPass();
  worldBeginInfo.framebuffer = m_WorldFB.GetFramebuffer();

  worldBeginInfo.renderArea.offset = {0,0};
  worldBeginInfo.renderArea.extent = { m_WorldFB.GetWidth(), m_WorldFB.GetHeight() };

  VkClearValue clears[] = {clearColor, clearDepth};
  worldBeginInfo.clearValueCount = 2;
  worldBeginInfo.pClearValues = clears;

  vkCmdBeginRenderPass(m_WorldCmdBuffer, &worldBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
  vkCmdBindDescriptorSets(m_WorldCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &m_PerFrameDescriptorSet, 0, nullptr);

  //Draw objects
  for(const auto &model : scene) {
    DrawModel(model, m_WorldCmdBuffer);
  }

  //End renderpass and setup sync with next pass
  vkCmdEndRenderPass(m_WorldCmdBuffer);
  vkEndCommandBuffer(m_WorldCmdBuffer);

  VkSemaphore worldSemaphore[] = {m_WorldFB.GetSemaphore()};

  VkSubmitInfo worldSubmit = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
  VkPipelineStageFlags shadowWaitStages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  worldSubmit.waitSemaphoreCount = 1;
  worldSubmit.pWaitSemaphores = &shadowSemaphore;
  worldSubmit.pWaitDstStageMask = &shadowWaitStages;
  worldSubmit.signalSemaphoreCount = 1;
  worldSubmit.pSignalSemaphores = worldSemaphore;
  worldSubmit.commandBufferCount = 1;
  worldSubmit.pCommandBuffers = &m_WorldCmdBuffer;
  vkQueueSubmit(m_Device.GetGraphicsQueue(), 1, &worldSubmit, VK_NULL_HANDLE);

  //Draw same scene in foveated command buffer
  {
    vkBeginCommandBuffer(m_FoveatedCmdBuffer, &beginInfo);

    {
      VkViewport viewport = {};
      viewport.x = 0.0f;
      viewport.y = 0.0f;
      viewport.width = (float)(m_FoveatedFB.GetWidth());
      viewport.height = (float)(m_FoveatedFB.GetHeight());
      viewport.minDepth = 0.0f;
      viewport.maxDepth = 1.0f;
      vkCmdSetViewport(m_FoveatedCmdBuffer, 0, 1, &viewport);

      GVec2 gazepoint = GazePointManager::GetGazePoint();

      const u32 MAX_FOVEATED_SIZE = 2400;
      const u32 MIN_FOVEATED_SIZE = 120;

      static u32 foveatedSize = 2 * MIN_FOVEATED_SIZE;

      u32 pixelX = gazepoint.x * m_FoveatedFB.GetWidth();
      u32 pixelY = gazepoint.y * m_FoveatedFB.GetHeight();

      int topleftX = pixelX - (foveatedSize / 2);
      int topleftY = pixelY - (foveatedSize / 2);

      int bottomRightX = topleftX + foveatedSize;
      int bottomRightY = topleftY + foveatedSize;

      //Clamp rectangle to window boundaries
      if (topleftX < 0) {
        topleftX = 0;
      }

      if (topleftY < 0) {
        topleftY = 0;
      }

      if (bottomRightX > m_FoveatedFB.GetWidth()) {
        bottomRightX = m_FoveatedFB.GetWidth();
      }

      if (bottomRightY > m_FoveatedFB.GetHeight()) {
        bottomRightY = m_FoveatedFB.GetHeight();
      }

      VkRect2D scissor = {};
      scissor.offset.x = topleftX;
      scissor.offset.y = topleftY;
      scissor.extent = {(u32)(bottomRightX - topleftX), (u32)(bottomRightY - topleftY)};
      vkCmdSetScissor(m_FoveatedCmdBuffer, 0, 1, &scissor);

      //Do any backend related ImGUI stuff
      if (enableFoveatedRendering) {
        ImGui::Begin("Foveated Square Settings");

        int newSize = foveatedSize;

        ImGui::Text("Gaze Coordinates { %f, %f }", gazepoint.x, gazepoint.y);
        ImGui::Text("Foveated Square Size:");
        if (ImGui::SliderInt("", &newSize, MIN_FOVEATED_SIZE, MAX_FOVEATED_SIZE, "%dpx")) {
          //Clamp the new size before storing it
          if (newSize < MIN_FOVEATED_SIZE) {
            newSize = MIN_FOVEATED_SIZE;
          }

          if (newSize > MAX_FOVEATED_SIZE) {
            newSize = MAX_FOVEATED_SIZE;
          }

          foveatedSize = newSize;
        }

        ImGui::End();
      }
    }

    VkClearValue fovClear = {lights.mDirectionalLight.m_AmbientColor.r,
                               lights.mDirectionalLight.m_AmbientColor.g,
                               lights.mDirectionalLight.m_AmbientColor.b, 0.0f};

    VkClearValue fovClears[] = {fovClear, clearDepth};

    VkRenderPassBeginInfo fovRenderPass = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    fovRenderPass.renderPass = m_FoveatedFB.GetRenderPass();
    fovRenderPass.framebuffer = m_FoveatedFB.GetFramebuffer();
    fovRenderPass.renderArea.offset = {0,0};
    fovRenderPass.renderArea.extent = {m_FoveatedFB.GetWidth(), m_FoveatedFB.GetHeight()};
    fovRenderPass.clearValueCount = 2;
    fovRenderPass.pClearValues = fovClears;

    vkCmdBeginRenderPass(m_FoveatedCmdBuffer, &fovRenderPass, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindDescriptorSets(m_FoveatedCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &m_PerFrameDescriptorSet, 0, nullptr);

    if (enableFoveatedRendering) {
      DrawFrameBuffer(m_FoveatedCmdBuffer, m_FoveatedClearShader->m_Pipeline, m_DummyImage->m_TextureDescriptorSet);

      //Draw objects
      for(const auto &model : scene) {
        DrawModel(model, m_FoveatedCmdBuffer);
      }
    }


    //End renderpass and setup sync with next pass
    vkCmdEndRenderPass(m_FoveatedCmdBuffer);
    vkEndCommandBuffer(m_FoveatedCmdBuffer);

    VkSemaphore semaphore[] = {m_FoveatedFB.GetSemaphore()};

    VkSubmitInfo submit = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = &m_ShadowToFoveated;
    submit.pWaitDstStageMask = &waitStages;
    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = semaphore;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &m_FoveatedCmdBuffer;
    vkQueueSubmit(m_Device.GetGraphicsQueue(), 1, &submit, VK_NULL_HANDLE);
  }

  //Startup 2nd renderpass for UI
  vkBeginCommandBuffer(m_UICmdBuffer, &beginInfo);

  {
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)(m_UIFB.GetWidth());
    viewport.height = (float)(m_UIFB.GetHeight());
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(m_UICmdBuffer, 0, 1, &viewport);

    VkRect2D scissor = {};
    scissor.offset = {0,0};
    scissor.extent = {m_UIFB.GetWidth(), m_UIFB.GetHeight()};
    vkCmdSetScissor(m_UICmdBuffer, 0, 1, &scissor);
  }

  VkRenderPassBeginInfo uiBeginInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
  uiBeginInfo.renderPass = m_UIFB.GetRenderPass();
  uiBeginInfo.framebuffer = m_UIFB.GetFramebuffer();
  uiBeginInfo.renderArea.offset = {0,0};
  uiBeginInfo.renderArea.extent = m_Surface.GetSwapchainExtent();
  uiBeginInfo.clearValueCount = 2;
  uiBeginInfo.pClearValues = clears;

  vkCmdBeginRenderPass(m_UICmdBuffer, &uiBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
  vkCmdBindDescriptorSets(m_UICmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &m_PerFrameDescriptorSet, 0, nullptr);

  //Draw framebuffer
  DrawFrameBuffer(m_UICmdBuffer, m_UIFBShader->m_Pipeline, m_WorldFBDescriptorSet);
  DrawFrameBuffer(m_UICmdBuffer, m_UIFBShader->m_Pipeline, m_FoveatedDescriptorSet);

  //Draw objects
  for(const auto &model : ui) {
    DrawModel(model, m_UICmdBuffer);
  }

  //Startup 3rd renderpass for aspect correction
  vkCmdEndRenderPass(m_UICmdBuffer);
  vkEndCommandBuffer(m_UICmdBuffer);

  VkSemaphore uiWaitSemaphores[] = {m_WorldFB.GetSemaphore(), m_FoveatedFB.GetSemaphore()};

  VkSubmitInfo uiSubmit = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
  uiSubmit.commandBufferCount = 1;
  uiSubmit.pCommandBuffers = &m_UICmdBuffer;
  uiSubmit.waitSemaphoreCount = 2;
  uiSubmit.pWaitSemaphores = uiWaitSemaphores;

  VkSemaphore uiSemaphore[] = {m_UIFB.GetSemaphore()};
  VkPipelineStageFlags uiWaitStages[] = { VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT};
  uiSubmit.pWaitDstStageMask = uiWaitStages;
  uiSubmit.signalSemaphoreCount = 1;
  uiSubmit.pSignalSemaphores = uiSemaphore;

  vkQueueSubmit(m_Device.GetGraphicsQueue(), 1, &uiSubmit, VK_NULL_HANDLE);

  u32 imgIndex;
  vkAcquireNextImageKHR(m_Device.GetDevice(), m_Surface.GetSwapchain(), std::numeric_limits<u64>::max(), m_ImageAvailable, VK_NULL_HANDLE, &imgIndex);
  vkBeginCommandBuffer(m_PresentCmdBuffer, &beginInfo);

  {
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)(m_Surface.GetSwapchainExtent().width);
    viewport.height = (float)(m_Surface.GetSwapchainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(m_PresentCmdBuffer, 0, 1, &viewport);

    VkRect2D scissor = {};
    scissor.offset = {0,0};
    scissor.extent = m_Surface.GetSwapchainExtent();
    vkCmdSetScissor(m_PresentCmdBuffer, 0, 1, &scissor);
  }

  VkRenderPassBeginInfo aspectBegin = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
  aspectBegin.renderPass = m_Surface.GetRenderPass();
  aspectBegin.framebuffer = m_Surface.GetFramebuffer(imgIndex);
  aspectBegin.renderArea.offset = { 0,0 };
  aspectBegin.renderArea.extent = m_Surface.GetSwapchainExtent();
  aspectBegin.clearValueCount = 1;
  aspectBegin.pClearValues = &clearColor;

  vkCmdBeginRenderPass(m_PresentCmdBuffer, &aspectBegin, VK_SUBPASS_CONTENTS_INLINE);
  vkCmdBindDescriptorSets(m_PresentCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &m_PerFrameDescriptorSet, 0, nullptr);
  
  //Draw FB image
  DrawFrameBuffer(m_PresentCmdBuffer, m_AspectShader->m_Pipeline, m_UIFBDescriptorSet);

  //Render ImGui data
  ImGui::Render();
  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), m_PresentCmdBuffer);

  //Finish command buffer
  vkCmdEndRenderPass(m_PresentCmdBuffer);
  vkEndCommandBuffer(m_PresentCmdBuffer);

  //Setup semaphore for syncing with presentation
  VkSubmitInfo aspectSubmit = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
  aspectSubmit.commandBufferCount = 1;
  aspectSubmit.pCommandBuffers = &m_PresentCmdBuffer;
  aspectSubmit.waitSemaphoreCount = 2;

  VkSemaphore aspectWaitSemaphores[] = { m_UIFB.GetSemaphore(), m_ImageAvailable };
  aspectSubmit.pWaitSemaphores = aspectWaitSemaphores;

  VkPipelineStageFlags aspectWaitStages[] = { VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
  aspectSubmit.pWaitDstStageMask = aspectWaitStages;
  aspectSubmit.signalSemaphoreCount = 1;
  aspectSubmit.pSignalSemaphores = &m_RenderFinished;

  //Submit commands
  vkQueueSubmit(m_Device.GetGraphicsQueue(), 1, &aspectSubmit, m_LastFrameFinished);

  //Setup present
  VkSwapchainKHR swapchains[] = { m_Surface.GetSwapchain() };
  VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = &m_RenderFinished;
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapchains;
  presentInfo.pImageIndices = &imgIndex;
  //Submit presentation to queue
  vkQueuePresentKHR(m_Device.GetPresentQueue(), &presentInfo);
}

void VKBackend::DrawModel(const Drawable &d, VkCommandBuffer cmdBfr) {
  VKShader* shader = static_cast<VKShader*>(d.mShader);
  VKVertexBuffer* vBuffer = static_cast<VKVertexBuffer*>(d.mVBuffer);
  VKTexture* texture = static_cast<VKTexture*>(d.mTexture);

  vkCmdBindPipeline(cmdBfr, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->m_Pipeline);

  if (texture != nullptr) {
    vkCmdBindDescriptorSets(cmdBfr, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 1, 1, &texture->m_TextureDescriptorSet, 0, nullptr);
  }

  vkCmdPushConstants(cmdBfr, m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Mat4), glm::value_ptr(d.mTransformMatrix));

  VkDeviceSize offsets[] = { 0 };
  vkCmdBindVertexBuffers(cmdBfr, 0, 1, &vBuffer->m_Buffer, offsets);
  vkCmdBindIndexBuffer(cmdBfr, vBuffer->m_Buffer, vBuffer->m_IndexOffset, VK_INDEX_TYPE_UINT32);
  vkCmdDrawIndexed(cmdBfr, d.mNumFaces * 3, 1, 0, 0, 0);
}

void VKBackend::DrawFrameBuffer(VkCommandBuffer cmdBfr, VkPipeline pipeline, VkDescriptorSet descSet) {
  VKVertexBuffer* vBuf = static_cast<VKVertexBuffer*>(m_FBModel.mVBuffer);
  VkDeviceSize offsets[] = { 0 };
  vkCmdBindPipeline(cmdBfr, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
  vkCmdBindDescriptorSets(cmdBfr, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 1, 1, &descSet, 0, nullptr);
  vkCmdBindVertexBuffers(cmdBfr, 0, 1, &vBuf->m_Buffer, offsets);
  vkCmdBindIndexBuffer(cmdBfr, vBuf->m_Buffer, vBuf->m_IndexOffset, VK_INDEX_TYPE_UINT32);
  vkCmdDrawIndexed(cmdBfr, m_FBModel.mNumFaces * 3, 1, 0, 0, 0);
}
std::string VKBackend::GetDeviceName() {
  VkPhysicalDeviceProperties deviceProperties = m_Device.GetDeviceProperties();
  return std::string(deviceProperties.deviceName);
}
u64 VKBackend::GetUsedVRAM() {
  VmaStats memStats;
  vmaCalculateStats(m_MemAllocator, &memStats);
  return memStats.total.usedBytes;
}

#pragma once
#include <vulkan/vulkan.h>
#include <string>

class VKError {
public:
  static void Setup(VkInstance instance, VkDebugReportFlagsEXT flags);
  static void Shutdown(VkInstance instance);
  static void CheckResult(VkResult result, std::string errorMessage);
private:
  static std::string VkResultToString(VkResult result);
  static VkDebugReportCallbackEXT m_DbgCallback;
};

#pragma once

#include <vector>
#include <string>
#include <cstdint>

// Forward declarations
struct VkInstance_T;
struct VkPhysicalDevice_T;
struct VkSurfaceKHR_T;

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#elif defined(__APPLE__)
// For macOS, we'll use MoltenVK, which requires specific headers and definitions.
// MoltenVK provides Vulkan-compatible API, but also might need its own setup.
// We'll include MoltenVK's Vulkan headers.
#define VK_USE_PLATFORM_METAL_EXT
#define VK_USE_PLATFORM_MACOS_MVK
#elif defined(__linux__)
#define VK_USE_PLATFORM_XCB_KHR
#endif

// Include Vulkan headers.
// If MoltenVK is correctly installed, these headers should work for macOS too.
#include <vulkan/vulkan.h>

// MoltenVK headers are optional; standard Vulkan headers are sufficient for surface creation APIs.
#ifdef VK_USE_PLATFORM_MACOS_MVK
#if __has_include(<MoltenVK/MoltenVK.h>)
#include <MoltenVK/MoltenVK.h>
#endif
#endif

namespace lastchaos::vulkan {

// Enum to represent different operating systems for Vulkan context
enum class VulkanOS {
    Windows,
    macOS,
    Linux,
    Unknown
};

// Structure to hold Vulkan instance and device information
struct VulkanContext {
    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE; // Window surface for rendering
    uint32_t graphicsQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    uint32_t presentQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    VulkanOS os = VulkanOS::Unknown;

    // Function to initialize Vulkan instance, physical device, and surface
    bool initialize(const std::string& appName, void* windowHandle);

    // Function to destroy Vulkan resources
    void destroy();

    // Get the VulkanOS based on the current platform
    static VulkanOS detectVulkanOS();
};

} // namespace lastchaos::vulkan

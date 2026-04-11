#include "VulkanContext.h"

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <stdexcept> // For std::runtime_error
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#elif defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#include <dlfcn.h> // For dlsym
#endif

// Include Vulkan loader/functions dynamically if needed for cross-platform compatibility.
// For simplicity here, we assume Vulkan headers and loader are available.
// In a real-world scenario, you might need to load Vulkan functions dynamically.

namespace lastchaos::vulkan {

VulkanOS VulkanContext::detectVulkanOS() {
#if defined(_WIN32)
    return VulkanOS::Windows;
#elif defined(__APPLE__)
    return VulkanOS::macOS;
#elif defined(__linux__)
    return VulkanOS::Linux;
#else
    return VulkanOS::Unknown;
#endif
}

bool VulkanContext::initialize(const std::string& appName, void* windowHandle) {
    os = detectVulkanOS();

    // --- Vulkan Instance Creation ---
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = appName.c_str();
    appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0); // Example version
    appInfo.pEngineName = "LastChaosEngine";
    appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0); // Example version
    appInfo.apiVersion = VK_API_VERSION_1_3; // Requesting Vulkan 1.3

    VkInstanceCreateInfo instanceInfo = {};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &appInfo;

    std::vector<const char*> instanceExtensions;
    std::vector<const char*> validationLayers;

    // --- Platform-specific Instance Extensions ---
#ifdef VK_USE_PLATFORM_WIN32_KHR
    if (os == VulkanOS::Windows) {
        instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
    }
#endif
#ifdef VK_USE_PLATFORM_MACOS_MVK
    if (os == VulkanOS::macOS) {
        // MoltenVK requires VK_EXT_metal_surface
        instanceExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME); // Required for MoltenVK on some setups
        instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME); // Often needed
        instanceExtensions.push_back(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
        // Some validation layers might be useful, but avoid strict requirements for now.
        // validationLayers.push_back("VK_LAYER_KHRONOS_validation");
    }
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
    if (os == VulkanOS::Linux) {
        instanceExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
    }
#endif

    // Add standard instance extensions (e.g., for validation layers)
    instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    // For MoltenVK, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME is often required.
    if (os == VulkanOS::macOS) {
         if (std::find(instanceExtensions.begin(), instanceExtensions.end(), VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME) == instanceExtensions.end()) {
            instanceExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        }
    }


    instanceInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
    instanceInfo.ppEnabledExtensionNames = instanceExtensions.data();

    // Optional: Enable validation layers for debugging
    // This requires Vulkan SDK to be installed with validation layers.
    // For cross-platform, it's better to only enable them if available.
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    bool enableValidation = false;
    for (const char* layerName : validationLayers) {
        bool found = false;
        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                found = true;
                break;
            }
        }
        if (found) {
            instanceExtensions.push_back(layerName);
            enableValidation = true;
        } else {
            std::cerr << "Warning: Validation layer " << layerName << " not found." << std::endl;
        }
    }
    // Re-set extension count if validation layers were added
    instanceInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
    instanceInfo.ppEnabledExtensionNames = instanceExtensions.data();


    VkResult result = vkCreateInstance(&instanceInfo, nullptr, &instance);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to create Vulkan instance! VkResult: " << result << std::endl;
        return false;
    }

    // --- Physical Device Selection ---
    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
    if (physicalDeviceCount == 0) {
        std::cerr << "No Vulkan physical devices found!" << std::endl;
        destroy();
        return false;
    }

    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());

    bool foundDevice = false;
    for (const auto& device : physicalDevices) {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(device, &properties);

        // Basic check: Prefer discrete GPUs, check for Vulkan support
        bool isDiscreteGPU = (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
        bool hasGraphicsQueue = false;
        bool hasPresentQueue = false;

        uint32_t queueFamilyCount;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        for (uint32_t i = 0; i < queueFamilyCount; ++i) {
            // Check for graphics queue support
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                hasGraphicsQueue = true;
                graphicsQueueFamilyIndex = i;

                // Check for presentation support
#ifdef VK_USE_PLATFORM_WIN32_KHR
                if (os == VulkanOS::Windows) {
                    VkBool32 presentSupport = false;
                    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
                    if (presentSupport) {
                        hasPresentQueue = true;
                        presentQueueFamilyIndex = i;
                        break; // Found suitable queues
                    }
                }
#endif
#ifdef VK_USE_PLATFORM_MACOS_MVK
                if (os == VulkanOS::macOS) {
                    // On macOS with MoltenVK, we can often use the same queue for graphics and presentation
                    // or a dedicated presentation queue if available.
                    // For simplicity, let's assume graphics queue can also present.
                    hasPresentQueue = true; // Assume graphics queue can present
                    presentQueueFamilyIndex = i;
                    // If a dedicated present queue is needed, more complex logic would be here.
                    break; // Found suitable queues
                }
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
                if (os == VulkanOS::Linux) {
                     VkBool32 presentSupport = false;
                    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
                    if (presentSupport) {
                        hasPresentQueue = true;
                        presentQueueFamilyIndex = i;
                        break; // Found suitable queues
                    }
                }
#endif
            }
        }

        if (hasGraphicsQueue && hasPresentQueue) {
            physicalDevice = device;
            foundDevice = true;
            // Prefer discrete GPU if available, otherwise take the first suitable one.
            if (isDiscreteGPU) {
                break;
            }
        }
    }

    if (!foundDevice) {
        std::cerr << "No suitable Vulkan physical device found!" << std::endl;
        destroy();
        return false;
    }

    // --- Window Surface Creation ---
    VkResult surfaceResult = VK_SUCCESS;

    switch (os) {
#ifdef VK_USE_PLATFORM_WIN32_KHR
        case VulkanOS::Windows:
            VkWin32SurfaceCreateInfoKHR win32SurfaceInfo = {};
            win32SurfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
            win32SurfaceInfo.hwnd = static_cast<HWND>(windowHandle);
            surfaceResult = vkCreateWin32SurfaceKHR(instance, &win32SurfaceInfo, nullptr, &surface);
            break;
#endif
#ifdef VK_USE_PLATFORM_MACOS_MVK
        case VulkanOS::macOS: {
            // For macOS, we need a CAMetalLayer. The windowHandle should be a pointer to it.
            // Alternatively, it might be a pointer to a NSWindow, from which we get the CAMetalLayer.
            // This part is highly dependent on how the windowing is set up.
            // Assuming windowHandle points to a CAMetalLayer* for now.
            // If it's NSWindow*, we'd need to get the layer: `[static_cast<NSWindow*>(windowHandle) contentView]`
            
            // A more robust approach might be to pass the CAMetalLayer directly.
            // For simplicity, let's assume windowHandle is a void pointer to a CAMetalLayer object.
            CAMetalLayer* metalLayer = static_cast<CAMetalLayer*>(windowHandle);
            if (!metalLayer) {
                 std::cerr << "Error: Invalid window handle (CAMetalLayer*) provided for macOS." << std::endl;
                 destroy();
                 return false;
            }

            VkMetalSurfaceCreateInfoEXT mtlSurfaceInfo = {};
            mtlSurfaceInfo.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
            mtlSurfaceInfo.pLayer = metalLayer;
            
            // vkCreateMetalSurfaceEXT is the function provided by MoltenVK's Vulkan loader
            // We need to get this function pointer.
            PFN_vkCreateMetalSurfaceEXT vkCreateMetalSurfaceEXT = reinterpret_cast<PFN_vkCreateMetalSurfaceEXT>(vkGetInstanceProcAddr(instance, "vkCreateMetalSurfaceEXT"));
            if (!vkCreateMetalSurfaceEXT) {
                std::cerr << "Error: vkCreateMetalSurfaceEXT function not found. Is MoltenVK properly linked/loaded?" << std::endl;
                destroy();
                return false;
            }

            surfaceResult = vkCreateMetalSurfaceEXT(instance, &mtlSurfaceInfo, nullptr, &surface);
            break;
        }
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
        case VulkanOS::Linux: {
            VkXcbSurfaceCreateInfoKHR xcbSurfaceInfo = {}; // For Linux
            xcbSurfaceInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
            // Assuming windowHandle is a pointer to a xcb_connection_t* and screen_num/window_id
            // This part is complex and depends on the windowing system integration.
            // For now, a placeholder:
            std::cerr << "Vulkan Linux surface creation not fully implemented in this example." << std::endl;
            // Example usage (requires xcb_connection_t and xcb_window_t):
            // xcbSurfaceInfo.connection = static_cast<xcb_connection_t*>(windowHandle); // Placeholder
            // xcbSurfaceInfo.window = window_id; // Need window ID
            // result = vkCreateXcbSurfaceKHR(instance, &xcbSurfaceInfo, nullptr, &surface);
            break;
        }
#endif
        default:
            std::cerr << "Unsupported OS for Vulkan surface creation." << std::endl;
            destroy();
            return false;
    }

    if (surfaceResult != VK_SUCCESS) {
        std::cerr << "Failed to create Vulkan surface! VkResult: " << surfaceResult << std::endl;
        destroy();
        return false;
    }

    return true;
}

void VulkanContext::destroy() {
    if (instance != VK_NULL_HANDLE) {
        if (surface != VK_NULL_HANDLE) {
            vkDestroySurfaceKHR(instance, surface, nullptr);
            surface = VK_NULL_HANDLE;
        }
        vkDestroyInstance(instance, nullptr);
        instance = VK_NULL_HANDLE;
    }
}

} // namespace lastchaos::vulkan

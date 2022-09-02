#pragma once

#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>
#include <optional>

const std::vector<const char*> validation_layers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> device_extensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
    const bool enable_validation_layers = false;
#else
    const bool enable_validation_layers = true;
#endif

struct QueueFamilyIndices {
    static QueueFamilyIndices find_queue_families(const vk::PhysicalDevice&, const vk::SurfaceKHR& surface);
    std::optional<uint32_t> graphics;
    std::optional<uint32_t> present;
    bool satisfied_all() const;
};

struct SwapChainSupportDetails {
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> present_modes;
};

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(VkInstance instance,
                                                              const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                                              const VkAllocationCallbacks *pAllocator,
                                                              VkDebugUtilsMessengerEXT *pMessenger);
VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                                           VkAllocationCallbacks const *pAllocator);
void VkToolMakeDebugUtilsMessengerEXT(const vk::Instance& inst);
vk::Result check_validation_layers();
std::vector<const char*> get_required_extensions();
VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
    void* p_user_data);
vk::DebugUtilsMessengerCreateInfoEXT get_messenger_create_info();
bool is_device_suitable(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface);
bool check_device_extensions_support(const vk::PhysicalDevice& device);
SwapChainSupportDetails query_swapchain_support(const vk::PhysicalDevice& phy_device, const vk::SurfaceKHR& surface);
vk::SurfaceFormatKHR choose_surface_format(const std::vector<vk::SurfaceFormatKHR>& formats);
vk::PresentModeKHR choose_present_mode(const std::vector<vk::PresentModeKHR>& modes);
vk::Extent2D choose_extent(const vk::SurfaceCapabilitiesKHR& capabilities);

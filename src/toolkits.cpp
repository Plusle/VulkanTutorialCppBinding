#include <toolkits.hpp>
#include <GLFW/glfw3.h>
#include <iostream>

PFN_vkCreateDebugUtilsMessengerEXT pfnVkCreateDebugUtilsMessengerEXT;
PFN_vkDestroyDebugUtilsMessengerEXT pfnVkDestroyDebugUtilsMessengerEXT;

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(VkInstance instance,
                                                              const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                                              const VkAllocationCallbacks *pAllocator,
                                                              VkDebugUtilsMessengerEXT *pMessenger) {
    return pfnVkCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                                           VkAllocationCallbacks const *pAllocator) {
    return pfnVkDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
}

void VkToolMakeDebugUtilsMessengerEXT(const vk::Instance& inst) {
    pfnVkCreateDebugUtilsMessengerEXT 
        = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        inst.getProcAddr("vkCreateDebugUtilsMessengerEXT"));
    pfnVkDestroyDebugUtilsMessengerEXT 
        = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        inst.getProcAddr("vkDestroyDebugUtilsMessengerEXT"));
}

vk::Result check_validation_layers() {
    uint32_t layer_count = 0;
    auto [ result, available_layers ] = vk::enumerateInstanceLayerProperties();
    if (result != vk::Result::eSuccess)
        return result;

    for (const char* layer_name : validation_layers) {
        bool found = false;
        
        for (const auto& layer : available_layers) {
            if (strcmp(layer_name, layer.layerName) == 0) {
                found = true;
                break;
            }
        }

        if (!found) 
            return vk::Result::eErrorExtensionNotPresent;
    }

    return vk::Result::eSuccess;
}

std::vector<const char*> get_required_extensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enable_validation_layers) 
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return extensions;
}

VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
    void* p_user_data) {

    if (severity > VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)    
        std::cerr << "Validation layer: " << p_callback_data->pMessage << "\n";

    return false;
}

vk::DebugUtilsMessengerCreateInfoEXT get_messenger_create_info() {
    vk::DebugUtilsMessageSeverityFlagsEXT severity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | 
                                                   vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | 
                                                   vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);

    vk::DebugUtilsMessageTypeFlagsEXT type(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | 
                                           vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | 
                                           vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance);

    return vk::DebugUtilsMessengerCreateInfoEXT()
            .setMessageSeverity(severity)
            .setMessageType(type)
            .setPfnUserCallback(debug_callback)
            .setPUserData(nullptr);
}

bool is_device_suitable(const vk::PhysicalDevice& device) {
    return true;
}
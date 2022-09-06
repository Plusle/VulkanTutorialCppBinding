#include <toolkits.hpp>
#include <iostream>
#include <fstream>
#include <set>

PFN_vkCreateDebugUtilsMessengerEXT pfnVkCreateDebugUtilsMessengerEXT;
PFN_vkDestroyDebugUtilsMessengerEXT pfnVkDestroyDebugUtilsMessengerEXT;

QueueFamilyIndices QueueFamilyIndices::find_queue_families(const vk::PhysicalDevice& phy_device, const vk::SurfaceKHR& surface) {
    QueueFamilyIndices indices;

    std::vector<vk::QueueFamilyProperties> queue_families
        = phy_device.getQueueFamilyProperties();

    uint32_t idx = 0;
    for (const auto& queue_family : queue_families) {
        if (queue_family.queueFlags & vk::QueueFlagBits::eGraphics) {
            indices.graphics = idx;
        }
        
        vk::Bool32 support;
        auto result = phy_device.getSurfaceSupportKHR(idx, surface, &support);
        if (support) {
            indices.present = idx;
        }

        if (indices.satisfied_all())
            break;
            
        ++idx;
    }

    return indices;
}

bool QueueFamilyIndices::satisfied_all() const {
    return graphics.has_value() && present.has_value();
}

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

bool check_validation_layers() {
    uint32_t layer_count = 0;
    auto available_layers = vk::enumerateInstanceLayerProperties();

    for (const char* layer_name : validation_layers) {
        bool found = false;
        
        for (const auto& layer : available_layers) {
            if (strcmp(layer_name, layer.layerName) == 0) {
                found = true;
                break;
            }
        }

        if (!found) 
            return false;
    }

    return true;
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

bool is_device_suitable(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface) {
// bool is_device_suitable(const vk::PhysicalDevice& device) {
    QueueFamilyIndices indices = QueueFamilyIndices::find_queue_families(device, surface);
    bool extensions_support = check_device_extensions_support(device);
    bool swapchain_adequate = false;
    if (extensions_support) {
        SwapChainSupportDetails support = query_swapchain_support(device, surface);
        swapchain_adequate = !support.formats.empty() && !support.present_modes.empty();     
    }
    return indices.satisfied_all() && extensions_support && swapchain_adequate;
}

bool check_device_extensions_support(const vk::PhysicalDevice& device) {
    auto extensions = device.enumerateDeviceExtensionProperties();

    std::set<std::string> required_extensions(device_extensions.begin(), device_extensions.end());
    for (const auto& extension : extensions) {
        required_extensions.erase(extension.extensionName);
    }

    return required_extensions.empty();
}

SwapChainSupportDetails query_swapchain_support(const vk::PhysicalDevice& phy_device, const vk::SurfaceKHR& surface) {
    return SwapChainSupportDetails {
        .capabilities = phy_device.getSurfaceCapabilitiesKHR(surface),
        .formats = phy_device.getSurfaceFormatsKHR(surface),
        .present_modes = phy_device.getSurfacePresentModesKHR(surface)
    };
}


vk::SurfaceFormatKHR choose_surface_format(const std::vector<vk::SurfaceFormatKHR>& formats) {
    for (const auto& format : formats) {
        if (format.format == vk::Format::eR8G8B8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) { 
            return format;
        }
    }
    return formats[0];
}

vk::PresentModeKHR choose_present_mode(const std::vector<vk::PresentModeKHR>& modes) {
    for (const auto& mode : modes) {
        if (mode == vk::PresentModeKHR::eMailbox)
            return mode;
    }
    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D choose_extent(const vk::SurfaceCapabilitiesKHR& capabilities, GLFWwindow* window) {
    // auto max = std::numeric_limits<uint32_t>::max();
    if (capabilities.currentExtent.width 
        != (std::numeric_limits<uint32_t>::max)()) {
            // != 0) {
        return capabilities.currentExtent;
    } else {
        int32_t width, height;
        glfwGetFramebufferSize(window, &width, &height);
        vk::Extent2D extent {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };
        extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        return extent;
    }
}

std::vector<char> read_file(const char* filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("Can't open file " + std::string(filename));
    
    size_t file_size = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(file_size);
    file.seekg(0);
    file.read(buffer.data(), file_size);
    return buffer;
}

vk::ShaderModule create_shader_module(const vk::Device& device, const std::vector<char>& buffer) {
    vk::ShaderModuleCreateInfo sm_create_info {
        .codeSize = buffer.size(),
        .pCode = reinterpret_cast<const uint32_t*>(buffer.data())
    };

    return device.createShaderModule(sm_create_info);
}
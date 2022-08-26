#include <application.hpp>
#include <toolkits.hpp>

void Application::run() {
    init();
    mainloop();
    cleanup();
}

void Application::init() {
    init_glfw();
    init_vulkan();
}

void Application::init_glfw() {
    const uint32_t height = 600;
    const uint32_t width = 800;

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_window = glfwCreateWindow(width, height, "Vulkan Window", nullptr, nullptr);
}

void Application::mainloop() {
    while (!glfwWindowShouldClose(m_window)) {
        glfwPollEvents();
    }
}

void Application::cleanup() {
    if (enable_validation_layers) {
        m_inst.destroyDebugUtilsMessengerEXT(m_db_messenger);
    }
    m_inst.destroy();

    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void Application::init_vulkan() {
    create_instance();
    setup_debugger();
    select_physical_device();
}

void Application::setup_debugger() {
    if (!enable_validation_layers) return;
        VkToolMakeDebugUtilsMessengerEXT(m_inst);
        
    vk::DebugUtilsMessengerCreateInfoEXT messenger_info
        = get_messenger_create_info();

    auto messenger_result = m_inst.createDebugUtilsMessengerEXT(messenger_info);
    m_db_messenger = messenger_result.value;
}

void Application::create_instance() {
    if (enable_validation_layers && check_validation_layers() != vk::Result::eSuccess)
        throw std::runtime_error("Requested validation layer(s) available.");

    vk::ApplicationInfo appinfo = vk::ApplicationInfo()
                                        .setPApplicationName("Hello, world!")
                                        .setApplicationVersion(VK_MAKE_VERSION(1, 0, 0))
                                        .setPEngineName("No Engine")
                                        .setEngineVersion(VK_MAKE_VERSION(1, 0, 0))
                                        .setApiVersion(VK_API_VERSION_1_3);

    std::vector<const char*> extensions = get_required_extensions();

    vk::InstanceCreateInfo info = vk::InstanceCreateInfo()
                                        .setPApplicationInfo(&appinfo)
                                        .setEnabledExtensionCount(extensions.size())
                                        .setPpEnabledExtensionNames(extensions.data());

    vk::DebugUtilsMessengerCreateInfoEXT messenger_info;
    if (enable_validation_layers) {
        messenger_info = get_messenger_create_info();
        info.setEnabledLayerCount(static_cast<uint32_t>(validation_layers.size()))
            .setPpEnabledLayerNames(validation_layers.data())
            .setPNext(&messenger_info);
    } else {
        info.setEnabledLayerCount(0)
            .setPNext(nullptr);
    }
    
    auto [ result, inst ] = vk::createInstance(info);
    if (result != vk::Result::eSuccess)
        throw std::runtime_error("Failed to create Vulkan instance.");
    m_inst = std::move(inst);
}

void Application::select_physical_device() {
    auto [ result, available_devices ] = m_inst.enumeratePhysicalDevices();
    if (result != vk::Result::eSuccess)
        throw std::runtime_error("Can't find an available GPU");

    for (const auto& device : available_devices) {
        if (is_device_suitable(device)) {
            m_phy_device = device;
            break;
        }
    }

    if (!m_phy_device)
        throw std::runtime_error("Can't find a suitable GPU");
}


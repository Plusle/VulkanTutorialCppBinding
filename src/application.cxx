#include <application.hpp>
#include <toolkits.hpp>

#include <cstdint>
#include <limits>
#include <set>
#include <array>

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
    int w, h;
    glfwGetWindowSize(m_window, &w, &h);
    std::cout << "w:" << w << " h:" << h << "\n";
}

void Application::mainloop() {
    while (!glfwWindowShouldClose(m_window)) {
        glfwPollEvents();
    }
}

void Application::cleanup() {
    if (enable_validation_layers) 
        m_inst.destroyDebugUtilsMessengerEXT(m_db_messenger);

    for (const auto& image_view : m_image_views)
        m_device.destroyImageView(image_view);
    m_device.destroyPipeline(m_pipeline);
    m_device.destroyPipelineLayout(m_pipeline_layout);
    m_device.destroyRenderPass(m_render_pass);
    m_device.destroySwapchainKHR(m_swapchain);
    m_device.destroy();
    m_inst.destroySurfaceKHR(m_surface);
    m_inst.destroy();

    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void Application::init_vulkan() {
    create_instance();
    setup_debugger();
    create_surface();
    select_physical_device();
    create_device();
    create_swapchain();
    create_image_view();
    create_render_pass();
    create_pipeline();
}

void Application::setup_debugger() {
    if (!enable_validation_layers) return;
        VkToolMakeDebugUtilsMessengerEXT(m_inst);
        
    vk::DebugUtilsMessengerCreateInfoEXT messenger_info
        = get_messenger_create_info();

    m_db_messenger = m_inst.createDebugUtilsMessengerEXT(messenger_info);
}

void Application::create_instance() {
    if (enable_validation_layers && !check_validation_layers())
        throw std::runtime_error("Requested validation layer(s) available.");

    vk::ApplicationInfo appinfo {
        .pApplicationName = "Hello, world!",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_3
    };

    std::vector<const char*> extensions = get_required_extensions();
    vk::InstanceCreateInfo info {
        .pApplicationInfo = &appinfo,
        .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data()
    };
                                        
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
    
    m_inst = vk::createInstance(info);
}

void Application::select_physical_device() {
    auto available_devices = m_inst.enumeratePhysicalDevices();
    
    for (const auto& device : available_devices) {
        if (is_device_suitable(device, m_surface)) {
            m_phy_device = device;
            break;
        }
    }

    if (!m_phy_device)
        throw std::runtime_error("Can't find a suitable GPU");
}

void Application::create_device() {
    QueueFamilyIndices indices = QueueFamilyIndices::find_queue_families(m_phy_device, m_surface);

    std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;
    std::set<uint32_t> unique_queue_families = { indices.graphics.value(), indices.present.value() };

    float priority = 1.0f;
    for (auto queue_family : unique_queue_families) {
        vk::DeviceQueueCreateInfo queue_create_info {
            .queueFamilyIndex = queue_family,
            .queueCount = 1,
            .pQueuePriorities = &priority
        };

        queue_create_infos.emplace_back(std::move(queue_create_info));
    }

    vk::PhysicalDeviceFeatures phy_device_features{};
    vk::DeviceCreateInfo device_create_info {
        .queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size()),
        .pQueueCreateInfos = queue_create_infos.data(),
        .enabledExtensionCount = static_cast<uint32_t>(device_extensions.size()),
        .ppEnabledExtensionNames = device_extensions.data(),
        .pEnabledFeatures = &phy_device_features
    };
    
    if (enable_validation_layers) {
        device_create_info.setEnabledLayerCount(validation_layers.size())
                        .setPpEnabledLayerNames(validation_layers.data());
    } else {
        device_create_info.setEnabledLayerCount(0);
    }

    m_device = m_phy_device.createDevice(device_create_info);
    m_queue = m_device.getQueue(indices.graphics.value(), 0);
}

void Application::create_surface() {
    vk::Win32SurfaceCreateInfoKHR surface_create_info {
        .hinstance = nullptr,
        .hwnd = glfwGetWin32Window(m_window)
    };
    
    m_surface = m_inst.createWin32SurfaceKHR(surface_create_info);
}

void Application::create_swapchain() {
    SwapChainSupportDetails details = query_swapchain_support(m_phy_device, m_surface);

    vk::SurfaceFormatKHR format = choose_surface_format(details.formats);
    vk::PresentModeKHR present = choose_present_mode(details.present_modes);
    vk::Extent2D extent = choose_extent(details.capabilities, m_window);

    uint32_t image_count = details.capabilities.minImageCount + 1;

    if (details.capabilities.maxImageCount > 0 && image_count > details.capabilities.maxImageCount) 
        image_count = details.capabilities.maxImageCount;
    
    vk::SwapchainCreateInfoKHR swapchain_create_info {
        .surface = m_surface,
        .minImageCount = image_count,
        .imageFormat = format.format,
        .imageColorSpace = format.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = vk::ImageUsageFlagBits::eColorAttachment
    };

    QueueFamilyIndices indices = QueueFamilyIndices::find_queue_families(m_phy_device, m_surface);
    uint32_t queue_family_indices[] = {
        indices.graphics.value(),
        indices.present.value()
    };

    if (indices.graphics != indices.present) {
        swapchain_create_info
            .setImageSharingMode(vk::SharingMode::eConcurrent)
            .setQueueFamilyIndexCount(2)
            .setPQueueFamilyIndices(queue_family_indices);
    } else {
        swapchain_create_info
            .setImageSharingMode(vk::SharingMode::eExclusive)
            .setQueueFamilyIndexCount(0)
            .setPQueueFamilyIndices(nullptr);
    }

    swapchain_create_info
        .setPreTransform(details.capabilities.currentTransform)
        .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
        .setPresentMode(present)
        .setClipped(static_cast<vk::Bool32>(true));

    m_swapchain = m_device.createSwapchainKHR(swapchain_create_info);
    m_extent = extent;
    m_format = format.format;

    m_images = m_device.getSwapchainImagesKHR(m_swapchain);
}

void Application::create_image_view() {
    m_image_views.resize(m_images.size());
    for (auto i = 0; i < m_image_views.size(); ++i) {
        vk::ImageViewCreateInfo im_create_info {
            .image = m_images[i],
            .viewType = vk::ImageViewType::e2D,
            .format = m_format,
            .components = vk::ComponentMapping { 
                vk::ComponentSwizzle::eR, 
                vk::ComponentSwizzle::eG, 
                vk::ComponentSwizzle::eB, 
                vk::ComponentSwizzle::eA 
            },
            .subresourceRange = vk::ImageSubresourceRange {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }           
        };
        m_image_views[i] = m_device.createImageView(im_create_info);
    }
}

void Application::create_render_pass() {
    vk::AttachmentDescription color_attachment {
        .format = m_format,
        .samples = vk::SampleCountFlagBits::e1,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
        .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
        .initialLayout = vk::ImageLayout::eUndefined,
        .finalLayout = vk::ImageLayout::ePresentSrcKHR
    
    };

    vk::AttachmentReference color_attach_ref {
        .attachment = 0,
        .layout = vk::ImageLayout::eAttachmentOptimal
    };

    vk::SubpassDescription subpass {
        .pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attach_ref
    };

    vk::RenderPassCreateInfo render_pass_create_info {
        .attachmentCount = 1,
        .pAttachments = &color_attachment,
        .subpassCount = 1,
        .pSubpasses = &subpass
    };

    m_render_pass = m_device.createRenderPass(render_pass_create_info);
}

void Application::create_pipeline() {
    std::vector<char> vert_shader = read_file("E:/code-cpp/TestVulkan/shader/binary/vert.spv");
    std::vector<char> frag_shader = read_file("E:/code-cpp/TestVulkan/shader/binary/frag.spv");
    vk::ShaderModule vs = create_shader_module(m_device, vert_shader);
    vk::ShaderModule fs = create_shader_module(m_device, frag_shader);

    vk::PipelineShaderStageCreateInfo stages[] = {
        vk::PipelineShaderStageCreateInfo {
            .stage = vk::ShaderStageFlagBits::eVertex,
            .module = vs,
            .pName = "main"
        },
        vk::PipelineShaderStageCreateInfo {
            .stage = vk::ShaderStageFlagBits::eFragment,
            .module = fs,
            .pName = "main"
        }
    };

    vk::PipelineVertexInputStateCreateInfo vertex_input_create_info {
        .vertexBindingDescriptionCount = 0,
        .vertexAttributeDescriptionCount = 0
    };

    vk::PipelineInputAssemblyStateCreateInfo input_assembly_create_info {
        .topology = vk::PrimitiveTopology::eTriangleList,
        .primitiveRestartEnable = static_cast<vk::Bool32>(false)
    };

    vk::PipelineViewportStateCreateInfo viewport_create_info {
        .viewportCount = 1,
        .scissorCount = 1
    };

    vk::PipelineRasterizationStateCreateInfo rasterization_create_info {
        .depthClampEnable = static_cast<vk::Bool32>(false),
        .rasterizerDiscardEnable = static_cast<vk::Bool32>(false),
        .polygonMode = vk::PolygonMode::eFill,
        .cullMode = vk::CullModeFlagBits::eBack,
        .frontFace = vk::FrontFace::eClockwise,
        .depthBiasEnable = static_cast<vk::Bool32>(false),
        .lineWidth = 1.0f
    };

    vk::PipelineMultisampleStateCreateInfo multisampling_create_info {
        .rasterizationSamples = vk::SampleCountFlagBits::e1,
        .sampleShadingEnable = static_cast<vk::Bool32>(false)
    };

    vk::PipelineColorBlendAttachmentState blend_attach_create_info {
        .blendEnable = static_cast<vk::Bool32>(false),
        .colorWriteMask = vk::ColorComponentFlagBits::eR
                        | vk::ColorComponentFlagBits::eG
                        | vk::ColorComponentFlagBits::eB
                        | vk::ColorComponentFlagBits::eA
    };

    vk::PipelineColorBlendStateCreateInfo blend_state_create_info {
        .logicOpEnable = static_cast<vk::Bool32>(false),
        .logicOp = vk::LogicOp::eCopy,
        .attachmentCount = 1,
        .pAttachments = &blend_attach_create_info,
        .blendConstants = std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 0.0f }
    };

    std::vector<vk::DynamicState> dynamic_states {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };

    vk::PipelineDynamicStateCreateInfo state_create_info {
        .dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()),
        .pDynamicStates = dynamic_states.data()
    };

    vk::PipelineLayoutCreateInfo pipeline_layout_create_info {
        .setLayoutCount = 0,
        .pushConstantRangeCount = 0
    };

    m_pipeline_layout = m_device.createPipelineLayout(pipeline_layout_create_info);

    vk::GraphicsPipelineCreateInfo pipeline_create_info {
        .stageCount = 2,
        .pStages = stages,
        .pVertexInputState = &vertex_input_create_info,
        .pInputAssemblyState = &input_assembly_create_info,
        .pViewportState = &viewport_create_info,
        .pRasterizationState = &rasterization_create_info,
        .pMultisampleState = &multisampling_create_info,
        .pDepthStencilState = nullptr,
        .pColorBlendState = &blend_state_create_info,
        .pDynamicState = &state_create_info,
        .layout = m_pipeline_layout,
        .renderPass = m_render_pass,
        .subpass = 0,
        .basePipelineHandle = nullptr,
        .basePipelineIndex = -1
    };
    // auto pipelines = m_device.createGraphicsPipelines(VK_NULL_HANDLE, std::array<vk::GraphicsPipelineCreateInfo, 1>{pipeline_create_info});
    // m_pipeline = *(pipelines.cbegin());
    m_pipeline = m_device.createGraphicsPipeline(VK_NULL_HANDLE, pipeline_create_info).value;
    m_device.destroyShaderModule(vs);
    m_device.destroyShaderModule(fs);
}
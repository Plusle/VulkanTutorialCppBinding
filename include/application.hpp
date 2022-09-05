#pragma once

#include <vk.hpp>
#include <iostream>
#include <optional>


class Application {
public:
    void run();
    void init();
    void init_vulkan();
    void init_glfw();
    void mainloop();
    void cleanup();

private:
    void create_instance();
    void setup_debugger();
    void select_physical_device();
    void create_device();
    void create_surface();
    void create_swapchain();
    void create_image_view();
    void create_pipeline();


    // vk::SurfaceFormatKHR choose_surface_format(const std::vector<vk::SurfaceFormatKHR>& formats);
    // vk::PresentModeKHR choose_present_mode(const std::vector<vk::PresentModeKHR>& modes);
    // vk::Extent2D choose_extent(const vk::SurfaceCapabilitiesKHR& capabilities);


    GLFWwindow* m_window;
    vk::Instance m_inst;
    vk::DebugUtilsMessengerEXT m_db_messenger;
    vk::PhysicalDevice m_phy_device = VK_NULL_HANDLE;
    vk::Device m_device;
    vk::Queue m_queue;
    vk::SurfaceKHR m_surface;
    vk::SwapchainKHR m_swapchain;
    vk::Format m_format;
    vk::Extent2D m_extent;
    std::vector<vk::Image> m_images;
    std::vector<vk::ImageView> m_image_views;
    vk::Pipeline m_pipeline;
};
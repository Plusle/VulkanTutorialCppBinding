#pragma once

#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>
#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_TYPESAFE_CONVERSION
#include <vulkan/vulkan.hpp>
#include <iostream>

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



    GLFWwindow* m_window;
    vk::Instance m_inst;
    vk::DebugUtilsMessengerEXT m_db_messenger;
    vk::PhysicalDevice m_phy_device = VK_NULL_HANDLE;
    vk::Device m_device;
};
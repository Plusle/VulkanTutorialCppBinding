#pragma once

#define VK_USE_PLATFORM_WIN32_KHR

#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#define VULKAN_HPP_NO_EXCEPTIONS
#define VULKAN_HPP_TYPESAFE_CONVERSION
#define VULKAN_HPP_NO_CONSTRUCTORS
#include <vulkan/vulkan.hpp>
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



    GLFWwindow* m_window;
    vk::Instance m_inst;
    vk::DebugUtilsMessengerEXT m_db_messenger;
    vk::PhysicalDevice m_phy_device = VK_NULL_HANDLE;
    vk::Device m_device;
    vk::Queue m_queue;
    vk::SurfaceKHR m_surface;
};
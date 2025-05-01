#pragma once

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

class Gfx {
    public:
        void Run();
    private:
        void CreateWindow();
        void VulkanInit();
        void Loop();
        void Cleanup();

    private:
        void CreateInstance();
        void CreatePhysicalDevice();
        void CreateDevice();

    private:
        GLFWwindow* window = nullptr;
        VkInstance instance;
        VkPhysicalDevice physical_device;
        VkDevice device;
        
};

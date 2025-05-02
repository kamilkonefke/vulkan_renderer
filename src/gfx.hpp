#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <optional>
#include <vector>

class Gfx {
    private:
        struct QueueFamilyIndices {
            std::optional<uint32_t> graphicsFamily;
            std::optional<uint32_t> presentFamily;

            bool isComplete() {
                return graphicsFamily.has_value() && presentFamily.has_value();
            }
        }; 

        // Extensions
        const std::vector<const char*> device_extensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        struct SwapChainSupportDetails {
            VkSurfaceCapabilitiesKHR capabilities;
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> presentModes;
        };

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
        bool IsDeviceSuitable(VkPhysicalDevice device);
        void CreateLogicalDevice();
        void CreateSurface();
        QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physical_device);
        void CreateSwapChain();
        SwapChainSupportDetails QuerySwapchainSupport(VkPhysicalDevice device);
        VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats);
        VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& available_present_modes);
        VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
        void CreateImageViews();


    private:
        GLFWwindow* window = nullptr;
        VkInstance instance;
        VkPhysicalDevice physical_device;
        VkDevice device;
        VkQueue graphics_queue;
        VkQueue present_queue;
        VkSurfaceKHR surface;
        VkSwapchainKHR swapchain;
        std::vector<VkImage> swapchain_images;
        std::vector<VkImageView> swapchain_image_view;
        VkFormat swapchain_image_format;
        VkExtent2D swapchain_extent;
};

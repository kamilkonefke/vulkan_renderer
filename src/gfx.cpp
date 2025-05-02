#include "gfx.hpp"

#include <stdexcept>
#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>

void Gfx::Run() {
    CreateWindow();
    VulkanInit();
    Loop();
    Cleanup();
}

void Gfx::Loop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
}

void Gfx::Cleanup() {
    vkDestroySwapchainKHR(device, swapchain, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyDevice(device, nullptr);
    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);
    glfwTerminate();
}

void Gfx::CreateWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(512, 512, "VK", nullptr, nullptr);
}

void Gfx::VulkanInit() {
    CreateInstance();
    CreateSurface();
    CreatePhysicalDevice();
    CreateLogicalDevice();
    CreateSwapChain();
}

void Gfx::CreateInstance() {
    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0 ,0);
    app_info.pApplicationName = "Vulkan renderer";
    app_info.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    app_info.pEngineName = "No Engine";
    app_info.apiVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);

    // Get extensions
    uint32_t extensions_count = 0;
    const char** extensions_names = glfwGetRequiredInstanceExtensions(&extensions_count);

    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;
    create_info.enabledExtensionCount = extensions_count;
    create_info.ppEnabledExtensionNames = extensions_names;

    if(vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create instance");
    }
}

void Gfx::CreatePhysicalDevice() {

    uint32_t physical_device_count = 0; 
    vkEnumeratePhysicalDevices(instance, &physical_device_count, nullptr);

    std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
    vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices.data());

    // Assign first discrete gpu
    for(VkPhysicalDevice device : physical_devices) {

        VkPhysicalDeviceProperties device_properties;
        vkGetPhysicalDeviceProperties(device, &device_properties);

        if(device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            if(IsDeviceSuitable(device)) {
                physical_device = device;
                std::cout << "Selected: " << device_properties.deviceName << '\t' << "ID: " << device_properties.deviceID << '\n';
                break;
            }
        }
    }

    if (physical_device == VK_NULL_HANDLE) {
        throw std::runtime_error("Failed to find `good enough` gpu");
    }
} 

bool Gfx::IsDeviceSuitable(VkPhysicalDevice device) {
    QueueFamilyIndices indicies = FindQueueFamilies(device);

    // check for swapchain
    bool have_swap_chain = false;
    SwapChainSupportDetails swap_chain_support = QuerySwapchainSupport(device);
    have_swap_chain = !swap_chain_support.formats.empty() && !swap_chain_support.presentModes.empty();

    return indicies.isComplete() && have_swap_chain;
}

Gfx::QueueFamilyIndices Gfx::FindQueueFamilies(VkPhysicalDevice physical_device) {
    QueueFamilyIndices indicies = {};
    // Get graphics and present quque family
    uint32_t queue_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_properties(queue_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_count, queue_properties.data());

    int i = 0;
    for (VkQueueFamilyProperties queue : queue_properties) {
        if (queue.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indicies.graphicsFamily = i;
        }

        VkBool32 is_present_supported = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &is_present_supported);

        if (is_present_supported) {
            indicies.presentFamily = i;
        }

        if (indicies.isComplete()) {
            break;
        }
        i++;
    }

    return indicies;
}


void Gfx::CreateLogicalDevice() {
    QueueFamilyIndices indicies = FindQueueFamilies(physical_device);

    float queue_piority = 1.0f;
    VkDeviceQueueCreateInfo queue_create_info = {};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    queue_create_info.pQueuePriorities = &queue_piority;
    queue_create_info.queueCount = 1;
    queue_create_info.queueFamilyIndex = 0;

    VkPhysicalDeviceFeatures device_features = {};

    VkDeviceCreateInfo dev_create_info = {};
    dev_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dev_create_info.pQueueCreateInfos = &queue_create_info;
    dev_create_info.pEnabledFeatures = &device_features;
    dev_create_info.queueCreateInfoCount = 1;

    // enable swapchain
    dev_create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
    dev_create_info.ppEnabledExtensionNames = device_extensions.data();

    if (vkCreateDevice(physical_device, &dev_create_info, nullptr, &device) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create logical device");
    }

    vkGetDeviceQueue(device, indicies.graphicsFamily.value(), 0, &graphics_queue);
    vkGetDeviceQueue(device, indicies.presentFamily.value(), 0, &present_queue);

    // Swaphcain capabilities
    
}

Gfx::SwapChainSupportDetails Gfx::QuerySwapchainSupport(VkPhysicalDevice device) {
    SwapChainSupportDetails details = {};

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);

    // If formats are avaliable then add it to struct
    if (format_count != 0) {
        details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, details.formats.data());
    }

    // Same for presence modes
    uint32_t present_mode_count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, nullptr);

    if (present_mode_count != 0) {
        details.presentModes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, details.presentModes.data());
    }

    return details;
}

void Gfx::CreateSwapChain() {
    SwapChainSupportDetails swap_chain_support = QuerySwapchainSupport(physical_device);

    VkSurfaceFormatKHR surface_format = ChooseSwapSurfaceFormat(swap_chain_support.formats);
    VkPresentModeKHR present_mode = ChooseSwapPresentMode(swap_chain_support.presentModes);
    VkExtent2D extent = ChooseSwapExtent(swap_chain_support.capabilities);

    uint32_t imageCount = swap_chain_support.capabilities.minImageCount + 1;
    if (swap_chain_support.capabilities.maxImageCount > 0 && imageCount > swap_chain_support.capabilities.maxImageCount) {
        imageCount = swap_chain_support.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = surface;
    create_info.minImageCount = imageCount;
    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = FindQueueFamilies(physical_device);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.queueFamilyIndexCount = 0; // Optional
        create_info.pQueueFamilyIndices = nullptr; // Optional
    }

    create_info.preTransform = swap_chain_support.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = present_mode;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device, &create_info, nullptr, &swapchain) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create swapchain");
    }

    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
    swapchain_images.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchain_images.data());

    swapchain_image_format = surface_format.format;
    swapchain_extent = extent;
}

VkSurfaceFormatKHR Gfx::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats) {
    for (const auto& available_format : available_formats) {
        if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return available_format;
        }
    }

    return available_formats[0];
}

VkPresentModeKHR Gfx::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& available_present_modes) {
    // Check if MAILBOX is available if not then set FIFO mode
    for (const auto& available_present_mode : available_present_modes) {
        if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return available_present_mode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Gfx::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actual_extent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actual_extent.width = std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actual_extent.height = std::clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actual_extent;
    }
}

void Gfx::CreateSurface() {
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create surfae.");
    }
}

// 

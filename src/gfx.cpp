#include "gfx.hpp"

#include <stdexcept>
#include <iostream>
#include <vector>

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
    CreatePhysicalDevice();
    CreateDevice();
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
            physical_device = device;
            std::cout << "Selected: " << device_properties.deviceName << '\t' << "ID: " << device_properties.deviceID << '\n';
        }
    }

    uint32_t queue_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_count, nullptr);

    std::vector<VkQueueFamilyProperties> properties_test(queue_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_count, properties_test.data());

    for (VkQueueFamilyProperties queue : properties_test) {
        if (queue.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            std::cout << queue.queueFlags << '\n';
        }
    }
} 

void Gfx::CreateDevice() {
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

    if (vkCreateDevice(physical_device, &dev_create_info, nullptr, &device) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create logical device");
    }
}

// 

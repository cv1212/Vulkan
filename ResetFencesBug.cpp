#include <vulkan/vulkan.h>
#include <cassert>
#include <vector>

#define BAIL_ON_CONDITION(C) if((C)) { return 1; }
#define BAIL_IF_NOT_SUCCESS(V) BAIL_ON_CONDITION((V) != VK_SUCCESS)

int main(int argc, char *argv[])
{
    // Initialize the VkApplicationInfo structure
    VkApplicationInfo applicationInfo0 = {};
    applicationInfo0.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo0.pNext = NULL;
    applicationInfo0.pApplicationName = "ResetFencesBug";
    applicationInfo0.applicationVersion = 1;
    applicationInfo0.pEngineName = "ResetFencesBugEngine";
    applicationInfo0.engineVersion = 1;
    applicationInfo0.apiVersion = VK_MAKE_VERSION(1, 0, 3);

    // Initialize the VkInstanceCreateInfo structure
    VkInstanceCreateInfo instanceCreateInfo0 = {};
    instanceCreateInfo0.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo0.pNext = NULL;
    instanceCreateInfo0.flags = 0;
    instanceCreateInfo0.pApplicationInfo = &applicationInfo0;
    instanceCreateInfo0.enabledExtensionCount = 0;
    instanceCreateInfo0.ppEnabledExtensionNames = NULL;
    instanceCreateInfo0.enabledLayerCount = 0;
    instanceCreateInfo0.ppEnabledLayerNames = NULL;

    // Create instance
    VkInstance instance0;
    VkResult result0 = vkCreateInstance(&instanceCreateInfo0, NULL, &instance0);
    BAIL_IF_NOT_SUCCESS(result0);

    // Enumerate physical devices
    std::vector<VkPhysicalDevice> gpus0;
    uint32_t gpuCount0 = 0;
    VkResult result1;

    do
    {
        result1 = vkEnumeratePhysicalDevices(instance0, &gpuCount0, NULL);
        gpus0.resize(gpuCount0);
        result1 = vkEnumeratePhysicalDevices(instance0, &gpuCount0, gpus0.data());
    }
    while(result1 == VK_INCOMPLETE);

    BAIL_IF_NOT_SUCCESS(result1);
    BAIL_ON_CONDITION(gpuCount0 == 0);

    // Enumerate queue family properties for each gpu available
    std::vector<std::vector<VkQueueFamilyProperties>> queueFamilyProperties0;

    for (VkPhysicalDevice physicalDevice : gpus0)
    {
        std::vector<VkQueueFamilyProperties> properties;
        uint32_t queueCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice,
                &queueCount, NULL);

        properties.resize(queueCount);

        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice,
                &queueCount, properties.data());

        queueFamilyProperties0.push_back(properties);
    }

    // Create logical devices that support graphics on the first available gpu
    std::vector<VkDevice> logicalDevices0;

    for (uint32_t j = 0; j < queueFamilyProperties0[0].size(); ++j)
    {
        if (queueFamilyProperties0[0][j].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
			// Request only 1 queue
            std::vector<float> queuePriorities;
            queuePriorities.push_back(1.0f);

            VkDeviceQueueCreateInfo queueInfo = {};
            queueInfo.queueFamilyIndex = j;
            queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueInfo.pNext = NULL;
            queueInfo.queueCount = queuePriorities.size();
            queueInfo.pQueuePriorities = queuePriorities.data();

            VkDeviceCreateInfo deviceInfo = {};
            deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            deviceInfo.flags = 0;
            deviceInfo.pNext = NULL;
            deviceInfo.queueCreateInfoCount = 1;
            deviceInfo.pQueueCreateInfos = &queueInfo;
            deviceInfo.enabledExtensionCount = 0;
            deviceInfo.ppEnabledExtensionNames = NULL;
            deviceInfo.enabledLayerCount = 0;
            deviceInfo.ppEnabledLayerNames = NULL;
            deviceInfo.pEnabledFeatures = NULL;

            VkDevice device;
            VkResult res = vkCreateDevice(gpus0[0], &deviceInfo,
                    NULL, &device);

            logicalDevices0.push_back(device);

            BAIL_IF_NOT_SUCCESS(res);
        }
    }


    // Make sure we have atleast 1 device which supports graphics
    BAIL_ON_CONDITION(logicalDevices0.empty());

    // Create fence
    VkFenceCreateInfo fenceCreateInfo0 = {};
    fenceCreateInfo0.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo0.pNext = NULL;
    fenceCreateInfo0.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkFence fence0;
    VkResult result2 = vkCreateFence(logicalDevices0[0], &fenceCreateInfo0,
            NULL, &fence0);

    BAIL_IF_NOT_SUCCESS(result2);

    // Check that fence was created in signled state
    VkResult result3 = vkGetFenceStatus(logicalDevices0[0], fence0);
    BAIL_IF_NOT_SUCCESS(result3);

    // Reset fence to the unsignaled state
    VkResult result4 = vkResetFences(logicalDevices0[0], 1, &fence0);
    BAIL_IF_NOT_SUCCESS(result4);

    // Check that fence was reset to the unsignled state
    VkResult result5 = vkGetFenceStatus(logicalDevices0[0], fence0);
    assert(result5 == VK_NOT_READY);

    vkDestroyFence(logicalDevices0[0], fence0, NULL);

    for (VkDevice device : logicalDevices0)
    {
        vkDestroyDevice(device, NULL);
    }

    vkDestroyInstance(instance0, NULL);

    return 0;
}

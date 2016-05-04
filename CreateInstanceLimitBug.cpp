#include <vulkan/vulkan.h>
#include <cassert>
#include <random>

int main(int argc, char *argv[])
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0, 1);

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
    assert(result0 == VK_SUCCESS);

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

    assert(gpuCount0 != 0);
    assert(result1 == VK_SUCCESS);

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

    // Create logical devices that support graphics
    std::vector<VkDevice> logicalDevices0;

    // Loop 9 times to hit limit
    for (uint32_t k; k < 9; ++k)
    {
        for (uint32_t i = 0; i < gpus0.size(); ++i)
        {
            for (uint32_t j = 0; j < queueFamilyProperties0[i].size(); ++j)
            {
                if (queueFamilyProperties0[i][j].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                {
                    // Assign random priorities
                    float sum = 0.0f;
                    std::vector<float> queuePriorities;
                    for (unsigned int k = 0;
                            k < queueFamilyProperties0[i][j].queueCount; ++k)
                    {
                        float priority = dis(gen);
                        queuePriorities.push_back(priority);
                        sum += priority;
                    }

                    // Normalise priorities
                    for (unsigned int k = 0; k < queuePriorities.size(); ++k)
                    {
                        queuePriorities[k] /= sum;
                    }

                    VkDeviceQueueCreateInfo queueInfo = {};
                    queueInfo.queueFamilyIndex = j;
                    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                    queueInfo.pNext = NULL;
                    queueInfo.queueCount = queueFamilyProperties0[i][j].queueCount;
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
                    VkResult res = vkCreateDevice(gpus0[i], &deviceInfo,
                            NULL, &device);

                    logicalDevices0.push_back(device);

                    assert(res == VK_SUCCESS);
                }
            }
        }
    }


    for (VkDevice device : logicalDevices0)
    {
        vkDestroyDevice(device, NULL);
    }

    vkDestroyInstance(instance0, NULL);

    return 0;
}

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include<stdexcept>
#include<cstdlib>
#include<cstring>
#include<vector>
#include<optional>

class HelloTriangeApplication{

    public:
        void run(){
            initWindow();
            initVulkan();
            mainLoop();
            cleanup();
        }

    private:
        void initVulkan(){

            createInstance();
            createSurface();
            pickPhysicalDevice();
            createLogicalDevice();

        }

        void createInstance(){

            if(enableValidationLayers&&!checkValidationLayerSupport()){
                throw std::runtime_error("validation layer requested, but not available!");
            }

            //Application info (not mendatory)
            VkApplicationInfo appInfo{};
            appInfo.sType=VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.pApplicationName="Hello Vulkaaaan";
            appInfo.applicationVersion=VK_MAKE_VERSION(1,0,0);
            appInfo.pEngineName= "No Engine??";
            appInfo.engineVersion= VK_MAKE_VERSION(1,0,0);
            appInfo.apiVersion=VK_API_VERSION_1_0;
            
            //Create info which describes extensions and validation layers
            VkInstanceCreateInfo createInfo{};
            createInfo.sType=VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            createInfo.pApplicationInfo=&appInfo;

            uint32_t glfwExtensionCount=0;
            const char** glfwExtensions;

            //required vulkan extentions for glfw implementation
            glfwExtensions=glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
            createInfo.enabledExtensionCount=glfwExtensionCount;
            createInfo.ppEnabledExtensionNames=glfwExtensions;
            
            std::vector<const char*> requiredExtensions;

            for(uint32_t i = 0; i < glfwExtensionCount; i++) {
                requiredExtensions.emplace_back(glfwExtensions[i]);
            }

            requiredExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

            createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

            createInfo.enabledExtensionCount = (uint32_t) requiredExtensions.size();
            createInfo.ppEnabledExtensionNames = requiredExtensions.data();

            //will be used in global validation layer
            if(enableValidationLayers){

                createInfo.enabledLayerCount=static_cast<uint32_t>(validationLayers.size());
                createInfo.ppEnabledLayerNames=validationLayers.data();
            }
            else{
                createInfo.enabledLayerCount=0;
            }

            if(vkCreateInstance(&createInfo,nullptr,&instance)!=VK_SUCCESS){
                throw std::runtime_error("failed to create instance!");
            }
        }

        void createSurface(){

            if(glfwCreateWindowSurface(instance,window,nullptr,&surface)!=VK_SUCCESS){

                throw std::runtime_error("failed to create window surface!");
            }

        }

        void pickPhysicalDevice(){

            uint32_t deviceCount;
            vkEnumeratePhysicalDevices(instance,&deviceCount,nullptr);

            if(deviceCount==0){
                throw std::runtime_error("No GPU found with Vulkan support!");
            }

            std::vector<VkPhysicalDevice> devices(deviceCount);
            vkEnumeratePhysicalDevices(instance,&deviceCount,devices.data());

            for(const auto& device:devices){

                if(isDeviceSuitable(device)){
                    physicalDevice=device;  
                    break;
                }
            }
            if(physicalDevice==VK_NULL_HANDLE){

                throw std::runtime_error("failed to find suitable GPU!");
            }        
        }

        struct QueueFamilyIndices{
            std::optional<uint32_t> graphicsFamily;

            bool isComplete(){

                return graphicsFamily.has_value();
            }
        };

        bool isDeviceSuitable(VkPhysicalDevice device){

            QueueFamilyIndices indices= findQueueFamily(device);

            return indices.isComplete();

        }

        QueueFamilyIndices findQueueFamily(VkPhysicalDevice device){

            QueueFamilyIndices indices;
            uint32_t queFamilyCount=0;
            vkGetPhysicalDeviceQueueFamilyProperties(device,&queFamilyCount,nullptr);

            std::vector<VkQueueFamilyProperties> queueFamilies(queFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(device,&queFamilyCount,queueFamilies.data());

            int i=0;

            for(const auto& queueFamily: queueFamilies){

                if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT){

                    indices.graphicsFamily=i;

                }
                if(indices.isComplete()){
                    break;
                }
                ++i;
            }
            return indices;
        }

        void createLogicalDevice(){

            QueueFamilyIndices indices= findQueueFamily(physicalDevice);

            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType=VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex= indices.graphicsFamily.value();
            queueCreateInfo.queueCount=1;
            float queuePriority=1.0;
            queueCreateInfo.pQueuePriorities=&queuePriority;

            VkPhysicalDeviceFeatures deviceFeatures{}; //for now left blank due to no necessary device feeature needed.

            VkDeviceCreateInfo createInfo{};
            createInfo.sType=VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            createInfo.enabledExtensionCount=0;
            createInfo.pQueueCreateInfos=&queueCreateInfo;
            createInfo.queueCreateInfoCount=1;
            createInfo.pEnabledFeatures=&deviceFeatures;

            if(enableValidationLayers){
                createInfo.enabledLayerCount=static_cast<uint32_t>(validationLayers.size());
                createInfo.ppEnabledLayerNames=validationLayers.data();
            }
            else{

                createInfo.enabledLayerCount=0;
            }

            if(vkCreateDevice(physicalDevice,&createInfo,nullptr,&device)!=VK_SUCCESS){
                throw std::runtime_error("failed to create logical device!");
            }

            vkGetDeviceQueue(device,indices.graphicsFamily.value(),0,&graphicsQueue);
        }
        

        void initWindow(){

            glfwInit();

            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_RESIZABLE,GLFW_FALSE);

            window= glfwCreateWindow(WIDTH,HEIGHT,"Vulkan",nullptr,nullptr);

        }
        void mainLoop(){
            while(!glfwWindowShouldClose(window)){
                glfwPollEvents();
            }
        }

        void cleanup(){
            vkDestroyDevice(device,nullptr);
            vkDestroySurfaceKHR(instance,surface,nullptr);
            vkDestroyInstance(instance,nullptr);

            glfwDestroyWindow(window);
            glfwTerminate();
        }

        bool checkValidationLayerSupport(){

            uint32_t layerCount;
            vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

            std::vector<VkLayerProperties> availableLayers(layerCount);
            vkEnumerateInstanceLayerProperties(&layerCount,availableLayers.data());

            for(const char* layerName: validationLayers){
                bool layerFound=false;

                for(const auto& layerProperties: availableLayers){
                    if(strcmp(layerName, layerProperties.layerName)){
                        layerFound=true;
                        break;
                    }
                }
                if(!layerFound){
                    return false;
                }
            }
            return true;
        }

        //WINDOW
        GLFWwindow* window;
        const uint32_t WIDTH=800;
        const uint32_t HEIGHT=600;

        //VULKAN
        VkInstance instance;
        VkSurfaceKHR surface;
        VkPhysicalDevice physicalDevice= VK_NULL_HANDLE;
        VkDevice device;
        VkQueue graphicsQueue;

        const std::vector<const char*> validationLayers = {
            "VK_LAYER_KHRONOS_validation"
        };

        #ifdef NDEBUG
            const bool enableValidationsLayers=false;
        #else
            const bool enableValidationLayers=true;
        #endif

       

};


int main() {
    HelloTriangeApplication app;

    try{
        app.run();
    }catch(const std::exception &e){
        std::cerr<<e.what()<<std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include<stdexcept>
#include<cstdlib>
#include<cstring>
#include<vector>
#include<optional>
#include<set>
#include<limits>

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
            createSwapChain();
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
            std::optional<uint32_t> presentFamily;

            bool isComplete(){

                return graphicsFamily.has_value()&& presentFamily.has_value();
            }
        };

        bool isDeviceSuitable(VkPhysicalDevice device){

            QueueFamilyIndices indices= findQueueFamily(device);
            bool extensionSupport= checkExtensionSupport(device);
            bool swapChainAdequet=false;

            if(extensionSupport){
                SwapChainSupportDetails swapChainSupport=querySwapChainSupport(device);
                swapChainAdequet= !swapChainSupport.formats.empty()&&!swapChainSupport.presentModes.empty();
            }

            return indices.isComplete()&&extensionSupport&&swapChainAdequet;

        }

        bool checkExtensionSupport(VkPhysicalDevice device){
            uint32_t extensionCount=0;
            vkEnumerateDeviceExtensionProperties(device,nullptr,&extensionCount,nullptr);

            std::vector<VkExtensionProperties> availableExtensions(extensionCount);
            vkEnumerateDeviceExtensionProperties(device,nullptr,&extensionCount,availableExtensions.data());

            std::set<std::string> requiredExtensions(deviceExtensions.begin(),deviceExtensions.end());
            for(const auto& extension: availableExtensions ){
                requiredExtensions.erase(extension.extensionName);
            }

            return requiredExtensions.empty();

        }

        QueueFamilyIndices findQueueFamily(VkPhysicalDevice device){

            QueueFamilyIndices indices;
            uint32_t queFamilyCount=0;
            vkGetPhysicalDeviceQueueFamilyProperties(device,&queFamilyCount,nullptr);

            std::vector<VkQueueFamilyProperties> queueFamilies(queFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(device,&queFamilyCount,queueFamilies.data());

            int i=0;


            for(const auto& queueFamily: queueFamilies){
                
                VkBool32 presentSupport=false;
                vkGetPhysicalDeviceSurfaceSupportKHR(device,i,surface,&presentSupport);

                if(presentSupport){
                    indices.presentFamily=i;
                }

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

            std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
            std::set<uint32_t> uniqueQueueFamilies={indices.graphicsFamily.value(),indices.presentFamily.value()};
            float queuePriority=1.0;

            for(uint32_t queueFamily: uniqueQueueFamilies){
                VkDeviceQueueCreateInfo queueCreateInfo={};
                queueCreateInfo.sType=VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueCreateInfo.queueFamilyIndex= queueFamily;
                queueCreateInfo.queueCount=1;
                queueCreateInfo.pQueuePriorities=&queuePriority;

                queueCreateInfos.push_back(queueCreateInfo);
            }


            VkPhysicalDeviceFeatures deviceFeatures{}; //for now left blank due to no necessary device feeature needed.

            VkDeviceCreateInfo createInfo{};
            createInfo.sType=VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

            createInfo.enabledExtensionCount=static_cast<uint32_t>(deviceExtensions.size());
            createInfo.ppEnabledExtensionNames=deviceExtensions.data();

            createInfo.queueCreateInfoCount=static_cast<uint32_t>(queueCreateInfos.size());
            createInfo.pQueueCreateInfos=queueCreateInfos.data();
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
            vkGetDeviceQueue(device,indices.presentFamily.value(),0,&presentQueue);
        }
        
        struct SwapChainSupportDetails{
            VkSurfaceCapabilitiesKHR capabilities;
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> presentModes;
        };

        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device){
            SwapChainSupportDetails details;
            
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device,surface,&details.capabilities);
            
            uint32_t formatCount;
            vkGetPhysicalDeviceSurfaceFormatsKHR(device,surface,&formatCount,nullptr);
            if(formatCount!=0){
                details.formats.resize(formatCount);
                vkGetPhysicalDeviceSurfaceFormatsKHR(device,surface,&formatCount,details.formats.data());
            }

            uint32_t presentModeCount;
            vkGetPhysicalDeviceSurfacePresentModesKHR(device,surface,&presentModeCount,nullptr);
            if(presentModeCount!=0){
                details.presentModes.resize(presentModeCount);
                vkGetPhysicalDeviceSurfacePresentModesKHR(device,surface,&presentModeCount,details.presentModes.data());
            }
            return details;
        }

        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> availableFormats){

            for(const auto& availableFormat: availableFormats){
                if(availableFormat.format==VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace==VK_COLOR_SPACE_SRGB_NONLINEAR_KHR){

                    return availableFormat;
                }
            }
            return availableFormats[0];
        }

        VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR> availablePresentModes){

            
            for(const auto& presentMode:availablePresentModes){

                if(presentMode == VK_PRESENT_MODE_MAILBOX_KHR){

                    return presentMode;
                }

            }
            return VK_PRESENT_MODE_FIFO_KHR;

        }

        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities){

            if(capabilities.currentExtent.height != std::numeric_limits<uint32_t>::max()){
                //Means that Vulkan set the height and width value
                return capabilities.currentExtent;
            }
            else{
                //Means we need to specify the values

                int width,height;
                glfwGetFramebufferSize(window,&width,&height);

                VkExtent2D actualExtent={
                    static_cast<uint32_t>(width),
                    static_cast<uint32_t>(height)
                };

                //clap the width height values within the extent capabilities of the system
                actualExtent.width=std::clamp(actualExtent.width,capabilities.minImageExtent.width,capabilities.maxImageExtent.width);
                actualExtent.height=std::clamp(actualExtent.height,capabilities.minImageExtent.height,capabilities.maxImageExtent.height);

                return actualExtent;
            }
        }

        void createSwapChain(){

            SwapChainSupportDetails swapChainSupport= querySwapChainSupport(physicalDevice);
            VkSurfaceFormatKHR surfaceFormat= chooseSwapSurfaceFormat(swapChainSupport.formats); 
            VkPresentModeKHR presentMode= choosePresentMode(swapChainSupport.presentModes);
            VkExtent2D extent= chooseSwapExtent(swapChainSupport.capabilities);

            uint32_t imageCount= swapChainSupport.capabilities.minImageCount+1;
            
            //we should not exceed maxImageCount limit if there is 
            //maxImageCount==0 is special value indicates there is no image limit
            if(swapChainSupport.capabilities.maxImageCount!=0 && imageCount > swapChainSupport.capabilities.maxImageCount){

                imageCount= swapChainSupport.capabilities.maxImageCount;
            }

            VkSwapchainCreateInfoKHR createInfo{};
            createInfo.sType=VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
            createInfo.surface=surface;
            createInfo.minImageCount=imageCount;
            createInfo.imageFormat=surfaceFormat.format;
            createInfo.imageColorSpace=surfaceFormat.colorSpace;
            createInfo.imageExtent=extent;
            createInfo.imageArrayLayers=1;
            createInfo.imageUsage= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

            QueueFamilyIndices indices= findQueueFamily(physicalDevice);

            uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(),indices.presentFamily.value()};
            
            //if the graphicsFamily and presentFamily are in different que families
            if(indices.graphicsFamily != indices.presentFamily){

                createInfo.imageSharingMode=VK_SHARING_MODE_CONCURRENT; // both families has the image
                createInfo.queueFamilyIndexCount=2;
                createInfo.pQueueFamilyIndices=queueFamilyIndices;

            }
            // else if the same queueFamily
            else{

                createInfo.imageSharingMode=VK_SHARING_MODE_EXCLUSIVE; // ?one of the families has the ownship?
                createInfo.queueFamilyIndexCount=0;
                createInfo.pQueueFamilyIndices=nullptr; // no need to specify the queue family indices hence both are in the same queue family

            }

            createInfo.preTransform=swapChainSupport.capabilities.currentTransform;
            createInfo.compositeAlpha= VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            createInfo.presentMode=presentMode;
            createInfo.clipped=VK_TRUE;
            createInfo.oldSwapchain=VK_NULL_HANDLE;

            if(vkCreateSwapchainKHR(device,&createInfo,nullptr,&swapChain)!=VK_SUCCESS){
                throw std::runtime_error("failed to create swapchain!");
            }

            vkGetSwapchainImagesKHR(device,swapChain,&imageCount,nullptr);
            swapChainImages.resize(imageCount);
            vkGetSwapchainImagesKHR(device,swapChain,&imageCount,swapChainImages.data());
            
            swapChainImageFormat=surfaceFormat.format;
            swapChainExtent=extent;
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
            vkDestroySwapchainKHR(device,swapChain,nullptr);
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
        VkQueue presentQueue;
        VkSwapchainKHR swapChain;
        std::vector<VkImage> swapChainImages;
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;

        const std::vector<const char*> deviceExtensions={
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };



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
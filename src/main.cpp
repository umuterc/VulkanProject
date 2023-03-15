#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include<stdexcept>
#include<cstdlib>
#include<vector>

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

        }

        void createInstance(){

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
            createInfo.enabledLayerCount=0;
            if(vkCreateInstance(&createInfo,nullptr,&instance)!=VK_SUCCESS){
                throw std::runtime_error("failed to create instance!");
            }
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
            vkDestroyInstance(instance,nullptr);

            glfwDestroyWindow(window);
            glfwTerminate();
        }

        //WINDOW
        GLFWwindow* window;
        const uint32_t WIDTH=800;
        const uint32_t HEIGHT=600;

        //VULKAN
        VkInstance instance;
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
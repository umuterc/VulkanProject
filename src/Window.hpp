#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include<string>

namespace VulkanEngine{

class Window
{
private:
    GLFWwindow* window;
    int width=800;
    int height=600;
    std::string WindowName;

    bool frameBufferResized=false;
    
    void initWindow();//call from constructer
    static void errorCallback(int error, const char* description);
    static void frameBufferResizeCallback(GLFWwindow* window, int width,int height);

public:
    Window(int width,int height, std::string WindowName);
    Window();
    ~Window();
    bool windowShouldClose();
    void fetchFrameBufferSize();
    VkResult createSurface(VkInstance instance,VkSurfaceKHR* surface);

    bool wasWindowResized();
    void resetWindowResizedFlag();
    VkExtent2D getExtent();
};

};




#include"Window.hpp"
#include <iostream>
#include<stdexcept>


namespace VulkanEngine{

Window::Window(int width,int height,std::string WindowName="Vulkan"):width(width),height(height),WindowName(WindowName)
{
    initWindow();
}

Window::Window()
{
    initWindow();
}

Window::~Window()
{  
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Window::fetchFrameBufferSize(){
        glfwGetFramebufferSize(window,&width,&height);
}

bool Window::windowShouldClose(){
    return glfwWindowShouldClose(window);
}

VkResult Window::createSurface(VkInstance instance,VkSurfaceKHR* surface){
       VkResult result = glfwCreateWindowSurface(instance,window,nullptr,surface);

       return result;
}

void Window::initWindow() {

    glfwSetErrorCallback(errorCallback);

    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return;
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window= glfwCreateWindow(width,height,WindowName.c_str(),nullptr,nullptr);
    glfwSetWindowUserPointer(window,this);
    glfwSetFramebufferSizeCallback(window,frameBufferResizeCallback);
}

void Window::errorCallback(int error, const char* description){
    std::cerr << "GLFW Error: " << description << std::endl;
}
void Window::frameBufferResizeCallback(GLFWwindow* window, int width,int height){
    auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    app->frameBufferResized = true;
}

bool Window::wasWindowResized(){
   return frameBufferResized;
}

void Window::resetWindowResizedFlag(){
    frameBufferResized=false;
}

VkExtent2D Window::getExtent(){

    VkExtent2D extent{
        .height=static_cast<uint32_t>(height),
        .width=static_cast<uint32_t>(width)
    };
    
    return extent;
}

};

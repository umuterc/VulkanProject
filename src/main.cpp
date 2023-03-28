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
#include<fstream>
#include<filesystem>

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
            createImageViews();
            createRenderPass();
            createGraphicsPipeline();
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

         void createImageViews(){
            swapChainImageViews.resize(swapChainImages.size());

            for(size_t i=0;i<swapChainImageViews.size(); ++i){
                VkImageViewCreateInfo createInfo{};

                createInfo.sType=VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                createInfo.image=swapChainImages[i];
                createInfo.viewType= VK_IMAGE_VIEW_TYPE_2D;
                createInfo.format=swapChainImageFormat;

                createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
                createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
                createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
                createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

                createInfo.subresourceRange.aspectMask=VK_IMAGE_ASPECT_COLOR_BIT;
                createInfo.subresourceRange.baseArrayLayer=0;
                createInfo.subresourceRange.baseMipLevel=0;
                createInfo.subresourceRange.layerCount=1;
                createInfo.subresourceRange.levelCount=1; 

                if(vkCreateImageView(device,&createInfo,nullptr,&swapChainImageViews[i])!=VK_SUCCESS){
                    throw std::runtime_error("failed to create image views!");
                }
            
            }
         }

        static std::vector<char> readFile(const std::string& fileName){
            std::ifstream file(fileName, std::ios::ate | std::ios::binary);

            if (!file.is_open()) {
                throw std::runtime_error("failed to open file!");
            }

            size_t fileSize= (size_t)file.tellg();
            std::vector<char> buffer(fileSize);

            file.seekg(0);
            file.read(buffer.data(),fileSize);

            file.close();

            return buffer;
        }

         void createGraphicsPipeline(){
            namespace fs = std::filesystem;
            std::cout << "Current path is " << fs::current_path() << '\n'; // (1)

            //read compiled shader code
            auto vertShaderCode = readFile("../../assets/shaders/vert.spv"); //TODO: Instead give the assets path to the cmake
            auto fragShaderCode = readFile("../../assets/shaders/frag.spv"); //TODO: Instead give the assets path to the cmake

            //wrap shader code with shader modules
            VkShaderModule vertShaderModule= createShaderModule(vertShaderCode);
            VkShaderModule fragShaderModule= createShaderModule(fragShaderCode);

            //create info to bind the shaders to the relevant pipeline stages
            VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
            {
                vertShaderStageInfo.sType=VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                vertShaderStageInfo.stage=VK_SHADER_STAGE_VERTEX_BIT;
                vertShaderStageInfo.module=vertShaderModule;
                vertShaderStageInfo.pName="main"; //specifies entry point
            }
            //create info to bind the shaders to the relevant pipeline stages
            VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
            {
                fragShaderStageInfo.sType=VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                fragShaderStageInfo.stage=VK_SHADER_STAGE_FRAGMENT_BIT;
                fragShaderStageInfo.module=fragShaderModule;
                fragShaderStageInfo.pName="main"; //specifies entry point
            }

            VkPipelineShaderStageCreateInfo shaderStages[]={vertShaderStageInfo,fragShaderStageInfo};

            //specify the dynamic states ??
            std::vector<VkDynamicState> dynamicStates={
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR
            };

            VkPipelineDynamicStateCreateInfo dynamicState{};
            {
                dynamicState.sType=VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
                dynamicState.dynamicStateCount=dynamicStates.size();
                dynamicState.pDynamicStates=dynamicStates.data();
            }

            //Specify the vertex shader input data
            VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
            {
                vertexInputInfo.sType=VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
                vertexInputInfo.pVertexAttributeDescriptions=nullptr;
                vertexInputInfo.vertexAttributeDescriptionCount=0;
                vertexInputInfo.pVertexBindingDescriptions=nullptr;
                vertexInputInfo.vertexBindingDescriptionCount=0;
            }

            //Specify how to draw the primitives
            VkPipelineInputAssemblyStateCreateInfo inputAssambly{};
            {
                inputAssambly.sType=VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
                inputAssambly.topology=VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
                inputAssambly.primitiveRestartEnable=VK_FALSE;
            }

            //Viewport

            // VkViewport viewport{};
            // {
            //     viewport.x=0.0f;
            //     viewport.y=0.0f;
            //     viewport.height= (float)swapChainExtent.height;
            //     viewport.width=(float)swapChainExtent.width;
            //     viewport.minDepth=0.0f;
            //     viewport.maxDepth=1.0f;
            // }

            // VkRect2D scissor{
            //     .extent=swapChainExtent,
            //     .offset={0,0}
            // };

            VkPipelineViewportStateCreateInfo viewportState{};
            {
                viewportState.sType=VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
                viewportState.viewportCount=1;
                viewportState.scissorCount=1;
            }

            VkPipelineRasterizationStateCreateInfo rasterizer{};
            {
                rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
                rasterizer.depthClampEnable=VK_FALSE; // if VK_TRUE do not discard the fragments which falls out of the view frustum
                rasterizer.rasterizerDiscardEnable=VK_FALSE; //if true discard all the fragments in the rasterizatio stage.No output to the fragment shader stage
                rasterizer.polygonMode=VK_POLYGON_MODE_FILL; //Using any mode other than fill requires enabling a GPU feature.
                rasterizer.lineWidth=1.0f; //any line thicker than 1.0f requires you to enable the wideLines GPU feature.
                rasterizer.cullMode=VK_CULL_MODE_BACK_BIT;
                rasterizer.frontFace=VK_FRONT_FACE_CLOCKWISE;
                rasterizer.depthBiasEnable=VK_FALSE;
            }


        
            VkPipelineMultisampleStateCreateInfo multisampling{}; //MSAA info 
            {
                multisampling.sType=VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
                multisampling.sampleShadingEnable=VK_FALSE; //
                multisampling.rasterizationSamples=VK_SAMPLE_COUNT_1_BIT;
            }

            //VkPipelineDepthStencilStateCreateInfo deptstencil{};

            VkPipelineColorBlendAttachmentState colorBlendAttachment{};
            {
                colorBlendAttachment.colorWriteMask= VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
                colorBlendAttachment.blendEnable = VK_FALSE;
            }

            VkPipelineColorBlendStateCreateInfo colorBlending{};
            {
                colorBlending.sType=VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
                colorBlending.logicOpEnable=VK_FALSE;
                colorBlending.attachmentCount=1;
                colorBlending.pAttachments=&colorBlendAttachment;
            }

            VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
            {
                pipelineLayoutInfo.sType=VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
                pipelineLayoutInfo.setLayoutCount = 0; // Optional
                pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
                pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
                pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
            }

            if(vkCreatePipelineLayout(device,&pipelineLayoutInfo,nullptr,&pipelineLayout)!=VK_SUCCESS){
                throw std::runtime_error("failed to create pipeline layout!");
            }

            VkGraphicsPipelineCreateInfo pipelineInfo{};
            {
                pipelineInfo.sType=VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
                pipelineInfo.stageCount=2; //# of used programmable stages (here vertex shader and fragment shader)
                pipelineInfo.pStages=shaderStages;
                pipelineInfo.pVertexInputState=&vertexInputInfo;
                pipelineInfo.pInputAssemblyState=&inputAssambly;
                pipelineInfo.pViewportState=&viewportState;
                pipelineInfo.pRasterizationState=&rasterizer;
                pipelineInfo.pMultisampleState=&multisampling;
                pipelineInfo.pDepthStencilState=nullptr;
                pipelineInfo.pColorBlendState=&colorBlending;
                pipelineInfo.pDynamicState=&dynamicState;
                pipelineInfo.layout=pipelineLayout;
                pipelineInfo.renderPass=renderPass;
                pipelineInfo.subpass=0; //describes the index of the subpass where this graphics pipeline will be used

                pipelineInfo.basePipelineHandle=VK_NULL_HANDLE; //not inherited from a base pipeline
                pipelineInfo.basePipelineIndex=-1;              //not inherited from a base pipeline

            }

            if(vkCreateGraphicsPipelines(device,VK_NULL_HANDLE,1,&pipelineInfo,nullptr,&graphicsPipeline)!=VK_SUCCESS){
                throw std::runtime_error("failed to create graphics pipeline!");
            }

            vkDestroyShaderModule(device,vertShaderModule,nullptr);
            vkDestroyShaderModule(device,fragShaderModule,nullptr);
         }

        void createRenderPass(){

            VkAttachmentDescription colorAttachment{};
            {
                colorAttachment.format= swapChainImageFormat;
                colorAttachment.samples=VK_SAMPLE_COUNT_1_BIT;
                colorAttachment.loadOp=VK_ATTACHMENT_LOAD_OP_CLEAR; //Clear the framebuffer before drawing
                colorAttachment.storeOp=VK_ATTACHMENT_STORE_OP_STORE; //We will show the colors on the screen
                colorAttachment.stencilLoadOp=VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                colorAttachment.stencilStoreOp=VK_ATTACHMENT_STORE_OP_DONT_CARE;
                colorAttachment.initialLayout=VK_IMAGE_LAYOUT_UNDEFINED;
                colorAttachment.finalLayout=VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            }
            VkAttachmentReference colorAttachmentReference{};
            {
                colorAttachmentReference.attachment=0;//idx of the attachment colorAttachment array
                colorAttachmentReference.layout=VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            }
            VkSubpassDescription subpass{};
            {
                subpass.pipelineBindPoint=VK_PIPELINE_BIND_POINT_GRAPHICS;
                subpass.colorAttachmentCount=1;
                subpass.pColorAttachments=&colorAttachmentReference;
            }

            VkRenderPassCreateInfo renderPassCreateInfo{};
            {
                renderPassCreateInfo.sType=VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
                renderPassCreateInfo.attachmentCount=1;
                renderPassCreateInfo.subpassCount=1;
                renderPassCreateInfo.pAttachments=&colorAttachment;
                renderPassCreateInfo.pSubpasses=&subpass;
            }

            if(vkCreateRenderPass(device,&renderPassCreateInfo,nullptr,&renderPass)!=VK_SUCCESS){

                throw std::runtime_error("failed to create render pass!");
            }

        }

        VkShaderModule createShaderModule(const std::vector<char>& code){
            VkShaderModuleCreateInfo createInfo{};
            {
                createInfo.sType=VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
                createInfo.codeSize=code.size();
                createInfo.pCode=reinterpret_cast<const uint32_t*>(code.data());
            }

            VkShaderModule shaderModule;

            if(vkCreateShaderModule(device,&createInfo,nullptr,&shaderModule)!=VK_SUCCESS){

                throw std::runtime_error("Failed to create shader module!");
            }
            return shaderModule;

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

            vkDestroyPipeline(device,graphicsPipeline,nullptr);
            vkDestroyPipelineLayout(device,pipelineLayout,nullptr);
            vkDestroyRenderPass(device,renderPass,nullptr);

            for(auto imageView: swapChainImageViews){
                vkDestroyImageView(device,imageView,nullptr);
            }

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

        VkRenderPass renderPass;
        VkPipelineLayout pipelineLayout;

        VkPipeline graphicsPipeline;

        std::vector<VkImageView> swapChainImageViews;

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
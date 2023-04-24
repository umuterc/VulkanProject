#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include<glm/glm.hpp>

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
#include<array>

class HelloTriangleApplication{

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
            createFrameBuffers();
            createCommandPool();
            createVertexBuffer();
            createCommandBuffers();
            createSyncObjects();
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
            
            std::vector<const char*> requiredExtensions;

            for(uint32_t i = 0; i < glfwExtensionCount; i++) {
                requiredExtensions.emplace_back(glfwExtensions[i]);
            }

            requiredExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
            requiredExtensions.emplace_back("VK_KHR_get_physical_device_properties2");

            createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

            createInfo.enabledExtensionCount = (uint32_t) requiredExtensions.size();
            createInfo.ppEnabledExtensionNames = requiredExtensions.data();

            if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
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

        void cleanUpSwapChain(){
             for (auto framebuffer : swapChainFrameBuffers) {
                vkDestroyFramebuffer(device, framebuffer, nullptr);
            }

            for(auto imageView: swapChainImageViews){
                vkDestroyImageView(device,imageView,nullptr);
            }

            vkDestroySwapchainKHR(device,swapChain,nullptr);

        }

        // In case of window resize the swapchain is becoming incompatible so it needs to be recreated.
        void recreateSwapChain(){
            int width = 0, height = 0;
            glfwGetFramebufferSize(window, &width, &height);
            while (width == 0 || height == 0) {
                glfwGetFramebufferSize(window, &width, &height);
                glfwWaitEvents();
            }
            vkDeviceWaitIdle(device);

            cleanUpSwapChain();
            createSwapChain();
            createImageViews();
            createFrameBuffers();

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
                auto bindingDescription=Vertex::getBindingDescription();
                auto attributeDescriptions=Vertex::getAttributeDescriptions();

                vertexInputInfo.sType=VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
                vertexInputInfo.vertexBindingDescriptionCount=1;
                vertexInputInfo.vertexAttributeDescriptionCount=attributeDescriptions.size();
                vertexInputInfo.pVertexAttributeDescriptions=attributeDescriptions.data();
                vertexInputInfo.pVertexBindingDescriptions=&bindingDescription;
            }

            //Specify how to draw the primitives
            VkPipelineInputAssemblyStateCreateInfo inputAssambly{};
            {
                inputAssambly.sType=VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
                inputAssambly.topology=VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
                inputAssambly.primitiveRestartEnable=VK_FALSE;
            }

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

            VkSubpassDependency dependency{};
            {
                dependency.srcSubpass=VK_SUBPASS_EXTERNAL;
                dependency.dstSubpass=0;
                dependency.srcStageMask=VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                dependency.srcAccessMask=0;
                dependency.dstStageMask=VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                dependency.dstAccessMask=VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            }

            VkRenderPassCreateInfo renderPassCreateInfo{};
            {
                renderPassCreateInfo.sType=VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
                renderPassCreateInfo.attachmentCount=1;
                renderPassCreateInfo.subpassCount=1;
                renderPassCreateInfo.pAttachments=&colorAttachment;
                renderPassCreateInfo.pSubpasses=&subpass;
                renderPassCreateInfo.dependencyCount=1;
                renderPassCreateInfo.pDependencies=&dependency;
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

        void createFrameBuffers(){
            swapChainFrameBuffers.resize(swapChainImageViews.size());

            for(size_t i=0;i<swapChainImageViews.size();++i ){

                VkImageView attachments[]={
                    swapChainImageViews[i]
                };

                VkFramebufferCreateInfo frameBufferInfo{};
                {
                    frameBufferInfo.sType=VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                    frameBufferInfo.renderPass=renderPass;
                    frameBufferInfo.attachmentCount=1;
                    frameBufferInfo.pAttachments= attachments;
                    frameBufferInfo.width=swapChainExtent.width;
                    frameBufferInfo.height=swapChainExtent.height;
                    frameBufferInfo.layers=1; //number of layers in image arrays
                }
                if(vkCreateFramebuffer(device,&frameBufferInfo,nullptr,&swapChainFrameBuffers[i])!=VK_SUCCESS){

                    throw std::runtime_error("failed to create frame buffer!");
                }
            }
        }

        void createCommandPool(){

                QueueFamilyIndices queueFamilyIndices= findQueueFamily(physicalDevice);
                
                VkCommandPoolCreateInfo poolInfo{};
                {
                    poolInfo.sType=VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
                    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
                    poolInfo.queueFamilyIndex=queueFamilyIndices.graphicsFamily.value(); // we have to set the command pool to a 
                }
                if(vkCreateCommandPool(device,&poolInfo,nullptr,&commandPool)!=VK_SUCCESS){

                    throw std::runtime_error("failed to create command pool!");
                }
        }

        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size){

            VkCommandBufferAllocateInfo allocInfo{};
            {
                allocInfo.sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                allocInfo.level=VK_COMMAND_BUFFER_LEVEL_PRIMARY;
                allocInfo.commandPool=commandPool;
                allocInfo.commandBufferCount=1;
            }

            VkCommandBuffer commandBuffer;
            vkAllocateCommandBuffers(device,&allocInfo,&commandBuffer);

            VkCommandBufferBeginInfo beginInfo{};
            {
                beginInfo.sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                beginInfo.flags=VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            }

            vkBeginCommandBuffer(commandBuffer,&beginInfo);

                VkBufferCopy copyRegion{};
                {
                    copyRegion.srcOffset=0;
                    copyRegion.dstOffset=0;
                    copyRegion.size=size;
                }
                vkCmdCopyBuffer(commandBuffer,srcBuffer,dstBuffer,1,&copyRegion);

            vkEndCommandBuffer(commandBuffer);

            VkSubmitInfo submitInfo{};
            {
                submitInfo.sType=VK_STRUCTURE_TYPE_SUBMIT_INFO;
                submitInfo.commandBufferCount=1;
                submitInfo.pCommandBuffers=&commandBuffer;
            }

            vkQueueSubmit(graphicsQueue,1,&submitInfo,VK_NULL_HANDLE);
            vkQueueWaitIdle(graphicsQueue); //@TODO: Add fence for copying

            vkFreeCommandBuffers(device,commandPool,1,&commandBuffer);

        }

        void createBuffer(VkDeviceSize size,VkBufferUsageFlags usage,VkMemoryPropertyFlags properties,VkBuffer& buffer,VkDeviceMemory& bufferMemory){

            VkBufferCreateInfo bufferInfo{};
            {   
                bufferInfo.sType=VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                bufferInfo.size=size;
                bufferInfo.usage=usage;
                bufferInfo.sharingMode=VK_SHARING_MODE_EXCLUSIVE; //owned by a single queue family at a time and ownership must be explicitly transfered before using it in another queue family
                bufferInfo.flags=0;
            }
            if(vkCreateBuffer(device,&bufferInfo,nullptr,&buffer)!=VK_SUCCESS){
                throw std::runtime_error("failed to create vertex buffer!");
            }

            VkMemoryRequirements memRequirements;
            vkGetBufferMemoryRequirements(device,buffer,&memRequirements);

            VkMemoryAllocateInfo allocateInfo{};
            {
                allocateInfo.sType=VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                allocateInfo.allocationSize=memRequirements.size;
                allocateInfo.memoryTypeIndex=findMemoryType(memRequirements.memoryTypeBits, properties); // @TODO: Test if cached is faster
            }

            if(vkAllocateMemory(device,&allocateInfo,nullptr,&bufferMemory)!=VK_SUCCESS){
                throw std::runtime_error("failed to allocate vertex buffer memory!");
            }

            vkBindBufferMemory(device,buffer,bufferMemory,0);

        }

        void createVertexBuffer(){

            VkDeviceSize bufferSize= sizeof(vertices[0])*vertices.size();
                
            VkBuffer stagingBuffer;
            VkDeviceMemory stagingBufferMemory;
           
            createBuffer(bufferSize,VK_BUFFER_USAGE_TRANSFER_SRC_BIT,VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,stagingBuffer,stagingBufferMemory);          

            void* data;
            vkMapMemory(device,stagingBufferMemory,0,bufferSize,0,&data);
            memcpy(data,vertices.data(),(size_t)bufferSize);
            vkUnmapMemory(device,stagingBufferMemory);

            createBuffer(bufferSize,VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,vertexBuffer,vertexBufferMemory);
            copyBuffer(stagingBuffer,vertexBuffer,bufferSize);

            vkDestroyBuffer(device,stagingBuffer,nullptr);
            vkFreeMemory(device,stagingBufferMemory,nullptr);

        }


        //This function will find the memory type that is suitable for the buffer corresponding to the properties and type filter
        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties){

            VkPhysicalDeviceMemoryProperties memProperties;
            vkGetPhysicalDeviceMemoryProperties(physicalDevice,&memProperties);

            for(uint32_t i=0; i<memProperties.memoryTypeCount; i++){
                if(typeFilter & (1<<i) && (memProperties.memoryTypes[i].propertyFlags & properties)== properties){
                    return i;
                }
            }
            throw std::runtime_error("failed to find suitable memory type!");
        }

        void createCommandBuffers(){
            commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
            
            VkCommandBufferAllocateInfo allocInfo{};
            {
                allocInfo.sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                allocInfo.commandPool=commandPool;
                allocInfo.level=VK_COMMAND_BUFFER_LEVEL_PRIMARY;
                allocInfo.commandBufferCount=commandBuffers.size();
            }
            if(vkAllocateCommandBuffers(device,&allocInfo,commandBuffers.data())!=VK_SUCCESS){
                throw std::runtime_error("failed to allocate command buffer!");
            }
        }

        void recordCommanbuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex){

            VkCommandBufferBeginInfo beginInfo{};
            {
                beginInfo.sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                beginInfo.flags=0; //optional
                beginInfo.pInheritanceInfo=nullptr; //optional
            }

            if(vkBeginCommandBuffer(commandBuffer,&beginInfo)!=VK_SUCCESS){
                throw std::runtime_error("failed to begin recording command buffer!");
            }

            VkClearValue clearColor={{{0.0f, 0.0f, 0.0f, 1.0f}}};

            VkRenderPassBeginInfo renderPassInfo{};
            {
                renderPassInfo.sType=VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                renderPassInfo.renderPass=renderPass;
                renderPassInfo.framebuffer=swapChainFrameBuffers[imageIndex];
                renderPassInfo.renderArea.offset={0,0};
                renderPassInfo.renderArea.extent=swapChainExtent;
                renderPassInfo.clearValueCount=1;
                renderPassInfo.pClearValues=&clearColor;
            }

            vkCmdBeginRenderPass(commandBuffer,&renderPassInfo,VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(commandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,graphicsPipeline);

            VkBuffer vertexBuffers[]= {vertexBuffer};
            VkDeviceSize offsets[]={0};
            vkCmdBindVertexBuffers(commandBuffer,0,1,vertexBuffers,offsets);

            VkViewport viewport{};
            {
                viewport.x=0.0f;
                viewport.y=0.0f;
                viewport.height= static_cast<float>(swapChainExtent.height);
                viewport.width=static_cast<float>(swapChainExtent.width);
                viewport.minDepth=0.0f;
                viewport.maxDepth=1.0f;
            }
            vkCmdSetViewport(commandBuffer,0,1,&viewport);

            VkRect2D scissor{
                .extent=swapChainExtent,
                .offset={0,0}
            };
            vkCmdSetScissor(commandBuffer,0,1,&scissor);
            vkCmdDraw(commandBuffer,static_cast<uint32_t>(vertices.size()),1,0,0);
            vkCmdEndRenderPass(commandBuffer);
            
            if(vkEndCommandBuffer(commandBuffer)!=VK_SUCCESS){

                throw std::runtime_error("failed to record command buffer!");
            }

        }
       
        void createSyncObjects(){

            imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
            renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
            inFlightfences.resize(MAX_FRAMES_IN_FLIGHT);

            VkSemaphoreCreateInfo semaphoreInfo{};
            {
                semaphoreInfo.sType=VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            }
            VkFenceCreateInfo fenceInfo{};
            {   
                fenceInfo.sType=VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
                fenceInfo.flags=VK_FENCE_CREATE_SIGNALED_BIT; // create fence as signaled initially
            }
            for(size_t i=0;i<MAX_FRAMES_IN_FLIGHT;++i){
                if( vkCreateSemaphore(device,&semaphoreInfo,nullptr,&imageAvailableSemaphores[i]) ||
                    vkCreateSemaphore(device,&semaphoreInfo,nullptr,&renderFinishedSemaphores[i]) ||
                    vkCreateFence(device,&fenceInfo,nullptr,&inFlightfences[i]) != VK_SUCCESS
                ){
                    throw std::runtime_error("failed to create sync objects!");
                }
            }

        }

        static void errorCallback(int error, const char* description){
            std::cerr << "GLFW Error: " << description << std::endl;
        }
        
        void initWindow(){
            
            glfwSetErrorCallback(errorCallback);

            if (!glfwInit())
            {
                std::cerr << "Failed to initialize GLFW" << std::endl;
                return;
            }
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

            window= glfwCreateWindow(WIDTH,HEIGHT,"Vulkan",nullptr,nullptr);
            glfwSetWindowUserPointer(window,this); // idk why
            glfwSetFramebufferSizeCallback(window,frameBufferResizeCallback);
            glfwSetWindowCloseCallback(window,windowCloseCallback);

        }

        static void frameBufferResizeCallback(GLFWwindow* window, int width,int height){
            auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
            app->frameBufferResized = true;
        }

        static void windowCloseCallback(GLFWwindow* window)
        {
            // Set the window should close flag
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        void mainLoop(){
            while(!glfwWindowShouldClose(window)){
                drawFrame();
                glfwPollEvents();
            }
             vkDeviceWaitIdle(device);
        }

        void drawFrame(){

            vkWaitForFences(device,1,&inFlightfences[currentFrame],VK_TRUE,UINT64_MAX); // wait for the previous frame

            uint32_t imageIdx;
            VkResult result = vkAcquireNextImageKHR(device,swapChain,UINT64_MAX,imageAvailableSemaphores[currentFrame],VK_NULL_HANDLE,&imageIdx);

            if(result==VK_ERROR_OUT_OF_DATE_KHR){
                recreateSwapChain();
            }
            else if(result!=VK_SUCCESS && result!=VK_SUBOPTIMAL_KHR){
                throw std::runtime_error("failed to acquire swapchain image!");
            }

            // Only reset the fence if we are submitting work
            vkResetFences(device, 1, &inFlightfences[currentFrame]);

            vkResetCommandBuffer(commandBuffers[currentFrame],0);// to bring the commandbuffer to the initial state. If command buffer is in the pending command buffer cannot be recorded.
            recordCommanbuffer(commandBuffers[currentFrame],imageIdx);

            VkSemaphore waitSemaphores[]={imageAvailableSemaphores[currentFrame]};
            VkPipelineStageFlags waitFlags[]={VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

            VkSemaphore signalSemaphores[]={renderFinishedSemaphores[currentFrame]};
        
            VkSubmitInfo submitInfo{};
            {
                submitInfo.sType=VK_STRUCTURE_TYPE_SUBMIT_INFO;
                submitInfo.waitSemaphoreCount=1;
                submitInfo.pWaitSemaphores=waitSemaphores; //which semaphore
                submitInfo.pWaitDstStageMask=waitFlags;    //which stage to wait
                submitInfo.commandBufferCount=1;
                submitInfo.pCommandBuffers=&commandBuffers[currentFrame];
                submitInfo.signalSemaphoreCount=1;
                submitInfo.pSignalSemaphores=signalSemaphores;
            }

            if(vkQueueSubmit(graphicsQueue,1,&submitInfo,inFlightfences[currentFrame])!=VK_SUCCESS){
                throw std::runtime_error("failed to submit draw command buffer!");
            }

            VkSwapchainKHR swapChains[]={swapChain}; 

            VkPresentInfoKHR presentInfo{};
            {
                presentInfo.sType=VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
                presentInfo.waitSemaphoreCount=1;
                presentInfo.pWaitSemaphores=signalSemaphores; //wait the signal to present the image
                presentInfo.swapchainCount=1;
                presentInfo.pSwapchains=swapChains;
                presentInfo.pImageIndices=&imageIdx;
                presentInfo.pResults=nullptr;
            }

            result = vkQueuePresentKHR(graphicsQueue,&presentInfo);

            if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || frameBufferResized) {
                frameBufferResized=false;
                recreateSwapChain();
            } else if (result != VK_SUCCESS) {
                throw std::runtime_error("failed to present swap chain image!");
            }
            currentFrame=(currentFrame+1)%MAX_FRAMES_IN_FLIGHT;

        }

        void cleanup(){

            cleanUpSwapChain();

            vkDestroyBuffer(device,vertexBuffer,nullptr);
            vkFreeMemory(device,vertexBufferMemory,nullptr);

            for(size_t i=0; i<MAX_FRAMES_IN_FLIGHT;++i){
                vkDestroySemaphore(device,imageAvailableSemaphores[i],nullptr);
                vkDestroySemaphore(device,renderFinishedSemaphores[i],nullptr);
                vkDestroyFence(device,inFlightfences[i],nullptr);
            }
            vkDestroyCommandPool(device,commandPool,nullptr);
            vkDestroyPipeline(device,graphicsPipeline,nullptr);
            vkDestroyPipelineLayout(device,pipelineLayout,nullptr);
            vkDestroyRenderPass(device,renderPass,nullptr);
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
        std::vector<VkFramebuffer> swapChainFrameBuffers;

        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;

        VkCommandPool commandPool;
        std::vector<VkCommandBuffer> commandBuffers;

        //Synchronization objects
        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::vector<VkFence> inFlightfences;
        
        bool frameBufferResized=false;
        const int MAX_FRAMES_IN_FLIGHT = 2;
        uint32_t currentFrame = 0;

        const std::vector<const char*> deviceExtensions={
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            "VK_KHR_portability_subset"
        };

        const std::vector<const char*> validationLayers = {
            "VK_LAYER_KHRONOS_validation"
        };

        #ifdef NDEBUG
            const bool enableValidationsLayers=false;
        #else
            const bool enableValidationLayers=true;
        #endif

        struct Vertex{
            glm::vec3 pos;
            glm::vec3 color;

            static VkVertexInputBindingDescription getBindingDescription(){
                VkVertexInputBindingDescription bindingDescription{};
                bindingDescription.binding=0;
                bindingDescription.stride=sizeof(Vertex);
                bindingDescription.inputRate=VK_VERTEX_INPUT_RATE_VERTEX;

                return bindingDescription;
            }

            static std::array<VkVertexInputAttributeDescription,2> getAttributeDescriptions(){
            std::array<VkVertexInputAttributeDescription,2> attributeDescriptions{};

            attributeDescriptions[0].binding=0;
            attributeDescriptions[0].location=0;
            attributeDescriptions[0].format=VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[0].offset=offsetof(Vertex,pos);

            attributeDescriptions[1].binding=0;
            attributeDescriptions[1].location=1;
            attributeDescriptions[1].format=VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[1].offset=offsetof(Vertex,color);

            return attributeDescriptions;
            }
        };

      

        //define vertex buffer
        const std::vector<Vertex> vertices={
            {{0.0f,-0.5f,0.0f},{1.0f,1.0f,1.0f}},
            {{0.5f,0.5f,0.0f},{0.0f,1.0f,0.0f}},
            {{-0.5f,0.5f,0.0f},{0.0f,0.0f,1.0f}}
        };
        

};


int main() {
    HelloTriangleApplication app;

    try{
        app.run();
    }catch(const std::exception &e){
        std::cerr<<e.what()<<std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
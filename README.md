# VulkanProject

This is a project that follows the [Vulkan Tutorial](https://vulkan-tutorial.com) using C++ and Vulkan.

## Requirements

- C++17 compiler
- [Vulkan SDK](https://vulkan.lunarg.com/sdk/home) (version 1.2.182 or later)
- [Git](https://git-scm.com/) (for cloning the repository and its submodules)
- [GLFW](https://www.glfw.org/) library (included as a submodule)
- [GLM](https://glm.g-truc.net/) library (included as a submodule)
- [CMake](https://cmake.org/) (version 3.12 or later)

Note: The GLFW and GLM libraries are included as submodules in this repository. After cloning this repository, you'll need to run the following commands to initialize the submodules:

```
git submodule init
git submodule update
```

This will clone the GLFW and GLM repositories into the `external/glfw` and `external/glm` directories, respectively.

## Getting Started

To build the project using CMake, follow these steps:

1. Clone the repository and its submodules:
```
git clone --recursive https://github.com/umuterc/VulkanProject
```
2. Create a `build` directory inside the repository: 
``` 
cd VulkanProject
mkdir build
cd build
```
3. Generate the build files using CMake: 
``` 
cmake ..
```
4. Build the project using the generated build files:
```
cmake --build
```
Note: If you're using a specific compiler or toolchain, you can specify it with the `-DCMAKE_CXX_COMPILER` flag:
```
cmake -DCMAKE_CXX_COMPILER=path/to/compiler ..
```
5. Run the executable:
```
./VulkanProject
```

That's it! You should now have a working Vulkan application. If you run into any issues, please consult the [Vulkan Tutorial website](https://vulkan-tutorial.com/) or create a new issue in the GitHub repository.

## Progress

- [x] Introduction to Vulkan and Setup
- [x] Instance Creation
- [x] Physical Devices and Queue Families
- [x] Logical Device and Queues
- [x] Window Surface
- [x] Swap Chain
- [x] Image Views
- [x] Graphics Pipeline Basics
- [x] Vertex Input Description
- [x] Shader Modules
- [x] Fixed Functions
- [x] Drawing Triangle

## Resources

- [Vulkan Tutorial](https://vulkan-tutorial.com) - Vulkan tutorial website.
- [Vulkan API Specification](https://www.vulkan.org/learn#key-resources) - The official Vulkan API documentation.
- [Vulkan Guide](https://github.com/KhronosGroup/Vulkan-Guide?utm_source=DevRel&utm_medium=KHRhomepage&utm_campaign=Vulkan_Guide) - Vulkan guide github repository.
- [GLFW Documentation](https://www.glfw.org/docs/latest/) - The official GLFW documentation.
- [GLM Documentation](https://glm.g-truc.net/0.9.9/index.html) - The official GLM documentation.

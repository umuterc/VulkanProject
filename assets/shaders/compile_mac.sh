#!/bin/sh

# Set execute permission on this script
chmod +x "$0"

# Compile vertex shader
/Users/umutercan/VulkanSDK/1.3.239.0/macOS/bin/glslc shader.vert -o vert.spv || {
    echo "Error: Failed to compile vertex shader"
    exit 1
}

# Compile fragment shader
/Users/umutercan/VulkanSDK/1.3.239.0/macOS/bin/glslc shader.frag -o frag.spv || {
    echo "Error: Failed to compile fragment shader"
    exit 1
}

# Print directory of compiled shader program binary
echo "Compiled shader program binary located in $(pwd)"

# Success message
echo "Shaders compiled successfully"

if [ ! -d "./Static/Shaders" ]; then
    mkdir Static/Shaders
fi
$VULKAN_SDK/bin/glslangValidator -V Source/Graphics/Shaders/shader.vert -o Static/Shaders/vert.spv
$VULKAN_SDK/bin/glslangValidator -V Source/Graphics/Shaders/shader.frag -o Static/Shaders/frag.spv

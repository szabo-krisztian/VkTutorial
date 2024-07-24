![fractal](https://github.com/szabo-krisztian/VkTutorial/blob/master/images/fractal.png?raw=true)

# Sierpiński triangle

This project demonstrates a basic Vulkan application where a Sierpiński triangle is rendered to the screen. The main objective of this tutorial is to provide an introduction to Vulkan and its capabilities by drawing a simple fractal shape.

## Known Inefficiencies

The current implementation uses Host-visible memory for storing vertex data. This means the data is accessible by the CPU but not optimized for GPU access. Using Device-visible memory would be more efficient for rendering, as it allows the GPU to access the data directly, reducing latency and improving performance. This is going to be changed in the future!

## Requirements

- A system with Vulkan-compatible hardware
- Vulkan SDK installed
- GLFW3 installed
- glm installed
- C++17-compatible compiler

## Building
```bat
git clone https://github.com/szabo-krisztian/VkTutorial.git

cd VkTutorial/code/finished-projects/sierpinski-triangle

mkdir build

cd build

cmake .. -DCMAKE_PREFIX_PATH="location-of-glfw3;location-of-vulkan" -DGLM_PATH="location-of-glm"

REM mine is: cmake .. -DCMAKE_PREFIX_PATH="C:/Program Files (x86)/GLFW/lib/cmake;C:/VulkanAPI/Lib/cmake" -DGLM_PATH="C:/glm"

cmake --build .
```

## Running

This would be it, however you need to compile the shaders. If you have Vulkan downloaded, you should have a ```glslc.exe``` at ```path-to-vulkan/bin/glslc.exe``` (.exe if you are using Windows). Assuming you are still in the ```build``` folder:

```bat
mkdir spvs

location-of-glslc/glslc.exe ../application/shaders/shader.vert -o spvs/vert.spv

location-of-glslc/glslc.exe ../application/shaders/shader.frag -o spvs/frag.spv

cd Debug

Main.exe
```
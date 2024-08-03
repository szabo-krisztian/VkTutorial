<p align="center">
    <img src="https://github.com/szabo-krisztian/VkTutorial/blob/master/images/icosahedron.gif" alt="icosahedron" />
</p>

# Mesh Division

This project demonstrates a basic Vulkan application that renders an icosahedron divided into smaller tetrahedrons. The main goal of this project is to showcase a simple yet effective algorithm that can turn any convex polyhedron into neatly divided or even randomly noisy tetrahedrons. If you are curious about the algorithm, check my [documentation](https://github.com/szabo-krisztian/VkTutorial/blob/master/documentation/documentation.pdf) file.

## Known Inefficiencies

The current implementation still has some inefficiencies related to memory management. The project setup involves separate blocks for buffers and memory, which could be streamlined in future updates to enhance rendering efficiency.

## Requirements

- A system with Vulkan-compatible hardware
- CMake version 3.10+
- Vulkan SDK installed
- GLFW3 installed
- glm installed
- C++17-compatible compiler

## Building
```bat
git clone https://github.com/szabo-krisztian/VkTutorial.git

cd VkTutorial/code/finished-projects/mesh-division

mkdir build

cd build

cmake .. -DCMAKE_PREFIX_PATH="location-of-glfw3;location-of-vulkan" -DGLM_PATH="location-of-glm"

REM mine is: cmake .. -DCMAKE_PREFIX_PATH="C:/Program Files (x86)/GLFW/lib/cmake;C:/VulkanAPI/Lib/cmake" -DGLM_PATH="C:/glm"

cmake --build .
```

## Running
This would be it, however you need to compile the shaders. If you have Vulkan downloaded, you should have a glslc.exe at path-to-vulkan/bin/glslc.exe (.exe if you are using Windows). Assuming you are still in the build folder:

```bat
mkdir spvs

location-of-glslc/glslc.exe ../application/shaders/shader.vert -o spvs/vert.spv

location-of-glslc/glslc.exe ../application/shaders/shader.frag -o spvs/frag.spv

cd Debug

Main.exe
```

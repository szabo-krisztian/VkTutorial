<p align="center">
    <img src="https://github.com/szabo-krisztian/VkTutorial/blob/master/images/mesh_shooter.gif" alt="shooter" />
</p>

# Rotating Cube

This project demonstrates a basic Vulkan application integrated with the PhysX library for physics simulation. For building and installing PhysX check the [documentation page](https://nvidia-omniverse.github.io/PhysX/physx/5.1.0/docs/BuildingWithPhysX.html). The main goal of this project is to provide a fundamental understanding of Vulkan by showcasing a simple interactive effect where players can shoot convex polyhedrons at a large convex polyhedron with free FPS camera movement. Each time the application runs, the polyhedrons you see are randomly generated and form convex meshes.

## Known Inefficiencies

The current implementation still has some inefficiencies related to memory management. The project setup involves separate blocks for buffers and memories, which could be streamlined in future updates to enhance rendering efficiency.

## Requirements

- A system with Vulkan-compatible hardware
- CMake version 3.10+
- Vulkan SDK installed
- GLFW3 installed
- glm installed
- PhysX installed
- C++17-compatible compiler

## Building
```bat
git clone https://github.com/szabo-krisztian/VkTutorial.git

cd VkTutorial/code/finished-projects/mesh-shooter

mkdir build

cd build

cmake .. -DCMAKE_PREFIX_PATH="location-of-glfw3;location-of-vulkan" -DGLM_PATH="location-of-glm" -DPHYSX_INCLUDE="location-of-physx/include" -DPHYSX_LIB="location-of-physx/bin/version/checked"

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
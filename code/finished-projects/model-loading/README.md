<p align="center">
    <img src="https://github.com/szabo-krisztian/VkTutorial/blob/master/images/bugatti.gif" alt="bugatti" />
</p>

# Model loading

This project demonstrates a basic Vulkan application that renders a 3D model. The 3D model used is a Bugatti mesh, created in Blender by [Bob Kimani](https://free3d.com/3d-model/bugatti-chiron-2017-model-31847.html). It is free to use, and additional information regarding the license can be found in the [application/bugatti/README.txt](https://github.com/szabo-krisztian/VkTutorial/blob/master/code/finished-projects/model-loading/application/bugatti/README.txt) file. The main goal of this project is to showcase how to load a 3D object using the [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader?tab=License-1-ov-file#readme) library and implement [Blinn-Phong](https://en.wikipedia.org/wiki/Blinn–Phong_reflection_model) shading. The mesh is composed of shapes, each with its own material, which contains shading information. During the draw loop, we render the vertices shape by shape, binding the respective material's desciptor set to the command buffer. Note that this example does not include textures.

## Known Inefficiencies

The current implementation still has some inefficiencies related to memory management. The project setup involves separate blocks for buffers and memories, which could be streamlined in future updates to enhance rendering efficiency.

## Requirements

- A system with Vulkan-compatible hardware
- CMake version 3.10+
- Vulkan SDK installed
- GLFW3 installed
- glm installed
- tinyobjloader installed
- C++17-compatible compiler
- mesh installed (place bugatti.obj, bugatti.mtl in the appication/bugatti folder)

## Building
```bat
git clone https://github.com/szabo-krisztian/VkTutorial.git

cd VkTutorial/code/finished-projects/model-loading

mkdir build

cd build

cmake .. -DCMAKE_PREFIX_PATH="location-of-glfw3;location-of-vulkan" -DGLM_PATH="location-of-glm" -DTINYLOADER_PATH="location-of-tinyobjloader"

REM mine is: cmake .. -DCMAKE_PREFIX_PATH="C:/Program Files (x86)/GLFW/lib/cmake/glfw3" -DGLM_PATH="C:/glm" -DTINYLOADER_PATH="C:/tinyobjloader"

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
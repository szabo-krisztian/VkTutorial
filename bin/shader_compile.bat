@echo off

REM Compiling the shaders
SET glslcExe=C:\VulkanAPI\Bin\glslc.exe
SET shadersDirectory="../src/shaders"

%glslcExe% %shadersDirectory%/simple_shader.vert -o %shadersDirectory%/simple_shader.vert.spv
%glslcExe% %shadersDirectory%/simple_shader.frag -o %shadersDirectory%/simple_shader.frag.spv
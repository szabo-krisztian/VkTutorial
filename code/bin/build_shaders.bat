SET shadersFolder=../src/shaders
SET glslcPath=C:\VulkanAPI\Bin\glslc.exe

%glslcPath% %shadersFolder%/shader.vert -o ./spvs/vert.spv
%glslcPath% %shadersFolder%/shader.frag -o ./spvs/frag.spv
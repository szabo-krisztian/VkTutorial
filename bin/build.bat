@echo off

call "C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Auxiliary/Build/vcvars64.bat"

REM Include directories
SET vulkanIncludeDirectory="%VULKAN_SDK%/Include"
SET glfwIncludeDirectory="C:/CppLibraries/glfw-3.4/Include"
SET glmIncludeDirectory="C:/CppLibraries"
SET sourceIncludeDirectory="../src"

REM Include compilation tags
SET extIncludes=/I%vulkanIncludeDirectory%^
                /I%glfwIncludeDirectory%^
                /I%glmIncludeDirectory%^
                /I%sourceIncludeDirectory%

REM Link directories
SET vulkanLinkDirectory="%VULKAN_SDK%/Lib"
SET glfwLinkDirectory="C:/CppLibraries/glfw-3.4/build/src/Release"

REM Link libs
SET vulkanLibs=vulkan-1.lib
SET glfwLibs=glfw3.lib gdi32.lib user32.lib shell32.lib

REM Link compilation tags
SET    extLinks=/link^
                /LIBPATH:%glfwLinkDirectory% %glfwLibs%^
                /LIBPATH:%vulkanLinkDirectory% %vulkanLibs%


SET locSrc=../src/vlk_window.cpp ../src/first_app.cpp ../src/vlk_pipeline.cpp ../src/vlk_device.cpp
SET mainSrc=../src/main.cpp
SET objDir=objs

SET defines=/D DEBUG

echo "Building main..."
echo.
echo.

REM Compile with /MD (use DLL version of CRT)
cl /MD /EHsc /Fo%objDir%\ %mainSrc% %locSrc% %extIncludes% %extLinks%
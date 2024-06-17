@echo off
setlocal enabledelayedexpansion

set baseDir=../src

REM ---------- Include ----------

SET vulkanInclude=/I"%VULKAN_SDK%/Include"
SET glfwInclude=/I"C:/CppLibraries/glfw-3.4/Include"
SET glmInclude=/I"C:/CppLibraries"

set srcIncludes=
for /R "%baseDir%" %%d in (.) do (
    if exist "%%d\*" (
        set "dirPath=%%d"
        set "dirPath=!dirPath:~0,-2!"
        set srcIncludes=!srcIncludes! /I"!dirPath!"
    )
)

SET includes=%vulkanInclude% %glfwInclude% %glmInclude% %srcIncludes%

REM ---------- Linking ----------

SET vulkanLink=/LIBPATH:"%VULKAN_SDK%/Lib" vulkan-1.lib
SET glfwLink=/LIBPATH:"C:/CppLibraries/glfw-3.4/build/src/Release" glfw3.lib gdi32.lib user32.lib shell32.lib

SET externalLinks=/link %vulkanLink% %glfwLink%

set srcLinks=
for /R "%baseDir%\vk-object-implementations" %%d in (*.cpp) do (
    set "srcLinks=!srcLinks! "%%d""
)

REM ---------- Compile ----------

SET mainSrc=../src/main.cpp
SET objDir=objs

REM Enter /D NDEBUG to turn off validation layers
SET defines=

echo "Building main..."
echo.
echo.

REM Compile with /MD (use DLL version of CRT)
call "C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Auxiliary/Build/vcvars64.bat"
cl /std:c++17 /MD /EHsc /Fo%objDir%\ %mainSrc% %srcLinks% %defines% %includes% %externalLinks%

endlocal
call "C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Auxiliary/Build/vcvars64.bat"

SET extIncludes=/I"%VULKAN_SDK%/Include"^
                /I"C:/CppLibraries/glfw-3.4/Include"^
                /I"C:/CppLibraries"^
                /I"../src"

SET    extLinks=/link^
                /LIBPATH:"C:/CppLibraries/glfw-3.4/build/src/Release" glfw3.lib gdi32.lib user32.lib shell32.lib^
                /LIBPATH:"%VULKAN_SDK%/Lib" vulkan-1.lib

SET locSrc=../src/my_instance.cpp ../src/my_device.cpp ../src/my_window.cpp ../src/my_swap_chain.cpp
SET mainSrc=../src/main.cpp
SET objDir=objs

REM Enter /D NDEBUG to turn off validation layers
SET defines=

echo "Building main..."
echo.
echo.

REM Compile with /MD (use DLL version of CRT)
cl /std:c++17 /MD /EHsc /Fo%objDir%\ %mainSrc% %locSrc% %defines% %extIncludes% %extLinks%
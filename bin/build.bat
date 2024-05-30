call "C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Auxiliary/Build/vcvars64.bat"

SET extIncludes=/I"%VULKAN_SDK%/Include"^
                /I"C:/CppLibraries/glfw-3.4/Include"^
                /I"C:/CppLibraries"^
                /I"../src"

SET    extLinks=/link^
                /LIBPATH:"C:/CppLibraries/glfw-3.4/build/src/Release" glfw3.lib gdi32.lib user32.lib shell32.lib^
                /LIBPATH:"%VULKAN_SDK%/Lib" vulkan-1.lib

SET locSrc=../src/vlk_window.cpp ../src/first_app.cpp
SET mainSrc=../src/main.cpp

SET defines=/D DEBUG

echo "Building main..."
echo.
echo.

REM Compile with /MD (use DLL version of CRT)
cl /MD /EHsc %mainSrc% %locSrc% %extIncludes% %extLinks%
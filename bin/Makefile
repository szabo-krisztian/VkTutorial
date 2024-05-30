# Define the compiler and compiler flags
CXX := g++
CXXFLAGS := -I"C:/CppLibraries/glfw-3.4/Include" -I"C:/CppLibraries" -I"$(VULKAN_SDK)/Include"
LDFLAGS := -L"C:/CppLibraries/glfw-3.4/build_static/src" -lglfw3 -lgdi32 -L"C:\VulkanAPI\Bin" -lvulkan-1

# Define the target executable
TARGET := main

# Define the source files
SRCS := ../src/main.cpp

# Default target to build the executable
all: $(TARGET)

# Rule to build the executable
$(TARGET): $(SRCS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS)

# Clean target to remove build artifacts
clean:
	rm -f $(TARGET)

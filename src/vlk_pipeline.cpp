#include "vlk_pipeline.hpp"

#include <fstream>
#include <stdexcept>
#include <iostream>

namespace vlk
{

VlkPipeline::VlkPipeline(const std::string& vertexFilepath, const std::string& fragmentFilepath)
{
    createGraphicsPipeline(vertexFilepath, fragmentFilepath);
}

std::vector<char> VlkPipeline::readFile(const std::string& filepath)
{
    std::ifstream file(filepath, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("failed to open file: " + filepath);
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();
    return buffer;
}

void VlkPipeline::createGraphicsPipeline(const std::string& vertexFilepath, const std::string& fragmentFilepath)
{
    auto vertexCode = readFile(vertexFilepath);
    auto fragmentCode = readFile(fragmentFilepath);

    std::cout << "size of vertexCode: " << vertexCode.size() << std::endl;
    std::cout << "size of fragmentCode: " << fragmentCode.size() << std::endl;
}

} // namespace vlk
#pragma once

#include <string>
#include <vector>

namespace vlk
{

class VlkPipeline
{
public:
    VlkPipeline(const std::string& vertexFilepath, const std::string& fragmentFilepath);

private:
    static std::vector<char> readFile(const std::string& filepath);

    void createGraphicsPipeline(const std::string& vertexFilepath, const std::string& fragmentFilepath);
};

} // namespace vlk
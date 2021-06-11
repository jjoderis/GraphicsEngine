#include "objectLoader.h"

#include <Core/Components/Geometry/geometry.h>
#include <Core/Components/Tag/tag.h>
#include <Core/ECS/registry.h>
#include <Util/fileHandling.h>

namespace filesystem = std::filesystem;

Engine::Math::Vector3 parseVector(const std::string &string)
{
    std::string::size_type endIndex;

    float x = std::stof(string, &endIndex);
    float y = std::stof(string.substr(endIndex), &endIndex);
    float z = std::stof(string.substr(endIndex));

    return Engine::Math::Vector3{x, y, z};
}

void Util::loadOBJFile(Engine::Registry &registry, const filesystem::path &filePath)
{
    std::istringstream stream{Util::readTextFile(filePath.c_str())};

    std::string line;

    unsigned int entity{registry.addEntity()};

    std::shared_ptr<Engine::TagComponent> tag = std::make_shared<Engine::TagComponent>("Unnamed Object");
    registry.addComponent<Engine::TagComponent>(entity, tag);

    // parse all information from the file content
    std::vector<Engine::Math::Vector3> vertices{};
    std::vector<Engine::Math::Vector3> normals{};
    std::vector<unsigned int> faces{};

    while (std::getline(stream, line))
    {
        if (line.at(0) == 'o' && line.at(1) == ' ')
        {
            // load object name
            tag->set(line.substr(2).c_str());
        }
        else if (line.at(0) == 'v' && line.at(1) == ' ')
        {
            // load vertex
            vertices.push_back(parseVector(line.substr(2)));
        }
        else if (line.at(0) == 'v' && line.at(1) == 'n' && line.at(2) == ' ')
        {
            // load normal
            normals.push_back(parseVector(line.substr(3)));
        }
        else if (line.at(0) == 'v' && line.at(1) == 't' && line.at(2) == ' ')
        {
            // load texture coordinate
        }
    }
}
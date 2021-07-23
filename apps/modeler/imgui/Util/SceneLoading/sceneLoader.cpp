#include "sceneLoader.h"
#include "parsing.h"
#include "serialization.h"

#include <Core/ECS/registry.h>
#include <Util/fileHandling.h>
#include <fstream>
#include <iostream>
#include <json.hpp>
#include <map>
#include <string>

using Util::writeTextToFile;

using json = nlohmann::json;

void Engine::Util::loadScene(const std::filesystem::path &path,
                             Engine::Registry &registry,
                             Engine::Util::OpenGLTextureIndex &textureIndex)
{
    std::ifstream i(path);
    json j;
    i >> j;
    SceneUtil::parseScene(registry, j, path, textureIndex);
}

void Engine::Util::saveScene(const std::filesystem::path &path,
                             Engine::Registry &registry,
                             const Engine::Util::OpenGLTextureIndex &textureIndex)
{
    auto fullPath = path;
    fullPath.append("scene.gltf");

    std::stringstream ss;

    ss << std::setw(4) << SceneUtil::serializeScene(registry, path, textureIndex) << '\n';

    writeTextToFile(fullPath.c_str(), ss.str());
}
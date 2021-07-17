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

void Engine::Util::loadScene(const std::filesystem::path &path, Engine::Registry &registry)
{
    std::ifstream i(path);
    json j;
    i >> j;
    SceneUtil::parseScene(registry, j, path);
}

void Engine::Util::saveScene(const std::filesystem::path &path, Engine::Registry &registry)
{
    auto fullPath = path;
    fullPath.append("scene.gltf");

    std::stringstream ss;

    ss << std::setw(4) << SceneUtil::serializeScene(registry, path) << '\n';

    writeTextToFile(fullPath.c_str(), ss.str());
}
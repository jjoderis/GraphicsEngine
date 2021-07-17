#include "parsing.h"

#include <Components/Camera/camera.h>
#include <Components/Geometry/geometry.h>
#include <Components/Hierarchy/hierarchy.h>
#include <Components/Tag/tag.h>
#include <Components/Transform/transform.h>
#include <Core/ECS/registry.h>
#include <map>
#include <memory>
#include <string>

using json = nlohmann::json;

using GeometryMap = std::map<int, std::shared_ptr<Engine::GeometryComponent>>;
int addEntity(Engine::Registry &registry, json &j, json &node, GeometryMap &geometries);
GeometryMap parseGeometries(json &j, const std::filesystem::path &path);

void SceneUtil::parseScene(Engine::Registry &registry, json &j, const std::filesystem::path &path)
{
    GeometryMap geometries = parseGeometries(j, path);

    for (auto &node : j["scenes"][0]["nodes"])
    {
        unsigned int nodeIndex = node.get<unsigned int>();
        addEntity(registry, j, j["nodes"][nodeIndex], geometries);
    }
}

void addTransform(Engine::Registry &registry, json &node, int entityIndex);

int addEntity(Engine::Registry &registry, json &j, json &node, GeometryMap &geometries)
{
    int entityIndex = registry.addEntity();

    if (node.find("name") != node.end())
    {
        registry.createComponent<Engine::TagComponent>(entityIndex, node["name"].get<std::string>());
    }
    else
    {
        registry.createComponent<Engine::TagComponent>(entityIndex, "Entity " + std::to_string(entityIndex));
    }

    if (node.find("children") != node.end())
    {
        for (auto &child : node["children"])
        {
            unsigned int childNodeIndex = child.get<unsigned int>();
            int childIndex = addEntity(registry, j, j["nodes"][childNodeIndex], geometries);
            auto childHierarchy = registry.createComponent<Engine::HierarchyComponent>(childIndex);
            childHierarchy->setParent(entityIndex);
            registry.updated<Engine::HierarchyComponent>(childIndex);
        }
    }

    addTransform(registry, node, entityIndex);

    if (node.find("mesh") != node.end())
    {
        registry.addComponent<Engine::GeometryComponent>(entityIndex, geometries.at(node["mesh"]));
    }

    if (node.find("camera") != node.end())
    {
        auto &cameraNode = j["cameras"][node["camera"].get<int>()];

        auto camera = registry.createComponent<Engine::CameraComponent>(entityIndex, registry);

        if (cameraNode["type"] == "perspective")
        {
            auto &attributes = cameraNode["perspective"];
            camera->setAspect(attributes["aspectRatio"]);
            camera->setFov(attributes["yfov"]);
            camera->setNear(attributes["znear"]);
            camera->setFar(attributes["zfar"]);
            camera->calculateProjection();
        }
        // TODO: implement ortographic
        registry.updated<Engine::CameraComponent>(entityIndex);
        registry.createComponent<Engine::ActiveCameraComponent>(entityIndex);
    }

    return entityIndex;
}

void addTransform(Engine::Registry &registry, json &node, int entityIndex)
{
    bool hasRotation = node.find("rotation") != node.end();
    bool hasTranslation = node.find("translation") != node.end();
    bool hasScale = node.find("scale") != node.end();

    if (hasRotation || hasTranslation || hasScale)
    {
        auto transform = registry.createComponent<Engine::TransformComponent>(entityIndex);
        if (hasRotation)
        {
            transform->getRotation() = {
                node["rotation"][0].get<float>(), node["rotation"][1].get<float>(), node["rotation"][2].get<float>()};
        }
        if (hasTranslation)
        {
            transform->getTranslation() = {node["translation"][0].get<float>(),
                                           node["translation"][1].get<float>(),
                                           node["translation"][2].get<float>()};
        }
        if (hasScale)
        {
            transform->getScaling() = {
                node["scale"][0].get<float>(), node["scale"][1].get<float>(), node["scale"][2].get<float>()};
        }
        transform->update();
        registry.updated<Engine::TransformComponent>(entityIndex);
    }
}

using BufferMap = std::map<int, std::vector<char>>;

std::vector<char> &getBuffer(json &j, BufferMap &buffers, int bufferIndex, std::filesystem::path path)
{
    if (buffers.find(bufferIndex) != buffers.end())
    {
        return buffers.at(bufferIndex);
    }
    else
    {
        auto &bufferNode = j["buffers"][bufferIndex];
        std::vector<char> buffer(bufferNode["byteLength"].get<int>());
        path.remove_filename();
        path.append(bufferNode["uri"].get<std::string>());
        FILE *in;
        in = fopen(path.c_str(), "rb");

        fread(buffer.data(), sizeof(char), buffer.size(), in);

        fclose(in);

        buffers.try_emplace(bufferIndex, std::move(buffer));
        return buffers.at(bufferIndex);
    }
}

template <typename T>
void loadData(json &j, BufferMap &buffers, int accessorIndex, T &target, const std::filesystem::path &path);

template <>
void loadData<std::vector<unsigned int>>(json &j,
                                         BufferMap &buffers,
                                         int accessorIndex,
                                         std::vector<unsigned int> &target,
                                         const std::filesystem::path &path)
{
    auto &accessor = j["accessors"][accessorIndex];
    auto &view = j["bufferViews"][accessor["bufferView"].get<int>()];
    auto &buffer = getBuffer(j, buffers, view["buffer"], path);

    int start = accessor["byteOffset"].get<int>() + view["byteOffset"].get<int>();
    int numElements = accessor["count"];
    unsigned int *data = (unsigned int *)(buffer.data() + start);
    target.reserve(numElements);

    for (int i = 0; i < numElements; ++i)
    {
        target.emplace_back(data[i]);
    }
}

template <>
void loadData<std::vector<Engine::Math::Vector3>>(json &j,
                                                  BufferMap &buffers,
                                                  int accessorIndex,
                                                  std::vector<Engine::Math::Vector3> &target,
                                                  const std::filesystem::path &path)
{
    auto &accessor = j["accessors"][accessorIndex];
    auto &view = j["bufferViews"][accessor["bufferView"].get<int>()];
    auto &buffer = getBuffer(j, buffers, view["buffer"], path);

    int start = accessor["byteOffset"].get<int>() + view["byteOffset"].get<int>();
    int numElements = accessor["count"];
    float *data = (float *)(buffer.data() + start);
    target.reserve(numElements);

    for (int i = 0; i < numElements; ++i)
    {
        Engine::Math::Vector3 v{data[3 * i], data[3 * i + 1], data[3 * i + 2]};
        target.emplace_back(v);
    }
}

template <>
void loadData<std::vector<Engine::Math::Vector2>>(json &j,
                                                  BufferMap &buffers,
                                                  int accessorIndex,
                                                  std::vector<Engine::Math::Vector2> &target,
                                                  const std::filesystem::path &path)
{
    auto &accessor = j["accessors"][accessorIndex];
    auto &view = j["bufferViews"][accessor["bufferView"].get<int>()];
    auto &buffer = getBuffer(j, buffers, view["buffer"], path);

    int start = accessor["byteOffset"].get<int>() + view["byteOffset"].get<int>();
    int numElements = accessor["count"];
    float *data = (float *)(buffer.data() + start);
    target.reserve(numElements);

    for (int i = 0; i < numElements; ++i)
    {
        Engine::Math::Vector2 v{data[2 * i], data[2 * i + 1]};
        target.emplace_back(v);
    }
}

GeometryMap parseGeometries(json &j, const std::filesystem::path &path)
{
    GeometryMap geometries{};

    if (j.find("meshes") != j.end())
    {
        BufferMap buffers{};

        int meshIndex = 0;
        for (auto &mesh : j["meshes"])
        {
            auto geometry = std::make_shared<Engine::GeometryComponent>();
            auto &primitives = mesh["primitives"][0];

            if (primitives.find("indices") != primitives.end())
            {
                loadData(j, buffers, primitives["indices"], geometry->getFaces(), path);
            }

            auto &attributes = primitives["attributes"];

            if (attributes.find("POSITION") != attributes.end())
            {
                loadData(j, buffers, attributes["POSITION"], geometry->getVertices(), path);
            }

            if (attributes.find("NORMAL") != attributes.end())
            {
                loadData(j, buffers, attributes["NORMAL"], geometry->getNormals(), path);
            }

            if (attributes.find("TEXCOORD_0") != attributes.end())
            {
                loadData(j, buffers, attributes["TEXCOORD_0"], geometry->getTexCoords(), path);
            }

            geometries.emplace(meshIndex, geometry);

            ++meshIndex;
        }
    }

    return geometries;
}
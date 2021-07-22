#include "parsing.h"

#include <Components/Camera/camera.h>
#include <Components/Geometry/geometry.h>
#include <Components/Hierarchy/hierarchy.h>
#include <Components/Light/light.h>
#include <Components/Material/material.h>
#include <Components/Render/render.h>
#include <Components/Shader/shader.h>
#include <Components/Tag/tag.h>
#include <Components/Transform/transform.h>
#include <Core/ECS/registry.h>
#include <glad/glad.h>
#include <map>
#include <memory>
#include <string>

using json = nlohmann::json;

using GeometryMap = std::map<json, std::shared_ptr<Engine::GeometryComponent>>;
using MaterialMap = std::map<json, std::shared_ptr<Engine::OpenGLMaterialComponent>>;
using ShaderMap = std::map<std::string, std::shared_ptr<Engine::OpenGLShaderComponent>>;
using MeshData = std::tuple<GeometryMap, MaterialMap, ShaderMap>;
int addEntity(Engine::Registry &registry, json &j, json &node, MeshData &MeshData);
GeometryMap parseGeometries(json &j, const std::filesystem::path &path);
MaterialMap parseMaterials(json &j, const std::filesystem::path &path);
ShaderMap parseShaders(json &j, const std::filesystem::path &path);

void SceneUtil::parseScene(Engine::Registry &registry, json &j, const std::filesystem::path &path)
{
    GeometryMap geometries = parseGeometries(j, path);
    MaterialMap materials = parseMaterials(j, path);
    ShaderMap shaders = parseShaders(j, path);

    MeshData meshData{geometries, materials, shaders};

    for (auto &node : j["scenes"][0]["nodes"])
    {
        unsigned int nodeIndex = node.get<unsigned int>();
        addEntity(registry, j, j["nodes"][nodeIndex], meshData);
    }
}

void addTransform(Engine::Registry &registry, json &node, int entityIndex);

int addEntity(Engine::Registry &registry, json &j, json &node, MeshData &meshData)
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
            int childIndex = addEntity(registry, j, j["nodes"][childNodeIndex], meshData);
            auto childHierarchy = registry.createComponent<Engine::HierarchyComponent>(childIndex);
            childHierarchy->setParent(entityIndex);
            registry.updated<Engine::HierarchyComponent>(childIndex);
        }
    }

    addTransform(registry, node, entityIndex);

    if (node.find("mesh") != node.end())
    {
        auto mesh = j["meshes"].at(node["mesh"].get<int>());

        if (mesh.find("material") != mesh.end())
        {
            auto material = j["materials"].at(mesh["material"].get<int>());

            if (material.find("extras") != material.end())
            {
                json extras = material["extras"];

                if (extras.find("shader") != extras.end())
                {
                    std::string shaderPath = extras["shader"]["path"];
                    registry.addComponent<Engine::OpenGLShaderComponent>(entityIndex,
                                                                         std::get<2>(meshData).at(shaderPath));

                    if (extras["shader"]["active"].get<bool>())
                    {
                        registry.createComponent<Engine::RenderComponent>(entityIndex);
                    }
                }

                extras.erase("shader");

                registry.addComponent<Engine::OpenGLMaterialComponent>(entityIndex, std::get<1>(meshData).at(extras));
            }
        }
        mesh.erase("material");

        registry.addComponent<Engine::GeometryComponent>(entityIndex, std::get<0>(meshData).at(mesh));
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

    if (node.find("extensions") != node.end())
    {
        if (node["extensions"].find("KHR_lights_punctual") != node["extensions"].end())
        {
            int lightIndex = node["extensions"]["KHR_lights_punctual"]["light"].get<int>();
            json &lightNode = j["extensions"]["KHR_lights_punctual"]["lights"][lightIndex];

            if (lightNode["type"] == "point")
            {
                registry.createComponent<Engine::PointLightComponent>(
                    entityIndex,
                    Engine::Math::Vector3{lightNode["color"][0].get<int>(),
                                          lightNode["color"][1].get<int>(),
                                          lightNode["color"][2].get<int>()},
                    Engine::Math::Vector3{0, 0, 0},
                    lightNode["intensity"]);
            }

            if (lightNode["type"] == "spot")
            {
                registry.createComponent<Engine::SpotLightComponent>(
                    entityIndex,
                    Engine::Math::Vector3{lightNode["color"][0].get<int>(),
                                          lightNode["color"][1].get<int>(),
                                          lightNode["color"][2].get<int>()},
                    Engine::Math::Vector3{0, 0, 0},
                    lightNode["intensity"],
                    lightNode["spot"]["outerConeAngle"].get<float>(),
                    lightNode["spot"]["innerConeAngle"].get<float>());
            }

            if (lightNode["type"] == "directional")
            {
                registry.createComponent<Engine::DirectionalLightComponent>(
                    entityIndex,
                    Engine::Math::Vector3{lightNode["color"][0].get<int>(),
                                          lightNode["color"][1].get<int>(),
                                          lightNode["color"][2].get<int>()},
                    Engine::Math::Vector3{0, 0, -1});
            }
        }
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

    int accessorOffset = 0;

    if (accessor.find("byteOffset") != accessor.end())
    {
        accessorOffset = accessor["byteOffset"].get<int>();
    }

    int viewOffset = 0;

    if (view.find("byteOffset") != view.end())
    {
        viewOffset = view["byteOffset"].get<int>();
    }

    int start = accessorOffset + viewOffset;
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

    int accessorOffset = 0;

    if (accessor.find("byteOffset") != accessor.end())
    {
        accessorOffset = accessor["byteOffset"].get<int>();
    }

    int viewOffset = 0;

    if (view.find("byteOffset") != view.end())
    {
        viewOffset = view["byteOffset"].get<int>();
    }

    int start = accessorOffset + viewOffset;
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

    int accessorOffset = 0;

    if (accessor.find("byteOffset") != accessor.end())
    {
        accessorOffset = accessor["byteOffset"].get<int>();
    }

    int viewOffset = 0;

    if (view.find("byteOffset") != view.end())
    {
        viewOffset = view["byteOffset"].get<int>();
    }

    int start = accessorOffset + viewOffset;
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

        for (auto &mesh : j["meshes"])
        {
            json geometryPart = mesh;
            geometryPart.erase("material");

            if (geometries.find(geometryPart) != geometries.end())
            {
                continue;
            }

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

            geometries.emplace(geometryPart, geometry);
        }
    }

    return geometries;
}

std::shared_ptr<Engine::OpenGLMaterialComponent>
parseMaterial(json &materialNode, json &j, const std::filesystem::path &path)
{
    auto material = std::make_shared<Engine::OpenGLMaterialComponent>();

    if (materialNode.find("extras") != materialNode.end())
    {
        if (materialNode["extras"].find("properties") != materialNode["extras"].end())
        {
            auto &properties = material->getMaterialData().second;

            auto &data = material->getData();
            int dataOffset = 0;

            json &propertyNodes = materialNode["extras"]["properties"];

            for (auto &propertyNode : propertyNodes)
            {
                Engine::MaterialUniformData property{};
                std::get<0>(property) = propertyNode["name"];

                json value = propertyNode["value"];

                unsigned int type = GL_NONE;
                int newOffset = dataOffset;

                if (value.is_number_float())
                {
                    type = GL_FLOAT;
                    newOffset = dataOffset + sizeof(float);
                    data.resize(newOffset);
                    *((float *)(data.data() + dataOffset)) = value.get<float>();
                }
                else if (value.is_array())
                {
                    if (value.size() == 3)
                    {
                        type = GL_FLOAT_VEC3;
                        newOffset = dataOffset + 3 * sizeof(float);
                        data.resize(newOffset);
                        float *start = (float *)(data.data() + dataOffset);
                        start[0] = value[0].get<float>();
                        start[1] = value[1].get<float>();
                        start[2] = value[2].get<float>();
                    }
                    else if (value.size() == 4)
                    {
                        type = GL_FLOAT_VEC4;
                        newOffset = dataOffset + 4 * sizeof(float);
                        data.resize(newOffset);
                        float *start = (float *)(data.data() + dataOffset);
                        start[0] = value[0].get<float>();
                        start[1] = value[1].get<float>();
                        start[2] = value[2].get<float>();
                        start[3] = value[3].get<float>();
                    }
                }

                std::get<1>(property) = type;
                std::get<2>(property) = dataOffset;
                dataOffset = newOffset;
                properties.push_back(property);
            }

            material->getMaterialData().first = dataOffset;
        }
    }

    return material;
}

MaterialMap parseMaterials(json &j, const std::filesystem::path &path)
{
    MaterialMap materials{};

    if (j.find("materials") != j.end())
    {
        for (auto &material : j["materials"])
        {
            if (material.find("extras") != material.end())
            {
                json materialPart = material["extras"];
                materialPart.erase("shader");

                if (materials.find(materialPart) != materials.end())
                {
                    continue;
                }

                materials.emplace(materialPart, parseMaterial(material, j, path));
            }
        }
    }

    return materials;
}

ShaderMap parseShaders(json &j, const std::filesystem::path &path)
{
    ShaderMap shaders;

    if (j.find("materials") != j.end())
    {
        for (auto &material : j["materials"])
        {
            if (material.find("extras") != material.end())
            {
                json &extras = material["extras"];

                if (extras.find("shader") != extras.end())
                {
                    auto shaderSubPath = extras["shader"]["path"].get<std::string>();
                    auto shaderPath = path;
                    shaderPath.remove_filename();
                    shaderPath.append(shaderSubPath);
                    auto shaderData = Engine::loadShaders(shaderPath.c_str());
                    auto shader = std::make_shared<Engine::OpenGLShaderComponent>(shaderData);

                    shaders.emplace(shaderSubPath, shader);
                }
            }
        }
    }

    return shaders;
}
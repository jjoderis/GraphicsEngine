#include "parsing.h"

#include <Components/Camera/camera.h>
#include <Components/Geometry/geometry.h>
#include <Components/Hierarchy/hierarchy.h>
#include <Components/Light/light.h>
#include <Components/Material/material.h>
#include <Components/Material/raytracingMaterial.h>
#include <Components/Render/render.h>
#include <Components/Shader/shader.h>
#include <Components/Tag/tag.h>
#include <Components/Texture/texture.h>
#include <Components/Transform/transform.h>
#include <Core/ECS/registry.h>
#include <OpenGL/Util/textureLoader.h>
#include <Util/textureIndex.h>
#include <glad/glad.h>
#include <imgui.h>
#include <map>
#include <memory>
#include <string>

using json = nlohmann::json;

using GeometryMap = std::map<json, std::shared_ptr<Engine::GeometryComponent>>;
using MaterialMap = std::map<json, std::shared_ptr<Engine::OpenGLMaterialComponent>>;
using ShaderMap = std::map<std::string, std::shared_ptr<Engine::OpenGLShaderComponent>>;
using MeshData = std::tuple<GeometryMap, MaterialMap, ShaderMap>;
int addEntity(Engine::Registry &registry,
              const std::filesystem::path &path,
              json &j,
              json &node,
              MeshData &MeshData,
              Engine::Util::OpenGLTextureIndex &textureIndex);
GeometryMap parseGeometries(json &j, const std::filesystem::path &path);
MaterialMap parseMaterials(json &j, const std::filesystem::path &path);
ShaderMap parseShaders(json &j, const std::filesystem::path &path);

void SceneUtil::parseScene(Engine::Registry &registry,
                           json &j,
                           const std::filesystem::path &path,
                           Engine::Util::OpenGLTextureIndex &textureIndex)
{
    Engine::Util::invertTextureOnImportOff();
    GeometryMap geometries = parseGeometries(j, path);
    MaterialMap materials = parseMaterials(j, path);
    ShaderMap shaders = parseShaders(j, path);

    MeshData meshData{geometries, materials, shaders};

    for (auto &node : j["scenes"][0]["nodes"])
    {
        unsigned int nodeIndex = node.get<unsigned int>();
        addEntity(registry, path, j, j["nodes"][nodeIndex], meshData, textureIndex);
    }

    Engine::Util::invertTextureOnImportOn();
}

void addTransform(Engine::Registry &registry, json &node, int entityIndex);

int addEntity(Engine::Registry &registry,
              const std::filesystem::path &path,
              json &j,
              json &node,
              MeshData &meshData,
              Engine::Util::OpenGLTextureIndex &textureIndex)
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

    addTransform(registry, node, entityIndex);

    if (node.find("children") != node.end())
    {
        for (auto &child : node["children"])
        {
            unsigned int childNodeIndex = child.get<unsigned int>();
            int childIndex = addEntity(registry, path, j, j["nodes"][childNodeIndex], meshData, textureIndex);
            auto childHierarchy = registry.createComponent<Engine::HierarchyComponent>(childIndex);
            childHierarchy->setParent(entityIndex);
            registry.updated<Engine::HierarchyComponent>(childIndex);
        }
    }

    if (node.find("mesh") != node.end())
    {
        auto mesh = j["meshes"].at(node["mesh"].get<int>());
        int count = 0;

        for (auto &primitive : mesh["primitives"])
        {
            int primitiveEntity;

            if (mesh["primitives"].size() == 1)
            {
                primitiveEntity = entityIndex;
            }
            else
            {
                std::string name{"Primitive " + std::to_string(count++)};
                primitiveEntity = registry.addEntity();
                registry.createComponent<Engine::TagComponent>(primitiveEntity, name);
                auto hierarchy{registry.createComponent<Engine::HierarchyComponent>(primitiveEntity)};
                hierarchy->setParent(entityIndex);
                registry.updated<Engine::HierarchyComponent>(primitiveEntity);
            }

            if (primitive.find("material") != primitive.end())
            {
                auto material = j["materials"].at(primitive["material"].get<int>());

                if (material.find("pbrMetallicRoughness") != material.end())
                {
                    if (material["pbrMetallicRoughness"].find("baseColorTexture") !=
                        material["pbrMetallicRoughness"].end())
                    {
                        int textureI = material["pbrMetallicRoughness"]["baseColorTexture"]["index"];
                        int imageIndex = j["textures"].at(textureI)["source"];
                        std::string imagePath = j["images"].at(imageIndex)["uri"];
                        auto fullPath = path;
                        fullPath.remove_filename();
                        fullPath.append(imagePath);
                        registry.createComponent<Engine::OpenGLTextureComponent>(primitiveEntity)
                            ->addTexture(textureIndex.needTexture(fullPath, GL_TEXTURE_2D));
                    }

                    if (material["pbrMetallicRoughness"].find("baseColorFactor") !=
                        material["pbrMetallicRoughness"].end())
                    {
                        json &jColor{material["pbrMetallicRoughness"]["baseColorFactor"]};
                        auto color{registry.createComponent<Engine::RaytracingMaterial>(primitiveEntity)};

                        for (int i{0}; i < jColor.size(); ++i)
                        {
                            color->getColor()(i) = jColor[i];
                        }
                    }
                }

                if (material.find("extras") != material.end())
                {
                    json extras = material["extras"];

                    if (extras.find("shader") != extras.end())
                    {
                        std::string shaderPath = extras["shader"]["path"];
                        registry.addComponent<Engine::OpenGLShaderComponent>(primitiveEntity,
                                                                             std::get<2>(meshData).at(shaderPath));

                        if (extras["shader"]["active"].get<bool>())
                        {
                            registry.createComponent<Engine::RenderComponent>(primitiveEntity);
                        }
                    }

                    extras.erase("shader");

                    registry.addComponent<Engine::OpenGLMaterialComponent>(primitiveEntity,
                                                                           std::get<1>(meshData).at(extras));
                }
            }
            primitive.erase("material");

            registry.addComponent<Engine::GeometryComponent>(primitiveEntity, std::get<0>(meshData).at(primitive));
        }
    }

    if (node.find("camera") != node.end())
    {
        auto &cameraNode = j["cameras"][node["camera"].get<int>()];

        auto camera = registry.createComponent<Engine::CameraComponent>(entityIndex, registry);

        if (cameraNode["type"] == "perspective")
        {
            auto &attributes = cameraNode["perspective"];

            auto viewport = ImGui::GetMainViewport();
            float width = viewport->Size.x;
            float height = viewport->Size.y;

            camera->setAspect(width / height);
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
                registry.createComponent<Engine::PointLightComponent>(entityIndex,
                                                                      Engine::Vector3{lightNode["color"][0].get<int>(),
                                                                                      lightNode["color"][1].get<int>(),
                                                                                      lightNode["color"][2].get<int>()},
                                                                      Engine::Vector3{0, 0, 0},
                                                                      lightNode["intensity"]);
            }

            if (lightNode["type"] == "spot")
            {
                registry.createComponent<Engine::SpotLightComponent>(entityIndex,
                                                                     Engine::Vector3{lightNode["color"][0].get<int>(),
                                                                                     lightNode["color"][1].get<int>(),
                                                                                     lightNode["color"][2].get<int>()},
                                                                     Engine::Vector3{0, 0, 0},
                                                                     lightNode["intensity"],
                                                                     lightNode["spot"]["outerConeAngle"].get<float>(),
                                                                     lightNode["spot"]["innerConeAngle"].get<float>());
            }

            if (lightNode["type"] == "directional")
            {
                registry.createComponent<Engine::DirectionalLightComponent>(
                    entityIndex,
                    Engine::Vector3{lightNode["color"][0].get<int>(),
                                    lightNode["color"][1].get<int>(),
                                    lightNode["color"][2].get<int>()},
                    Engine::Vector3{0, 0, -1});
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
            Engine::Quaternion q{node["rotation"][0].get<float>(),
                                 node["rotation"][1].get<float>(),
                                 node["rotation"][2].get<float>(),
                                 node["rotation"][3].get<float>()};
            transform->setRotation(q);
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
void castData(T *data, std::vector<unsigned int> &target, int numElements)
{
    target.reserve(numElements);

    for (int i = 0; i < numElements; ++i)
    {
        target.emplace_back(data[i]);
    }
}

void loadData(json &j,
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

    switch (accessor["componentType"].get<int>())
    {
    case GL_SHORT:
        castData<short>((short *)(buffer.data() + start), target, numElements);
        break;
    case GL_UNSIGNED_SHORT:
        castData<unsigned short>((unsigned short *)(buffer.data() + start), target, numElements);
        break;
    case GL_UNSIGNED_INT:
        castData<unsigned int>((unsigned int *)(buffer.data() + start), target, numElements);
        break;
    default:
        throw "Unsuported type for face";
    }
}

template <typename T, typename = typename std::enable_if<MathLib::is_point_or_vector<T>::value, T>::type>
void loadData(json &j, BufferMap &buffers, int accessorIndex, std::vector<T> &target, const std::filesystem::path &path)
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
        T v{};

        for (int j{0}; j < v.size(); ++j)
        {
            v(j) = data[v.size() * i + j];
        }

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
            for (auto &primitive : mesh["primitives"])
            {
                json geometryPart = primitive;
                geometryPart.erase("material");

                if (geometries.find(geometryPart) != geometries.end())
                {
                    continue;
                }

                auto geometry = std::make_shared<Engine::GeometryComponent>();

                if (primitive.find("indices") != primitive.end())
                {
                    loadData(j, buffers, primitive["indices"], geometry->getFaces(), path);
                }

                auto &attributes = primitive["attributes"];

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

                geometry->calculateBoundingBox();

                geometries.emplace(geometryPart, geometry);
            }
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

    material->update();

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
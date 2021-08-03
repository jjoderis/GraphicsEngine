#include "serialization.h"

#include <Components/Camera/camera.h>
#include <Components/Geometry/geometry.h>
#include <Components/Hierarchy/hierarchy.h>
#include <Components/Light/light.h>
#include <Components/Material/material.h>
#include <Components/Render/render.h>
#include <Components/Shader/shader.h>
#include <Components/Tag/tag.h>
#include <Components/Texture/texture.h>
#include <Components/Transform/transform.h>
#include <Core/ECS/registry.h>
#include <glad/glad.h>
#include <json.hpp>

using json = nlohmann::json;

using GeometryMap = std::map<Engine::GeometryComponent *, json>;
using MaterialMap = std::map<Engine::OpenGLMaterialComponent *, json>;
using ShaderMap = std::map<Engine::OpenGLShaderComponent *, std::string>;
using MeshMap = std::map<json, int>;
using MaterialNodeMap = std::map<json, int>;
using MeshData = std::tuple<GeometryMap, MaterialMap, MeshMap, ShaderMap, MaterialNodeMap>;
using PointLightMap = std::map<Engine::PointLightComponent *, int>;
using SpotLightMap = std::map<Engine::SpotLightComponent *, int>;
using DirectionalLightMap = std::map<Engine::DirectionalLightComponent *, int>;
using LightData = std::tuple<PointLightMap, SpotLightMap, DirectionalLightMap>;
using TextureMap = std::map<std::filesystem::path, int>;
using TextureData = std::tuple<TextureMap>;
int addEntity(unsigned int entity,
              const std::filesystem::path &path,
              Engine::Registry &registry,
              json &j,
              MeshData &meshData,
              LightData &lightData,
              TextureData &textureData);
MeshData addMeshes(Engine::Registry &registry, json &j, const std::filesystem::path &path);

LightData addLights(Engine::Registry &registry, json &j);

json SceneUtil::serializeScene(Engine::Registry &registry,
                               const std::filesystem::path &path,
                               const Engine::Util::OpenGLTextureIndex &textureIndex)
{
    auto &entities{registry.getEntities()};

    json rootNodes = json::array();

    json j;
    j["nodes"] = json::array();
    j["meshes"] = json::array();
    j["buffers"] = json::array();
    j["bufferViews"] = json::array();
    j["accessors"] = json::array();
    j["textures"] = json::array();
    j["images"] = json::array();

    auto meshData = addMeshes(registry, j, path);

    auto lightData = addLights(registry, j);

    auto textureDirectoryPath = path;
    textureDirectoryPath.append("textures");
    std::filesystem::create_directory(textureDirectoryPath);

    TextureData TextureData{};

    for (auto entity : entities)
    {
        auto hierarchy = registry.getComponent<Engine::HierarchyComponent>(entity);

        if (!hierarchy || hierarchy->getParent() < 0)
        {
            // add the entity to the rootNodes list if it has no parent
            int index = addEntity(entity, path, registry, j, meshData, lightData, TextureData);
            rootNodes.push_back(index);
        }
    }

    json scene = json::object();
    scene["nodes"] = rootNodes;
    j["scenes"] = {scene};
    j["asset"] = {{"version", "2.0"}};

    return j;
}

void addTransform(unsigned int entity, Engine::Registry &registry, json &node);

int addEntity(unsigned int entity,
              const std::filesystem::path &path,
              Engine::Registry &registry,
              json &j,
              MeshData &meshData,
              LightData &lightData,
              TextureData &textureData)
{
    auto node = json::object();
    if (auto tag = registry.getComponent<Engine::TagComponent>(entity))
    {
        node["name"] = tag->get();
    }

    if (auto hierarchy = registry.getComponent<Engine::HierarchyComponent>(entity))
    {
        auto &children = hierarchy->getChildren();
        if (children.size())
        {
            json childList = json::array();

            for (auto child : children)
            {
                int index = addEntity(child, path, registry, j, meshData, lightData, textureData);
                childList.push_back(index);
            }

            node["children"] = childList;
        }
    }

    addTransform(entity, registry, node);

    // TODO: this will prevent the export of only a material if there is no geometry
    if (auto geometry = registry.getComponent<Engine::GeometryComponent>(entity))
    {
        GeometryMap &geometryMap = std::get<0>(meshData);

        auto mesh = geometryMap.at(geometry.get());

        if (auto material = registry.getComponent<Engine::OpenGLMaterialComponent>(entity))
        {
            auto &materialMap = std::get<1>(meshData);
            auto materialNode = materialMap.at(material.get());

            if (auto shader = registry.getComponent<Engine::OpenGLShaderComponent>(entity))
            {
                std::string &shaderPath = std::get<3>(meshData).at(shader.get());
                materialNode["extras"]["shader"] = {{"path", shaderPath}, {"active", false}};

                if (registry.hasComponent<Engine::RenderComponent>(entity))
                {
                    materialNode["extras"]["shader"]["active"] = true;
                }
            }

            if (auto texture = registry.getComponent<Engine::OpenGLTextureComponent>(entity))
            {
                if (texture->getNumTextures())
                {
                    materialNode["pbrMetallicRoughness"] = json::object();
                    materialNode["pbrMetallicRoughness"]["baseColorTexture"] = json::object();

                    auto absPath = texture->getTexture(0).getPath();
                    auto &textureMap = std::get<0>(textureData);
                    if (textureMap.find(absPath) != textureMap.end())
                    {
                        materialNode["pbrMetallicRoughness"]["baseColorTexture"]["index"] = textureMap.at(absPath);
                    }
                    else
                    {
                        // TODO: export samplers
                        int index = j["images"].size();
                        textureMap.emplace(absPath, index);
                        std::string texturePath = "textures/" + absPath.filename().string();
                        auto filePath{path};
                        filePath.append(texturePath);
                        j["images"].push_back({{"uri", texturePath}});
                        std::filesystem::copy(absPath, filePath, std::filesystem::copy_options::overwrite_existing);
                        j["textures"].push_back({{"source", index}});
                        materialNode["pbrMetallicRoughness"]["baseColorTexture"]["index"] = index;
                    }
                }
            }

            int materialIndex;

            MaterialNodeMap &nodeMap = std::get<4>(meshData);
            if (nodeMap.find(materialNode) != nodeMap.end())
            {
                materialIndex = nodeMap.at(materialNode);
            }
            else
            {
                materialIndex = j["materials"].size();
                j["materials"].push_back(materialNode);
                nodeMap.emplace(materialNode, materialIndex);
            }

            mesh["material"] = materialIndex;
        }

        int meshIndex;

        MeshMap &meshMap = std::get<2>(meshData);
        if (meshMap.find(mesh) != meshMap.end())
        {
            meshIndex = meshMap.at(mesh);
        }
        else
        {
            meshIndex = j["meshes"].size();
            j["meshes"].push_back(mesh);
            meshMap.emplace(mesh, meshIndex);
        }

        node["mesh"] = meshIndex;
    }

    if (auto camera = registry.getComponent<Engine::CameraComponent>(entity))
    {
        node["camera"] = j["cameras"].size();
        json cameraNode = json::object();

        if (camera->isPerspective())
        {
            cameraNode["type"] = "perspective";

            cameraNode["perspective"] = {{"aspectRatio", camera->getAspect()},
                                         {"yfov", camera->getFov()},
                                         {"zfar", camera->getFar()},
                                         {"znear", camera->getNear()}};
        }
        else
        {
            cameraNode["type"] = "ortographic";

            int xmax = std::max(camera->getRight(), camera->getLeft());
            int xmin = std::min(camera->getRight(), camera->getLeft());
            int ymax = std::max(camera->getTop(), camera->getBottom());
            int ymin = std::min(camera->getTop(), camera->getBottom());

            cameraNode["ortographic"] = {
                {"xmag", xmax - xmin}, {"yfov", ymax - ymin}, {"zfar", camera->getFar()}, {"znear", camera->getNear()}};
        }

        j["cameras"].push_back(cameraNode);
    }

    auto pointLight = registry.getComponent<Engine::PointLightComponent>(entity);
    auto spotLight = registry.getComponent<Engine::SpotLightComponent>(entity);
    auto directionalLight = registry.getComponent<Engine::DirectionalLightComponent>(entity);

    if (pointLight || spotLight || directionalLight)
    {
        node["extensions"] = json::object();

        if (pointLight)
        {
            node["extensions"]["KHR_lights_punctual"] = {{"light", std::get<0>(lightData).at(pointLight.get())}};
        }
        if (spotLight)
        {
            node["extensions"]["KHR_lights_punctual"] = {{"light", std::get<1>(lightData).at(spotLight.get())}};
        }
        if (directionalLight)
        {
            node["extensions"]["KHR_lights_punctual"] = {{"light", std::get<2>(lightData).at(directionalLight.get())}};
        }
    }

    int index = j["nodes"].size();
    j["nodes"].push_back(node);

    return index;
}

void addTransform(unsigned int entity, Engine::Registry &registry, json &node)
{
    if (auto transform = registry.getComponent<Engine::TransformComponent>(entity))
    {
        auto &rotation = transform->getRotation();
        auto &translation = transform->getTranslation();
        auto &scale = transform->getScaling();

        auto &axis = rotation.qv();
        auto &angle = rotation.qw();
        node["rotation"] = {axis.at(0), axis.at(1), axis.at(2), angle};
        node["translation"] = {translation.at(0), translation.at(1), translation.at(2)};
        node["scale"] = {scale.at(0), scale.at(1), scale.at(2)};
    }
}

template <typename T>
void addTypeData(json &accessor);
template <>
void addTypeData<unsigned int>(json &accessor)
{
    accessor["componentType"] = GL_UNSIGNED_SHORT;
    accessor["type"] = "SCALAR";
}
template <>
void addTypeData<float>(json &accessor)
{
    accessor["componentType"] = GL_FLOAT;
    accessor["type"] = "SCALAR";
}
template <>
void addTypeData<std::vector<unsigned int>>(json &accessor)
{
    addTypeData<unsigned int>(accessor);
}
template <>
void addTypeData<std::vector<Engine::Math::Vector2>>(json &accessor)
{
    addTypeData<float>(accessor);
    accessor["type"] = "VEC2";
}
template <>
void addTypeData<std::vector<Engine::Math::Vector3>>(json &accessor)
{
    addTypeData<float>(accessor);
    accessor["type"] = "VEC3";
}
template <>
void addTypeData<std::vector<Engine::Math::Vector4>>(json &accessor)
{
    addTypeData<float>(accessor);
    accessor["type"] = "VEC4";
}

template <typename T>
json createAccessor(int viewIndex, T data)
{
    json accessor = json::object();
    accessor["bufferView"] = viewIndex;
    accessor["byteOffset"] = 0;
    accessor["count"] = data.size();
    addTypeData<T>(accessor);

    return accessor;
}

int finalOffset(int offset, int dataSize)
{
    int remainder = offset % dataSize;

    // we need to pad to the next divisible number
    if (remainder)
    {
        offset = offset - remainder + dataSize;
    }

    return offset;
}

json createBufferView(int bufferIndex, int byteOffset, std::vector<Engine::Math::Vector3> &data)
{
    int dataSize = sizeof(float);
    byteOffset = finalOffset(byteOffset, dataSize);

    json view = json::object();
    view["buffer"] = bufferIndex;
    view["byteOffset"] = byteOffset;
    view["byteLength"] = 3 * dataSize * data.size();
    view["target"] = 34962; // ARRAY_BUFFER

    return view;
}

json createBufferView(int bufferIndex, int byteOffset, std::vector<Engine::Math::Vector2> &data)
{
    int dataSize = sizeof(float);
    byteOffset = finalOffset(byteOffset, dataSize);

    json view = json::object();
    view["buffer"] = bufferIndex;
    view["byteOffset"] = byteOffset;
    view["byteLength"] = 2 * dataSize * data.size();
    view["target"] = 34962; // ARRAY_BUFFER

    return view;
}

json createBufferView(int bufferIndex, int byteOffset, std::vector<unsigned int> &faces)
{
    int dataSize = sizeof(unsigned int);
    byteOffset = finalOffset(byteOffset, dataSize);

    json view = json::object();
    view["buffer"] = bufferIndex;
    view["byteOffset"] = byteOffset;
    view["byteLength"] = dataSize * faces.size();
    view["target"] = 34963; // ELEMENT_ARRAY_BUFFER

    return view;
}

void padFile(FILE *out, int padding)
{

    for (int i = 0; i < padding; ++i)
    {
        fwrite("\0", 1, 1, out);
    }
}

// detects if the platform is bigEndian
// we would need to swap the bytes
// source: https://stackoverflow.com/a/8979034
constexpr bool isBigEndian()
{
    int i = 1;
    return !*((char *)&i);
}

void addToFile(FILE *out, std::vector<Engine::Math::Vector3> &data, int padding)
{
    static bool bigEndian = isBigEndian();

    padFile(out, padding);

    // TODO: consider endianess (gltf is little endian)
    for (auto &vector : data)
    {
        fwrite(vector.raw(), sizeof(float), 3, out);
    }
}

void addToFile(FILE *out, std::vector<Engine::Math::Vector2> &data, int padding)
{
    static bool bigEndian = isBigEndian();

    padFile(out, padding);

    // TODO: consider endianess (gltf is little endian)
    for (auto &vector : data)
    {
        fwrite(vector.raw(), sizeof(float), 2, out);
    }
}

void addToFile(FILE *out, std::vector<unsigned int> &data, int padding)
{
    static bool bigEndian = isBigEndian();

    padFile(out, padding);

    // TODO: consider endianess (gltf is little endian)
    fwrite(data.data(), sizeof(unsigned int), data.size(), out);
}

json addGeometry(Engine::GeometryComponent &geometry, json &j, std::filesystem::path path)
{
    auto &vertices{geometry.getVertices()};

    json mesh = json::object();

    int meshIndex = j["meshes"].size();

    int bufferIndex = j["buffers"].size();

    auto bufferPath = "buffers/buffer" + std::to_string(bufferIndex) + ".bin";
    path.append(bufferPath);
    FILE *out;
    out = fopen(path.c_str(), "wb");

    int byteOffset = 0;
    int actualOffset = 0;

    int geometryViewIndex = j["bufferViews"].size();
    int geometryAccessorIndex = j["accessors"].size();
    mesh["primitives"][0]["attributes"]["POSITION"] = geometryAccessorIndex;
    addToFile(out, vertices, 0);

    j["accessors"].push_back(createAccessor(geometryViewIndex, vertices));
    auto geometryView = createBufferView(bufferIndex, byteOffset, vertices);
    byteOffset = geometryView["byteOffset"].get<int>() + geometryView["byteLength"].get<int>();
    j["bufferViews"].push_back(geometryView);

    auto &normals = geometry.getNormals();
    if (normals.size())
    {
        int normalViewIndex = j["bufferViews"].size();
        int normalAccessorIndex = j["accessors"].size();
        mesh["primitives"][0]["attributes"]["NORMAL"] = normalAccessorIndex;

        j["accessors"].push_back(createAccessor(normalViewIndex, normals));
        auto normalView = createBufferView(bufferIndex, byteOffset, normals);
        actualOffset = normalView["byteOffset"].get<int>();
        addToFile(out, normals, actualOffset - byteOffset);
        byteOffset = actualOffset + normalView["byteLength"].get<int>();
        j["bufferViews"].push_back(normalView);
    }

    auto &texCoords = geometry.getTexCoords();
    if (texCoords.size())
    {
        int texViewIndex = j["bufferViews"].size();
        int texAccessorIndex = j["accessors"].size();
        mesh["primitives"][0]["attributes"]["TEXCOORD_0"] = texAccessorIndex;

        j["accessors"].push_back(createAccessor(texViewIndex, texCoords));
        auto texView = createBufferView(bufferIndex, byteOffset, texCoords);
        actualOffset = texView["byteOffset"].get<int>();
        addToFile(out, texCoords, actualOffset - byteOffset);
        byteOffset = actualOffset + texView["byteLength"].get<int>();
        j["bufferViews"].push_back(texView);
    }

    auto &faces = geometry.getFaces();
    if (faces.size())
    {
        int faceViewIndex = j["bufferViews"].size();
        int faceAccessorIndex = j["accessors"].size();
        mesh["primitives"][0]["indices"] = faceAccessorIndex;

        j["accessors"].push_back(createAccessor(faceViewIndex, faces));
        auto faceView = createBufferView(bufferIndex, byteOffset, faces);
        actualOffset = faceView["byteOffset"].get<int>();
        addToFile(out, faces, actualOffset - byteOffset);
        byteOffset = actualOffset + faceView["byteLength"].get<int>();
        j["bufferViews"].push_back(faceView);
    }

    j["buffers"].push_back({{"uri", bufferPath}, {"byteLength", byteOffset}});

    fclose(out);

    return mesh;
}

json addMaterial(Engine::OpenGLMaterialComponent &material, json &j, std::filesystem::path path)
{
    json materialNode = json::object();
    materialNode["extras"] = json::object();
    materialNode["extras"]["properties"] = json::array();

    auto &properties = material.getMaterialData().second;

    for (auto &property : properties)
    {
        const char *name{std::get<0>(property).c_str()};
        unsigned int type{std::get<1>(property)};
        int offset{std::get<2>(property)};

        float *v;
        switch (type)
        {
        case GL_FLOAT:
            materialNode["extras"]["properties"].push_back(
                {{"name", name}, {"value", *material.getProperty<float>(offset)}});
            break;
        case GL_FLOAT_VEC4:
            v = material.getProperty<float>(offset);
            materialNode["extras"]["properties"].push_back({{"name", name}, {"value", {v[0], v[1], v[2], v[3]}}});
            break;
        case GL_FLOAT_VEC3:
            v = material.getProperty<float>(offset);
            materialNode["extras"]["properties"].push_back({{"name", name}, {"value", {v[0], v[1], v[2]}}});
            break;
        }
    }

    return materialNode;
}

void exportShaders(Engine::OpenGLMaterialComponent &material, std::filesystem::path path) {}

MeshData addMeshes(Engine::Registry &registry, json &j, const std::filesystem::path &path)
{
    GeometryMap geometryMap{};
    MaterialMap materialMap{};
    ShaderMap shaderMap{};

    auto bufferDirectoryPath = path;
    bufferDirectoryPath.append("buffers");
    std::filesystem::create_directory(bufferDirectoryPath);

    auto &geometries = registry.getComponents<Engine::GeometryComponent>();

    for (auto &geometry : geometries)
    {
        geometryMap.emplace(geometry.get(), addGeometry(*geometry, j, path));
    }

    auto &materials = registry.getComponents<Engine::OpenGLMaterialComponent>();

    for (auto &material : materials)
    {
        materialMap.emplace(material.get(), addMaterial(*material, j, path));
    }

    auto shaderDirectoryPath = path;
    shaderDirectoryPath.append("shaders");
    std::filesystem::create_directory(shaderDirectoryPath);

    auto &shaders = registry.getComponents<Engine::OpenGLShaderComponent>();

    int shaderIndex = 0;
    for (auto shader : shaders)
    {
        std::string shaderSubPath{"shaders/shader" + std::to_string(shaderIndex)};
        shaderMap.emplace(shader.get(), shaderSubPath);
        std::filesystem::path shaderPath{path};
        shaderPath.append(shaderSubPath);
        std::filesystem::create_directory(shaderPath);
        Engine::saveShaders(shaderPath, shader->getShaders());

        ++shaderIndex;
    }

    return {geometryMap, materialMap, MeshMap{}, shaderMap, MaterialNodeMap{}};
}

LightData addLights(Engine::Registry &registry, json &j)
{
    PointLightMap pointLights{};
    SpotLightMap spotLights{};
    DirectionalLightMap directionalLights{};

    json lights = json::array();

    for (auto &pointLight : registry.getComponents<Engine::PointLightComponent>())
    {
        json lightNode = json::object();

        auto &color = pointLight->getColor();
        lightNode["color"] = {color.at(0), color.at(1), color.at(2)};
        lightNode["intensity"] = pointLight->getIntensity();
        lightNode["type"] = "point";

        pointLights.emplace(pointLight.get(), lights.size());
        lights.push_back(lightNode);
    }

    for (auto &spotLight : registry.getComponents<Engine::SpotLightComponent>())
    {
        json lightNode = json::object();

        auto &color = spotLight->getColor();
        lightNode["color"] = {color.at(0), color.at(1), color.at(2)};
        lightNode["intensity"] = spotLight->getIntensity();
        lightNode["type"] = "spot";
        lightNode["spot"] = {{"innerConeAngle", spotLight->getPenumbra()}, {"outerConeAngle", spotLight->getCutoff()}};

        spotLights.emplace(spotLight.get(), lights.size());
        lights.push_back(lightNode);
    }

    for (auto &directionalLight : registry.getComponents<Engine::DirectionalLightComponent>())
    {
        json lightNode = json::object();

        auto &color = directionalLight->getColor();
        lightNode["color"] = {color.at(0), color.at(1), color.at(2)};
        lightNode["intensity"] = 1.0;
        lightNode["type"] = "directional";

        directionalLights.emplace(directionalLight.get(), lights.size());
        lights.push_back(lightNode);
    }

    if (lights.size())
    {
        if (j.find("extensions") != j.end())
        {
            j["extensions"] = json::object();
        }
        if (j["extensions"].find("KHR_lights_punctual") != j["extensions"].end())
        {
            j["extensions"]["KHR_lights_punctual"] = json::object();
        }

        j["extensions"]["KHR_lights_punctual"]["lights"] = lights;

        if (j.find("extensionsUsed") != j.end())
        {
            j["extensionsUsed"] = json::array();
        }

        j["extensionsUsed"].push_back("KHR_lights_punctual");
    }

    return {pointLights, spotLights, directionalLights};
}
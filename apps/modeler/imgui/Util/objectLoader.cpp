#include "objectLoader.h"

#include <Core/Components/Geometry/geometry.h>
#include <Core/Components/Hierarchy/hierarchy.h>
#include <Core/Components/Tag/tag.h>
#include <Core/Components/Transform/transform.h>
#include <Core/ECS/registry.h>
#include <OpenGL/Components/Texture/texture.h>
#include <OpenGL/Util/textureIndex.h>
#include <Util/fileHandling.h>
#include <glad/glad.h>
#include <map>
#include <set>

namespace filesystem = std::filesystem;

void loadMaterialLibrary(const filesystem::path &filePath,
                         std::map<std::string, std::set<std::filesystem::path>> &library);

std::shared_ptr<Engine::HierarchyComponent> createEntity(std::string &name,
                                                         unsigned int &currentEntity,
                                                         Engine::Registry &registry,
                                                         std::shared_ptr<Engine::TagComponent> &tag,
                                                         std::shared_ptr<Engine::GeometryComponent> &geometry)
{
    currentEntity = registry.addEntity();
    tag = registry.addComponent<Engine::TagComponent>(currentEntity, std::make_shared<Engine::TagComponent>(name));
    geometry =
        registry.addComponent<Engine::GeometryComponent>(currentEntity, std::make_shared<Engine::GeometryComponent>());
    registry.addComponent<Engine::TransformComponent>(currentEntity, std::make_shared<Engine::TransformComponent>());
    std::shared_ptr<Engine::HierarchyComponent> hierarchy = registry.addComponent<Engine::HierarchyComponent>(
        currentEntity, std::make_shared<Engine::HierarchyComponent>());

    return hierarchy;
}

// based on: https://stackoverflow.com/a/14887071
void Util::loadOBJFile(Engine::Registry &registry,
                       const filesystem::path &filePath,
                       Engine::Util::OpenGLTextureIndex &textureIndex)
{
    std::istringstream stream{Util::readTextFile(filePath.c_str())};

    unsigned int rootEntity{registry.addEntity()};

    std::shared_ptr<Engine::TagComponent> tag = std::make_shared<Engine::TagComponent>("Import Root");
    registry.addComponent<Engine::TagComponent>(rootEntity, tag);
    std::shared_ptr<Engine::GeometryComponent> geometry =
        registry.addComponent<Engine::GeometryComponent>(rootEntity, std::make_shared<Engine::GeometryComponent>());

    // parse all information from the file content
    std::vector<Engine::Math::Vector3> vertices{};
    std::vector<Engine::Math::Vector3> normals{};
    std::vector<Engine::Math::Vector2> texCoords{};

    std::map<std::string, int> refMap{};

    unsigned int currentEntity = rootEntity;
    unsigned int currentGroup = rootEntity;

    std::map<std::string, std::set<std::filesystem::path>> textureLib{};

    while (!stream.eof())
    {
        std::vector<Engine::Math::Vector3> &gVertices{geometry->getVertices()};
        std::vector<Engine::Math::Vector3> &gNormals{geometry->getNormals()};
        std::vector<Engine::Math::Vector2> &gTexCoords{geometry->getTexCoords()};
        std::vector<unsigned int> &gFaces{geometry->getFaces()};

        std::string line;

        while (std::getline(stream, line))
        {
            std::istringstream lineStream{line};
            std::string type;
            lineStream >> type;

            if (type == "v")
            {
                // load vertex
                float x = 0, y = 0, z = 0, w = 1;
                lineStream >> x >> y >> z;
                vertices.push_back(Engine::Math::Vector3{x, y, z} / w);
            }
            else if (type == "vn")
            {
                // load normal
                float i = 0, j = 0, k = 0;
                lineStream >> i >> j >> k;
                normals.push_back(Engine::Math::Vector3{i, j, k});
            }
            else if (type == "vt")
            {
                // load texture coordinate
                float u = 0, v = 0;
                lineStream >> u >> v;
                texCoords.push_back(Engine::Math::Vector2{u, v});
            }
            else if (type == "f")
            {
                std::vector<unsigned int> indices{};

                std::string refStr;
                while (lineStream >> refStr)
                {
                    std::istringstream ref(refStr);
                    std::string vStr, vtStr, vnStr;
                    std::getline(ref, vStr, '/');
                    std::getline(ref, vtStr, '/');
                    std::getline(ref, vnStr, '/');
                    int v = atoi(vStr.c_str());
                    int vt = atoi(vtStr.c_str());
                    int vn = atoi(vnStr.c_str());
                    v = (v >= 0) ? v - 1 : vertices.size() + v;
                    vt = (vt >= 0) ? vt - 1 : texCoords.size() + vt;
                    vn = (vn >= 0) ? vn - 1 : normals.size() + vn;

                    if (!refMap.count(refStr))
                    {
                        refMap.emplace(refStr, gVertices.size());
                        gVertices.push_back(vertices[v]);
                        if (vtStr != "")
                        {
                            gTexCoords.push_back(texCoords[vt]);
                        }
                        // TODO: Find reason for crash when there are no normals on render
                        // if (vnStr != "")
                        // {
                        //     gNormals.push_back(normals[vn]);
                        // }
                        if (vnStr == "")
                        {
                            geometry->calculateNormals();
                        }
                    }
                    indices.push_back(refMap.at(refStr));
                }

                for (int i = 1; i < indices.size() - 1; ++i)
                {
                    gFaces.push_back(indices[0]);
                    gFaces.push_back(indices[i]);
                    gFaces.push_back(indices[i + 1]);
                }
            }
            else if (type == "o")
            {
                // load object name
                std::string name;
                lineStream >> name;

                auto hierarchy = createEntity(name, currentEntity, registry, tag, geometry);
                hierarchy->setParent(rootEntity);
                registry.updated<Engine::HierarchyComponent>(currentEntity);
                break;
            }
            else if (type == "g")
            {
                // load object name
                std::string name;
                lineStream >> name;

                auto hierarchy = createEntity(name, currentEntity, registry, tag, geometry);
                hierarchy->setParent(rootEntity);
                currentGroup = currentEntity;
                registry.updated<Engine::HierarchyComponent>(currentEntity);
                break;
            }
            else if (type == "usemtl")
            {
                // load object name
                std::string name;
                lineStream >> name;

                auto hierarchy = createEntity(name, currentEntity, registry, tag, geometry);
                auto textureComp = registry.addComponent<Engine::OpenGLTextureComponent>(
                    currentEntity, std::make_shared<Engine::OpenGLTextureComponent>());
                for (auto path : textureLib.at(name))
                {
                    textureComp->addTexture(textureIndex.needTexture(path, GL_TEXTURE_2D, textureComp.get()),
                                            GL_TEXTURE_2D);
                }
                if (name == "default")
                {
                    hierarchy->setParent(rootEntity);
                }
                else
                {
                    hierarchy->setParent(currentGroup);
                }
                registry.updated<Engine::HierarchyComponent>(currentEntity);
                break;
            }
            else if (type == "mtllib")
            {
                std::string relativePath;
                lineStream >> relativePath;

                std::filesystem::path libPath = filePath;
                libPath.replace_filename(relativePath);

                loadMaterialLibrary(libPath, textureLib);
            }
        }
    }
}

std::filesystem::path getFilePath(std::istringstream &stream, std::filesystem::path base)
{
    base.remove_filename();
    stream.ignore();

    char delim = '\\';

    std::string foo;
    while (std::getline(stream, foo, delim))
    {
        base.append(foo);
    }

    return base;
}

void loadMaterialLibrary(const filesystem::path &filePath,
                         std::map<std::string, std::set<std::filesystem::path>> &library)
{
    std::istringstream stream{Util::readTextFile(filePath.c_str())};

    std::string line;

    std::string materialName;

    while (std::getline(stream, line))
    {
        std::istringstream lineStream{line};
        std::string type;

        lineStream >> type;

        if (type == "newmtl")
        {
            lineStream >> materialName;

            library.emplace(materialName, std::set<std::filesystem::path>{});
        }
        else if (type == "map_Ka")
        {
            library.at(materialName).emplace(getFilePath(lineStream, filePath));
        }
        else if (type == "map_Kd")
        {
            library.at(materialName).emplace(getFilePath(lineStream, filePath));
        }
    }
}
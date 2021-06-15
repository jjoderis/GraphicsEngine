#include "geometryTracker.h"

#include "../../../../Core/Components/Geometry/geometry.h"
#include "../../../../Core/ECS/registry.h"
#include <glad/glad.h>

Engine::Systems::OpenGLRenderGeometryTracker::OpenGLRenderGeometryTracker(unsigned int entity, Registry &registry)
    : m_registry{registry}
{
    m_updateCallback = m_registry.onUpdate<Engine::GeometryComponent>(
        entity,
        [&](unsigned int updateEntity, std::weak_ptr<GeometryComponent> geometry) { update(geometry.lock().get()); });
    update(registry.getComponent<GeometryComponent>(entity).get());
}

Engine::Systems::OpenGLRenderGeometryTracker::~OpenGLRenderGeometryTracker()
{
    if (m_VAO > 0)
    {
        glDeleteVertexArrays(1, &m_VAO);
    }
    if (m_VBO > 0)
    {
        glDeleteBuffers(1, &m_VBO);
    }
    if (m_EBO > 0)
    {
        glDeleteBuffers(1, &m_EBO);
    }
}

constexpr size_t positionSize{3 * sizeof(float)};
constexpr size_t normalSize{3 * sizeof(float)};
constexpr size_t texCoordSize{2 * sizeof(float)};
constexpr size_t faceSize{sizeof(unsigned int)};

void Engine::Systems::OpenGLRenderGeometryTracker::update(GeometryComponent *geometry)
{
    std::vector<Math::Vector3> &vertices{geometry->getVertices()};
    std::vector<Math::Vector3> &normals{geometry->getNormals()};
    std::vector<Math::Vector2> &texCoords{geometry->getTexCoords()};
    std::vector<unsigned int> &faces{geometry->getFaces()};

    size_t pointSize{positionSize};

    bool useNormals{0};

    if (normals.size())
    {
        useNormals = true;
        pointSize += normalSize;
    }

    bool useTexCoords{0};

    if (texCoords.size())
    {
        useTexCoords = true;
        pointSize += texCoordSize;
    }

    if (m_VAO == 0)
    {
        glGenVertexArrays(1, &m_VAO);
    }
    glBindVertexArray(m_VAO);

    // logic for when number of points or faces changes
    if (m_VBO > 0 && (m_numPoints != vertices.size() || m_useNormals != useNormals || m_useTexCoords != m_useTexCoords))
    {
        glDeleteBuffers(1, &m_VBO);
    }
    if (m_EBO > 0 && m_numFaces != faces.size())
    {
        glDeleteBuffers(1, &m_EBO);
    }

    m_numPoints = vertices.size();
    m_numFaces = faces.size();
    if (m_VBO == 0)
    {
        m_useNormals = useNormals;
        m_useTexCoords = useTexCoords;
        glGenBuffers(1, &m_VBO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        // allocate memory for the geometry buffer
        glBufferData(GL_ARRAY_BUFFER, m_numPoints * pointSize, NULL, GL_DYNAMIC_DRAW);
    }
    if (m_EBO == 0)
    {
        glGenBuffers(1, &m_EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
        // allocate memory for the face buffer
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_numFaces * faceSize, NULL, GL_DYNAMIC_DRAW);
    }
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);

    int attribIndex = 0;
    size_t offset{0};
    // activate positional data
    glVertexAttribPointer(attribIndex, 3, GL_FLOAT, GL_FALSE, pointSize, (void *)0);
    glEnableVertexAttribArray(attribIndex++);
    offset += positionSize;

    if (m_useNormals)
    {
        glVertexAttribPointer(attribIndex, 3, GL_FLOAT, GL_FALSE, pointSize, (void *)offset);
        glEnableVertexAttribArray(attribIndex++);
        offset += normalSize;
    }
    if (m_useTexCoords)
    {
        glVertexAttribPointer(attribIndex, 2, GL_FLOAT, GL_FALSE, pointSize, (void *)offset);
        glEnableVertexAttribArray(attribIndex++);
        // offset += texCoordSize;
    }

    for (int i = 0; i < m_numPoints; ++i)
    {
        glBufferSubData(GL_ARRAY_BUFFER, pointSize * i, positionSize, vertices[i].raw());
        if (m_useNormals)
        {
            glBufferSubData(GL_ARRAY_BUFFER, pointSize * i + positionSize, normalSize, normals[i].raw());
        }
        if (m_useTexCoords)
        {
            glBufferSubData(GL_ARRAY_BUFFER, pointSize * i + offset, texCoordSize, texCoords[i].raw());
        }
    }

    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, faces.size() * faceSize, faces.data());
}

void Engine::Systems::OpenGLRenderGeometryTracker::draw()
{
    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, m_numFaces, GL_UNSIGNED_INT, 0);
}
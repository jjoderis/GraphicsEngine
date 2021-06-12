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
constexpr size_t pointSize{positionSize + normalSize};
constexpr size_t faceSize{sizeof(unsigned int)};

void Engine::Systems::OpenGLRenderGeometryTracker::update(GeometryComponent *geometry)
{
    std::vector<Math::Vector3> &vertices{geometry->getVertices()};
    std::vector<Math::Vector3> &normals{geometry->getNormals()};
    std::vector<unsigned int> &faces{geometry->getFaces()};

    if (m_VAO == 0)
    {
        glGenVertexArrays(1, &m_VAO);
    }
    glBindVertexArray(m_VAO);

    // logic for when number of points or faces changes
    if (m_VBO > 0 && m_numPoints != vertices.size())
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

    // activate positional data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, pointSize, (void *)0);
    glEnableVertexAttribArray(0);
    // activate normal data // TODO: maybe dependant on shader?
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, pointSize, (void *)positionSize);
    glEnableVertexAttribArray(1);

    for (int i = 0; i < m_numPoints; ++i)
    {
        glBufferSubData(GL_ARRAY_BUFFER, pointSize * i, positionSize, vertices[i].raw());
        glBufferSubData(GL_ARRAY_BUFFER, pointSize * i + positionSize, normalSize, normals[i].raw());
    }

    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, faces.size() * faceSize, faces.data());
}

void Engine::Systems::OpenGLRenderGeometryTracker::draw()
{
    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, m_numFaces, GL_UNSIGNED_INT, 0);
}
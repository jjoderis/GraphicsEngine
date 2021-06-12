#include "transformTracker.h"

#include "../../../../Core/Components/Transform/transform.h"
#include "../../../../Core/ECS/registry.h"
#include <glad/glad.h>

Engine::Systems::OpenGLRenderTransformTracker::OpenGLRenderTransformTracker(unsigned int entity, Registry &registry)
    : m_registry{registry}
{
    m_updateCallback = m_registry.onUpdate<Engine::TransformComponent>(
        entity,
        [&](unsigned int updateEntity, std::weak_ptr<TransformComponent> transform)
        { update(transform.lock().get()); });

    update(registry.getComponent<TransformComponent>(entity).get());
}

Engine::Systems::OpenGLRenderTransformTracker::~OpenGLRenderTransformTracker()
{
    if (m_UBO != 0)
    {
        glDeleteBuffers(1, &m_UBO);
    }
}

constexpr size_t matrixSize{16 * sizeof(float)};
constexpr size_t transformSize{2 * matrixSize};

void Engine::Systems::OpenGLRenderTransformTracker::update(TransformComponent *transform)
{
    if (m_UBO == 0)
    {
        glGenBuffers(1, &m_UBO);
        glBindBuffer(GL_UNIFORM_BUFFER, m_UBO);
        glBufferData(GL_UNIFORM_BUFFER, transformSize, NULL, GL_DYNAMIC_DRAW);
    }
    glBindBuffer(GL_UNIFORM_BUFFER, m_UBO);

    glBufferSubData(GL_UNIFORM_BUFFER, 0, matrixSize, transform->getModelMatrix().raw());
    glBufferSubData(GL_UNIFORM_BUFFER, matrixSize, matrixSize, transform->getNormalMatrix().raw());
}

unsigned int Engine::Systems::OpenGLRenderTransformTracker::getBuffer() { return m_UBO; }
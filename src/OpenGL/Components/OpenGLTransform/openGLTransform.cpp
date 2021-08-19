#include "openGLTransform.h"

#include "../../../Core/Components/Transform/transform.h"
#include <glad/glad.h>

constexpr size_t matrixSize{16 * sizeof(float)};
constexpr size_t transformSize{2 * matrixSize};

Engine::OpenGLTransformComponent::OpenGLTransformComponent(TransformComponent *transform)
{
    glGenBuffers(1, &m_UBO);
    glBindBuffer(GL_UNIFORM_BUFFER, m_UBO);
    glBufferData(GL_UNIFORM_BUFFER, transformSize, NULL, GL_DYNAMIC_DRAW);

    update(transform);
}

Engine::OpenGLTransformComponent::~OpenGLTransformComponent() { glDeleteBuffers(1, &m_UBO); }

void Engine::OpenGLTransformComponent::update(TransformComponent *transform)
{
    glBindBuffer(GL_UNIFORM_BUFFER, m_UBO);

    glBufferSubData(GL_UNIFORM_BUFFER, 0, matrixSize, transform->getModelMatrix().raw());
    glBufferSubData(GL_UNIFORM_BUFFER, matrixSize, matrixSize, transform->getNormalMatrix().raw());
}

void Engine::OpenGLTransformComponent::bind(int index) { glBindBufferBase(GL_UNIFORM_BUFFER, index, m_UBO); }
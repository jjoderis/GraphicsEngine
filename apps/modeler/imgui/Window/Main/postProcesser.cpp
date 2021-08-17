#include "postProcesser.h"

#include <Components/Hierarchy/hierarchy.h>
#include <Components/OpenGLGeometry/openGLGeometry.h>
#include <Components/OpenGLTransform/openGLTransform.h>
#include <Components/Render/render.h>
#include <Core/ECS/registry.h>
#include <algorithm>
#include <glad/glad.h>

ModelerUtil::PostProcesser::PostProcesser(Engine::Registry &registry, int &selectedEntity)
    : m_registry{registry}, m_selectedEntity{selectedEntity}, m_cameraTracker{m_activeCameraUBO, registry}
{
    float vertices[]{-1, -1, 0, 0, 0, 1, -1, 0, 1, 0, 1, 1, 0, 1, 1, -1, 1, 0, 0, 1};

    int indices[]{0, 1, 2, 0, 2, 3};

    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    glGenBuffers(1, &m_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &m_EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glUniformBlockBinding(m_selectedProgram.getProgram(), m_selectedProgram.getBlockIndex("Camera"), m_cameraIndex);
    glUniformBlockBinding(
        m_selectedProgram.getProgram(), m_selectedProgram.getBlockIndex("Transform"), m_transformIndex);
    m_isSelectedIndex = m_selectedProgram.getLocation("isSelected");
}

void ModelerUtil::PostProcesser::postProcess(unsigned int renderedScene)
{
    m_selectedFramebuffer.clear();

    // highlight all rendered entities that are part of this entity
    if (m_selectedEntity > -1)
    {
        // get all entities that are part of this entity
        std::vector<unsigned int> renderableGroupedEntities{(unsigned int)m_selectedEntity};

        if (auto hierarchy{m_registry.getComponent<Engine::HierarchyComponent>(m_selectedEntity)})
        {
            hierarchy->getDecendants(renderableGroupedEntities, m_registry);
        }

        for (unsigned int entity : renderableGroupedEntities)
        {
            if (m_registry.hasComponent<Engine::RenderComponent>(entity))
            {
                m_selectedFramebuffer.bind();
                glBindBufferBase(GL_UNIFORM_BUFFER, m_cameraIndex, m_activeCameraUBO);
                m_selectedProgram.use();
                glUniform1i(m_isSelectedIndex, (entity == m_selectedEntity) ? 1 : 0);
                m_registry.getComponent<Engine::OpenGLTransformComponent>(entity)->bind(m_transformIndex);
                m_registry.getComponent<Engine::OpenGLGeometryComponent>(entity)->draw();
                m_selectedFramebuffer.unbind();
                glUseProgram(0);
            }
        }
    }

    m_frameBuffer.clear();
    m_frameBuffer.bind();
    m_program.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderedScene);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_selectedFramebuffer.getTexture());
    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    m_frameBuffer.unbind();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

Engine::OpenGLFramebuffer &ModelerUtil::PostProcesser::getFramebuffer() { return m_frameBuffer; }

void ModelerUtil::PostProcesser::resize(int width, int height) { m_frameBuffer.resize(width, height); }
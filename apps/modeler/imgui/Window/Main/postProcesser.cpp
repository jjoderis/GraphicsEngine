#include "postProcesser.h"

#include <glad/glad.h>

ModelerUtil::PostProcesser::PostProcesser()
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

    m_indexPosition = m_program.getLocation("selectedEntity");
}

void ModelerUtil::PostProcesser::postProcess(unsigned int renderedScene, unsigned int entityTexture, int selectedEntity)
{
    // make sure that selected entity is not matched when no entity is selected
    selectedEntity = (selectedEntity < 0) ? -2 : selectedEntity;
    m_frameBuffer.clear();
    m_frameBuffer.bind();
    m_program.use();
    glUniform1i(m_indexPosition, (int)selectedEntity);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderedScene);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, entityTexture);
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
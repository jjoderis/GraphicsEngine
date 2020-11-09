#include "render.h"

Engine::OpenGLRenderComponent::OpenGLRenderComponent(Engine::Registry &registry) : m_registry{registry} {
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);

    const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "uniform vec4 color;\n"
    "uniform mat4 model;\n"
    "out vec4 fColor;\n"
    "void main()\n"
    "{\n"
    "   fColor = color;"
    "   gl_Position = model * vec4(aPos, 1.0);\n"
    "}\0";

    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION:FAILED\n" << infoLog << '\n';
        exit(EXIT_FAILURE);
    }

    const char* fragmentShaderSource =
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec4 fColor;\n"
    "void main()\n"
    "{\n"
    "    FragColor = fColor;\n"
    "}\n";

    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION:FAILED\n" << infoLog << '\n';
        exit(EXIT_FAILURE);
    }

    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING::FAILED\n" << infoLog << '\n';
        exit(EXIT_FAILURE);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    m_program = shaderProgram;
}

Engine::OpenGLRenderComponent::~OpenGLRenderComponent() {
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
    glDeleteBuffers(1, &m_EBO);
}

void Engine::OpenGLRenderComponent::extendGeometryBuffers(unsigned int entityId, Engine::GeometryComponent* geometry) {
    std::vector<Math::Vector3>& vertices{ geometry->getVertices() };
    size_t geometryVertexSize = 3 * sizeof(float) * vertices.size();

    // create new buffer with space for the current geometries + space for the new geometry
    unsigned int newVBO{0};
    glGenBuffers(1, &newVBO);
    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, newVBO);
    glBufferData(GL_ARRAY_BUFFER, m_vboSize + geometryVertexSize, NULL, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // copy the data from the old buffer into the new one
    if (m_vboSize) {
        glCopyNamedBufferSubData(m_VBO, newVBO, 0, 0, m_vboSize);
    }

    //  TODO: compare with moving of data as a whole block e.g. geometry.getVerticesAsBlock()
    // copy the data of the new geometry into the new buffer
    for (int i = 0; i < vertices.size(); ++i) 
    {   
        glBufferSubData(GL_ARRAY_BUFFER, m_vboSize + 3 * sizeof(float) * i, 3 * sizeof(float), vertices[i].raw());
    }

    // delete the old buffer and update the meta data
    glDeleteBuffers(1, &m_VBO);
    m_VBO = newVBO;

    meta_data &entityData = m_entityData.at(entityId);
    std::get<0>(entityData) = m_vboSize;
    std::get<1>(entityData) = m_vboSize + geometryVertexSize;

    m_vboSize += geometryVertexSize;

    std::vector<unsigned int>& faces{geometry->getFaces()};
    size_t geometryFaceSize{sizeof(unsigned int) * faces.size()};

    // create new index buffer with space for the old and new indices;
    unsigned int newEBO{};
    glGenBuffers(1, &newEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, newEBO);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_eboSize + geometryFaceSize, NULL, GL_DYNAMIC_DRAW);

    // copy the data from the old buffer into the new one
    if (m_eboSize) {
        glCopyNamedBufferSubData(m_EBO, newEBO, 0, 0, m_eboSize);
    }

    // copy the data of the new geometry into the new buffer
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, m_eboSize, faces.size() * sizeof(unsigned int), faces.data());

    glDeleteBuffers(1, &m_EBO);

    m_EBO = newEBO;

    std::get<2>(entityData) = m_eboSize;
    std::get<3>(entityData) = m_eboSize + geometryFaceSize;

    m_eboSize += geometryFaceSize;
    m_numPrimitives = m_eboSize / sizeof(unsigned int);
}

void Engine::OpenGLRenderComponent::extendMaterialBuffers(unsigned int entityId, MaterialComponent* material) {
    m_color = material->getColor();
}

void Engine::OpenGLRenderComponent::extendTransformBuffers(unsigned int entityId, TransformComponent* transform) {
    auto temp = transform->getModelMatrix();
    m_modelMatrix = temp;
}

void Engine::OpenGLRenderComponent::associate(unsigned int entity) {
    // if the entity is already associated with this component do nothing
    if (m_entityData.find(entity) != m_entityData.end()) {
        return;
    }

    m_entityData.emplace(entity, std::tuple<
        unsigned int, // start in VBO
        unsigned int, // end in VBO
        unsigned int, // start in EBO
        unsigned int, // eng in EBO
        std::shared_ptr<std::function<void(unsigned int, GeometryComponent*)>>, // callback for when a GeometryComponent is added to the entity
        std::shared_ptr<std::function<void(unsigned int, GeometryComponent*)>>, // callback for when a GeometryComponent is removed from the entity
        std::shared_ptr<std::function<void(unsigned int, GeometryComponent*)>> // callback for when a GeometryComponent is updated to the entity
    >{
        -1, -1, -1, -1,
        std::shared_ptr<std::function<void(unsigned int, GeometryComponent*)>>{},
        std::shared_ptr<std::function<void(unsigned int, GeometryComponent*)>>{},
        std::shared_ptr<std::function<void(unsigned int, GeometryComponent*)>>{} 
    });

    // entity has a geometry component => setup buffers to use geometry data of that component
    GeometryComponent* geometry = m_registry.getComponent<Engine::GeometryComponent>(entity);
    MaterialComponent* material = m_registry.getComponent<Engine::MaterialComponent>(entity);
    TransformComponent* transform = m_registry.getComponent<TransformComponent>(entity);
    if (geometry && material) {
        extendMaterialBuffers(entity, material);
        extendTransformBuffers(entity, transform);
        extendGeometryBuffers(entity, geometry);
        // TODO: set callback on component to update the buffers when the geometry is updated
    } else {
        // TODO: setup callback that calls the insert function as soon as a geometry becomes available
    }

    // TODO: setup of material
    // TODO: set callback on entity to dissassociate on removal of components
}

void Engine::OpenGLRenderComponent::dissassociate(unsigned int entity) {
    // TODO: remove components from buffers
}

void Engine::OpenGLRenderComponent::updateVertex(unsigned int entity, int vertexIndex, Math::Vector3& newVertexData) {
    if (m_entityData.find(entity) != m_entityData.end()) {
        meta_data &entityData = m_entityData.at(entity);

        glBindVertexArray(m_VAO);
        glBufferSubData(GL_ARRAY_BUFFER, std::get<0>(entityData) + 3 * sizeof(float) * vertexIndex, 3 * sizeof(float), newVertexData.raw());
    }
}

void Engine::OpenGLRenderComponent::updateFace(unsigned int entity, int faceIndex, const unsigned int* newFace) {
    if (m_entityData.find(entity) != m_entityData.end()) {
        meta_data &entityData = m_entityData.at(entity);

        glBindVertexArray(m_VAO);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, std::get<2>(entityData) + 3 * sizeof(unsigned int) * faceIndex, 3 * sizeof(unsigned int), newFace);
    }
}

void Engine::OpenGLRenderComponent::updateColor(unsigned int entity, Math::Vector4& newColor) {
    m_color = newColor;
}

void Engine::OpenGLRenderComponent::updateTransform(unsigned int entity, Math::Matrix4& modelMatrix) {
    m_modelMatrix = modelMatrix;
}

void Engine::OpenGLRenderComponent::render() {
    glUseProgram(m_program);

    glBindVertexArray(m_VAO);

    glUniform4fv(glGetUniformLocation(m_program, "color"), 1, m_color.raw());
    glUniformMatrix4fv(glGetUniformLocation(m_program, "model"), 1, GL_FALSE, m_modelMatrix.raw());

    glDrawElements(GL_TRIANGLES, m_numPrimitives, GL_UNSIGNED_INT, 0);
}
#include <gtest/gtest.h>
#include <Components/Render/render.h>
#include <GLFW/glfw3.h>

const char* vertexShader =
"#version 330 core\n"
"layout (location = 0) in vec3 vPosition;\n"

"void main() {\n"
"    gl_Position = vec4(vPosition, 1.0);\n"
"}\n";

const char* fragmentShader = 
"#version 330 core\n"

"out vec4 FragColor;\n"
"void main() {\n"
"    FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
"}\n";

using namespace Engine;
class RenderTest : public ::testing::Test
{
public:
    RenderTest() {
        TransformComponent transform{};

        entity1 = m_registry.addEntity(); // entityId = 0

        geometry1 = m_registry.addComponent<Engine::GeometryComponent>(entity1, std::make_shared<GeometryComponent>(
            std::initializer_list<Math::Vector3>{
                Engine::Math::Vector3{-1.0, 0.0, 0.0 },
                Engine::Math::Vector3{ 1.0, 0.0, 0.0 },
                Engine::Math::Vector3{ 0.0, 1.0, 0.0 },
            },
            std::initializer_list<unsigned int>{
                0, 1, 2
            }
        ));
        geometry1->calculateNormals();

        entity2 = m_registry.addEntity(); // entityId = 1
        m_registry.addComponent<Engine::GeometryComponent>(entity2, geometry1);
        material1 = m_registry.addComponent<Engine::MaterialComponent>(entity2, std::make_shared<Engine::MaterialComponent>( 1.0, 2.0, 3.0, 4.0 ));

        entity3 = m_registry.addEntity(); // entityId = 2
        m_registry.addComponent<Engine::GeometryComponent>(entity3, geometry1);
        transform1 = m_registry.addComponent<Engine::TransformComponent>(entity3, std::make_shared<Engine::TransformComponent>());        

        entity4 = m_registry.addEntity(); // entityId = 3
    }
protected :
    Registry m_registry{};
    std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<OpenGLRenderComponent>)>> onRenderComponent;
    unsigned int entity1;
    unsigned int entity2;
    unsigned int entity3;
    unsigned int entity4;
    std::shared_ptr<GeometryComponent> geometry1;
    std::shared_ptr<MaterialComponent> material1;
    std::shared_ptr<TransformComponent> transform1;

    std::vector<float> getVertexBufferContent(const std::shared_ptr<OpenGLRenderComponent>& render) {
        int bufferSize{};
        glBindBuffer(GL_ARRAY_BUFFER, render->m_VBO);
        glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);
        std::vector<float> buffer(bufferSize / sizeof(float), 0.0f);
        glGetBufferSubData(GL_ARRAY_BUFFER, 0, bufferSize, buffer.data());
        return buffer;
    }

    std::vector<unsigned int> getIndexBufferContent(const std::shared_ptr<OpenGLRenderComponent>& render) {
        int bufferSize{};
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render->m_EBO);
        glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);
        std::vector<unsigned int> buffer(bufferSize / sizeof(unsigned int), 0u);
        glGetBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, bufferSize, buffer.data());
        return buffer;
    }

    std::vector<float> getMaterialBufferContent(const std::shared_ptr<OpenGLRenderComponent>& render) {
        int bufferSize{};
        glBindBuffer(GL_UNIFORM_BUFFER, render->m_materialUBO);
        glGetBufferParameteriv(GL_UNIFORM_BUFFER, GL_BUFFER_SIZE, &bufferSize);
        std::vector<float> buffer(bufferSize / sizeof(float), 0.0f);
        glGetBufferSubData(GL_UNIFORM_BUFFER, 0, bufferSize, buffer.data());
        return buffer;
    }

    std::vector<float> getTransformBufferContent(const std::shared_ptr<OpenGLRenderComponent>& render) {
        int bufferSize{};
        glBindBuffer(GL_UNIFORM_BUFFER, render->m_transformUBO);
        glGetBufferParameteriv(GL_UNIFORM_BUFFER, GL_BUFFER_SIZE, &bufferSize);
        std::vector<float> buffer(bufferSize / sizeof(float), 0.0f);
        glGetBufferSubData(GL_UNIFORM_BUFFER, 0, bufferSize, buffer.data());
        return buffer;
    }
};

TEST_F(RenderTest, init_opengl) {
    // completetly stupid but no idea how to avoid this
    // TODO: find a way to avoid this (at least the window)
    if(!glfwInit()) {
        std::cout << "TEST\n";
        FAIL();
    }
    GLFWwindow* m_window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
    glfwMakeContextCurrent(m_window);
    if(!gladLoadGL()) {
        FAIL();
    }
}

TEST_F(RenderTest, initial_state) {
    std::shared_ptr<OpenGLRenderComponent> renderComponent
        = m_registry.addComponent<Engine::OpenGLRenderComponent>(entity4, std::make_shared<Engine::OpenGLRenderComponent>(
            m_registry,
            std::initializer_list<OpenGLShader>{
                OpenGLShader{GL_VERTEX_SHADER, vertexShader},
                OpenGLShader{GL_FRAGMENT_SHADER, fragmentShader}
            }
        ));

    // expect there to be no vertices and indices in the respective buffers
    std::vector<float> expectedVertices{};
    EXPECT_EQ(getVertexBufferContent(renderComponent), expectedVertices);

    std::vector<unsigned int> expectedIndices{};
    EXPECT_EQ(getIndexBufferContent(renderComponent), expectedIndices);

    // expect there to be a default material
    std::vector<float> expectedColors{
        1.0, 0.01, 0.6, 1.0, // Default material
    };
    EXPECT_EQ(getMaterialBufferContent(renderComponent), expectedColors);

    // expect there to be a default transform
    std::vector<float> expectedTransforms{
        1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, // Default modelMatrix
        1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, // Default normalMatrix
    };
    EXPECT_EQ(getTransformBufferContent(renderComponent), expectedTransforms);
}

TEST_F(RenderTest, writes_geometry_to_buffer_if_one_exists) {
    std::shared_ptr<OpenGLRenderComponent> renderComponent
        = m_registry.addComponent<Engine::OpenGLRenderComponent>(entity1, std::make_shared<Engine::OpenGLRenderComponent>(
            m_registry,
            std::initializer_list<OpenGLShader>{
                OpenGLShader{GL_VERTEX_SHADER, vertexShader},
                OpenGLShader{GL_FRAGMENT_SHADER, fragmentShader}
            }
        ));

    std::vector<float> vertexBuffer{ getVertexBufferContent(renderComponent) };
    std::vector<float> expected{
       -1, 0, 0, 0, 0, 1, 0, 0, // vertex 1 position and normal and material index and transform index 
        1, 0, 0, 0, 0, 1, 0, 0, // vertex 2 position and normal and material index and transform index 
        0, 1, 0, 0, 0, 1, 0, 0  // vertex 3 position and normal and material index and transform index 
    };

    EXPECT_EQ(vertexBuffer, expected);

    std::vector<unsigned int> indexBuffer{ getIndexBufferContent(renderComponent) };
    std::vector<unsigned int> expectedIndexBuffer{ 0, 1, 2 };

    EXPECT_EQ(indexBuffer, expectedIndexBuffer);
}

TEST_F(RenderTest, writes_material_to_buffer_and_sets_index) {
    std::shared_ptr<OpenGLRenderComponent> renderComponent
        = m_registry.addComponent<Engine::OpenGLRenderComponent>(entity2, std::make_shared<Engine::OpenGLRenderComponent>(
            m_registry,
            std::initializer_list<OpenGLShader>{
                OpenGLShader{GL_VERTEX_SHADER, vertexShader},
                OpenGLShader{GL_FRAGMENT_SHADER, fragmentShader}
            }
        ));

    std::vector<float> vertexBuffer{ getVertexBufferContent(renderComponent) };
    std::vector<float> expected{
       -1, 0, 0, 0, 0, 1, 1, 0, // vertex 1 position and normal and material index and transform index 
        1, 0, 0, 0, 0, 1, 1, 0, // vertex 2 position and normal and material index and transform index 
        0, 1, 0, 0, 0, 1, 1, 0  // vertex 3 position and normal and material index and transform index 
    };

    EXPECT_EQ(vertexBuffer, expected);

    std::vector<float> materialBuffer{ getMaterialBufferContent(renderComponent) };
    std::vector<float> expectedMaterialBuffer{
        1.0, 0.01, 0.6, 1.0, // Default material
        1.0, 2.0, 3.0, 4.0   // material1
    };

    EXPECT_EQ(materialBuffer, expectedMaterialBuffer); // float equality; maybe not the best idea
}

TEST_F(RenderTest, writes_material_to_buffer_and_sets_index_after_add) {
    std::shared_ptr<OpenGLRenderComponent> renderComponent
        = m_registry.addComponent<Engine::OpenGLRenderComponent>(entity1, std::make_shared<Engine::OpenGLRenderComponent>(
            m_registry,
            std::initializer_list<OpenGLShader>{
                OpenGLShader{GL_VERTEX_SHADER, vertexShader},
                OpenGLShader{GL_FRAGMENT_SHADER, fragmentShader}
            }
        ));

    m_registry.addComponent<Engine::MaterialComponent>(entity1, material1);

    std::vector<float> vertexBuffer{ getVertexBufferContent(renderComponent) };
    std::vector<float> expected{
       -1, 0, 0, 0, 0, 1, 1, 0, // vertex 1 position and normal and material index and transform index 
        1, 0, 0, 0, 0, 1, 1, 0, // vertex 2 position and normal and material index and transform index 
        0, 1, 0, 0, 0, 1, 1, 0  // vertex 3 position and normal and material index and transform index 
    };

    EXPECT_EQ(vertexBuffer, expected);

    std::vector<float> materialBuffer{ getMaterialBufferContent(renderComponent) };
    std::vector<float> expectedMaterialBuffer{
        1.0, 0.01, 0.6, 1.0, // Default material
        1.0, 2.0, 3.0, 4.0   // material1
    };

    EXPECT_EQ(materialBuffer, expectedMaterialBuffer); // float equality; maybe not the best idea
}

TEST_F(RenderTest, removes_material_from_buffer_and_resets_indices_on_remove) {
    std::shared_ptr<OpenGLRenderComponent> renderComponent
        = m_registry.addComponent<Engine::OpenGLRenderComponent>(entity2, std::make_shared<Engine::OpenGLRenderComponent>(
            m_registry,
            std::initializer_list<OpenGLShader>{
                OpenGLShader{GL_VERTEX_SHADER, vertexShader},
                OpenGLShader{GL_FRAGMENT_SHADER, fragmentShader}
            }
        ));

    m_registry.removeComponent<Engine::MaterialComponent>(entity2);

    std::vector<float> vertexBuffer{ getVertexBufferContent(renderComponent) };
    std::vector<float> expected{
       -1, 0, 0, 0, 0, 1, 0, 0, // vertex 1 position and normal and material index and transform index 
        1, 0, 0, 0, 0, 1, 0, 0, // vertex 2 position and normal and material index and transform index 
        0, 1, 0, 0, 0, 1, 0, 0  // vertex 3 position and normal and material index and transform index 
    };

    EXPECT_EQ(vertexBuffer, expected);

    std::vector<float> materialBuffer{ getMaterialBufferContent(renderComponent) };
    std::vector<float> expectedMaterialBuffer{
        1.0, 0.01, 0.6, 1.0, // Default material
    };

    EXPECT_EQ(materialBuffer, expectedMaterialBuffer); // float equality; maybe not the best idea
}

TEST_F(RenderTest, writes_transform_to_buffer_and_sets_index) {
    std::shared_ptr<OpenGLRenderComponent> renderComponent
        = m_registry.addComponent<Engine::OpenGLRenderComponent>(entity3, std::make_shared<Engine::OpenGLRenderComponent>(
            m_registry,
            std::initializer_list<OpenGLShader>{
                OpenGLShader{GL_VERTEX_SHADER, vertexShader},
                OpenGLShader{GL_FRAGMENT_SHADER, fragmentShader}
            }
        ));

    std::vector<float> vertexBuffer{ getVertexBufferContent(renderComponent) };
    std::vector<float> expected{
       -1, 0, 0, 0, 0, 1, 0, 1, // vertex 1 position and normal and material index and transform index 
        1, 0, 0, 0, 0, 1, 0, 1, // vertex 2 position and normal and material index and transform index 
        0, 1, 0, 0, 0, 1, 0, 1  // vertex 3 position and normal and material index and transform index 
    };

    EXPECT_EQ(vertexBuffer, expected);

    std::vector<float> transformBuffer{ getTransformBufferContent(renderComponent) };
    std::vector<float> expectedTransformBuffer{
        1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, // Default modelMatrix
        1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, // Default normalMatrix
        1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0,  // transform1 modelMatrix
        1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0  // transform1 normalMatrix
    };

    EXPECT_EQ(transformBuffer, expectedTransformBuffer); // float equality; maybe not the best idea
}

TEST_F(RenderTest, writes_transform_to_buffer_and_sets_index_after_add) {
    std::shared_ptr<OpenGLRenderComponent> renderComponent
        = m_registry.addComponent<Engine::OpenGLRenderComponent>(entity1, std::make_shared<Engine::OpenGLRenderComponent>(
            m_registry,
            std::initializer_list<OpenGLShader>{
                OpenGLShader{GL_VERTEX_SHADER, vertexShader},
                OpenGLShader{GL_FRAGMENT_SHADER, fragmentShader}
            }
        ));

    m_registry.addComponent<Engine::TransformComponent>(entity1, transform1);

    std::vector<float> vertexBuffer{ getVertexBufferContent(renderComponent) };
    std::vector<float> expected{
       -1, 0, 0, 0, 0, 1, 0, 1, // vertex 1 position and normal and material index and transform index 
        1, 0, 0, 0, 0, 1, 0, 1, // vertex 2 position and normal and material index and transform index 
        0, 1, 0, 0, 0, 1, 0, 1  // vertex 3 position and normal and material index and transform index  
    };

    EXPECT_EQ(vertexBuffer, expected);

    std::vector<float> transformBuffer{ getTransformBufferContent(renderComponent) };
    std::vector<float> expectedTransformBuffer{
        1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, // Default modelMatrix
        1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, // Default normalMatrix
        1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0,  // transform1 modelMatrix
        1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0  // transform1 normalMatrix
    };

    EXPECT_EQ(transformBuffer, expectedTransformBuffer); // float equality; maybe not the best idea
}

TEST_F(RenderTest, removes_transform_from_buffer_and_resets_indices_on_remove) {
    std::shared_ptr<OpenGLRenderComponent> renderComponent
        = m_registry.addComponent<Engine::OpenGLRenderComponent>(entity3, std::make_shared<Engine::OpenGLRenderComponent>(
            m_registry,
            std::initializer_list<OpenGLShader>{
                OpenGLShader{GL_VERTEX_SHADER, vertexShader},
                OpenGLShader{GL_FRAGMENT_SHADER, fragmentShader}
            }
        ));

    m_registry.removeComponent<Engine::TransformComponent>(entity3);

    std::vector<float> vertexBuffer{ getVertexBufferContent(renderComponent) };
    std::vector<float> expected{
       -1, 0, 0, 0, 0, 1, 0, 0, // vertex 1 position and normal and material index and transform index 
        1, 0, 0, 0, 0, 1, 0, 0, // vertex 2 position and normal and material index and transform index 
        0, 1, 0, 0, 0, 1, 0, 0  // vertex 3 position and normal and material index and transform index 
    };

    EXPECT_EQ(vertexBuffer, expected);

    std::vector<float> transformBuffer{ getTransformBufferContent(renderComponent) };
    std::vector<float> expectedTransformBuffer{
        1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, // Default modelMatrix
        1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, // Default normalMatrix
    };

    EXPECT_EQ(transformBuffer, expectedTransformBuffer); // float equality; maybe not the best idea
}

TEST_F(RenderTest, awaits_geometry_before_initializing_entity) {
    std::shared_ptr<OpenGLRenderComponent> renderComponent
        = m_registry.addComponent<Engine::OpenGLRenderComponent>(entity4, std::make_shared<Engine::OpenGLRenderComponent>(
            m_registry,
            std::initializer_list<OpenGLShader>{
                OpenGLShader{GL_VERTEX_SHADER, vertexShader},
                OpenGLShader{GL_FRAGMENT_SHADER, fragmentShader}
            }
        ));

    m_registry.removeComponent<Engine::TransformComponent>(entity3);

    std::vector<float> vertexBuffer{ getVertexBufferContent(renderComponent) };
    std::vector<float> expected{};

    EXPECT_EQ(vertexBuffer, expected);

    std::vector<unsigned int> indexBuffer{ getIndexBufferContent(renderComponent) };
    std::vector<unsigned int> expectedIndexBuffer{};

    EXPECT_EQ(indexBuffer, expectedIndexBuffer);

    m_registry.addComponent<Engine::GeometryComponent>(entity4, geometry1);

    vertexBuffer = getVertexBufferContent(renderComponent);
    expected = {
       -1, 0, 0, 0, 0, 1, 0, 0, // vertex 1 position and normal and material index and transform index 
        1, 0, 0, 0, 0, 1, 0, 0, // vertex 2 position and normal and material index and transform index 
        0, 1, 0, 0, 0, 1, 0, 0  // vertex 3 position and normal and material index and transform index 
    };

    indexBuffer = getIndexBufferContent(renderComponent);
    expectedIndexBuffer = { 0, 1, 2 };

    EXPECT_EQ(indexBuffer, expectedIndexBuffer);
}

TEST_F(RenderTest, resizes_buffer_on_additional_vertices) {
    std::shared_ptr<OpenGLRenderComponent> renderComponent
        = m_registry.addComponent<Engine::OpenGLRenderComponent>(entity2, std::make_shared<Engine::OpenGLRenderComponent>(
            m_registry,
            std::initializer_list<OpenGLShader>{
                OpenGLShader{GL_VERTEX_SHADER, vertexShader},
                OpenGLShader{GL_FRAGMENT_SHADER, fragmentShader}
            }
        ));

    geometry1->addVertex(Engine::Math::Vector3{ 2.0f, 4.0f, 6.0f });
    geometry1->getNormals().emplace_back(Engine::Math::Vector3{-1.0, 0.0, 0.0});
    m_registry.updated<Engine::GeometryComponent>(entity2);

    std::vector<float> expectedVertices{
       -1, 0, 0, 0, 0, 1, 1, 0, // vertex 1 position and normal and material index and transform index 
        1, 0, 0, 0, 0, 1, 1, 0, // vertex 2 position and normal and material index and transform index 
        0, 1, 0, 0, 0, 1, 1, 0,  // vertex 3 position and normal and material index and transform index 
        2, 4, 6,-1, 0, 0, 1, 0  // new vertex
    };

    EXPECT_EQ(getVertexBufferContent(renderComponent), expectedVertices);
}




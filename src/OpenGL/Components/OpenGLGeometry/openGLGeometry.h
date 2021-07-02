#ifndef ENGINE_OPENGL_COMPONENTS_GEOMETRY
#define ENGINE_OPENGL_COMPONENTS_GEOMETRY

/**
 *
 * This class is used to make our geometry data available to our OpenGL Renderer
 *
 * It should be used in a way that for each renderer entity its GeometryComponent has an associated
 *OpenGLGeometryComponent.
 *
 * Changes to the GeometryComponent should be streamed into this Component. Which kind of data is used e.g. normals,
 *texCoords should be deduced from the ShaderComponent of the associated entity
 *
 * (Possible problem!: if two entities share a geometry but have different shaders that require different information
 * -> we might need two OpenGLGeometryComponents, one for each entity)
 *
 **/

namespace Engine
{

class GeometryComponent;

class OpenGLGeometryComponent
{
public:
    OpenGLGeometryComponent() = delete;
    // initialize buffers
    OpenGLGeometryComponent(GeometryComponent *geometry);
    ~OpenGLGeometryComponent();

    // update buffers on geometry changes
    void update(GeometryComponent *geometry);
    // calls the OpenGL draw function with our geometry data
    void draw();

    void drawPoints(bool on);
    bool drawingPoints();

private:
    // buffers for OpenGL Object Data
    unsigned int m_VAO{0};
    unsigned int m_VBO{0};
    unsigned int m_EBO{0};
    // data used for rendering
    int m_numPoints{0};
    int m_numFaces{0};
    // defines which geometry data is currently used
    bool m_useNormals{0};
    bool m_useTexCoords{0};

    bool m_drawPoints{0};
};

} // namespace Engine

#endif
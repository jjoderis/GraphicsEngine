#ifndef ENGINE_OPENGL_COMPONENTS_TRANSFORM
#define ENGINE_OPENGL_COMPONENTS_TRANSFORM

namespace Engine
{
class TransformComponent;

class OpenGLTransformComponent
{
public:
    OpenGLTransformComponent(TransformComponent *transform);
    ~OpenGLTransformComponent();

    void update(TransformComponent *transform);
    void bind();

private:
    unsigned int m_UBO{0};
};

} // namespace Engine

#endif
# engine core library
set(CORE_HEADERS
    Util/fileHandling.h
    Core/ECS/componentTable.h
    Core/ECS/registry.h
    Core/ECS/util.h
    Core/Math/math.h
    Core/Components/Tag/tag.h
    Core/Components/Geometry/geometry.h
    Core/Components/Transform/transform.h
    Core/Components/Camera/camera.h
    Core/Components/Light/light.h
    Core/Components/Hierarchy/hierarchy.h
    Core/Components/Render/render.h
    Core/Systems/HierarchyTracker/hierarchyTracker.h
)

set(CORE_SOURCES
    Util/fileHandling.cpp
    Core/Math/math.cpp
    Core/Components/Tag/tag.cpp
    Core/Components/Geometry/geometry.cpp
    Core/Components/Transform/transform.cpp
    Core/Components/Camera/camera.cpp
    Core/Components/Light/light.cpp
    Core/Components/Hierarchy/hierarchy.cpp
    Core/Components/Render/render.cpp
    Core/Systems/HierarchyTracker/hierarchyTracker.cpp
)
add_subdirectory("${EXTERN_DIR}/mathlib" "${BUILD_DIR}/external/mathlib")

add_library(engineCore ${CORE_SOURCES} ${CORE_HEADERS})

target_link_libraries(engineCore PUBLIC mathlib)

target_include_directories(engineCore INTERFACE Core/)

# engine OpengGL specific library

set(OPENGL_HEADERS
    OpenGL/Util/textureLoader.h
    OpenGL/Util/textureIndex.h
    OpenGL/Components/Shader/shader.h
    OpenGL/Components/Texture/texture.h
    OpenGL/Components/Material/material.h
    OpenGL/Shader/shader.h
    OpenGL/Renderer/renderer.h
    OpenGL/Systems/CameraTracker/cameraTracker.h
    OpenGL/Systems/LightsTracker/lightsTracker.h
    OpenGL/Systems/RenderTracker/renderTracker.h
)

set(OPENGL_SOURCES
    OpenGL/Util/textureLoader.cpp
    OpenGL/Util/textureIndex.cpp
    OpenGL/Components/Shader/shader.cpp
    OpenGL/Components/Texture/texture.cpp
    OpenGL/Components/Material/material.cpp
    OpenGL/Shader/shader.cpp
    OpenGL/Renderer/renderer.cpp
    OpenGL/Systems/CameraTracker/cameraTracker.cpp
    OpenGL/Systems/LightsTracker/lightsTracker.cpp
    OpenGL/Systems/RenderTracker/renderTracker.cpp
    OpenGL/Systems/RenderTracker/MaterialTracker/materialTracker.cpp
    OpenGL/Systems/RenderTracker/GeometryTracker/geometryTracker.cpp
    OpenGL/Systems/RenderTracker/TransformTracker/transformTracker.cpp
    OpenGL/Systems/RenderTracker/ShaderTracker/shaderTracker.cpp
)
add_subdirectory("${EXTERN_DIR}/stb" "${BUILD_DIR}/external/stb")

add_library(engineOpenGL ${OPENGL_SOURCES} ${OPENGL_HEADERS})

target_link_libraries(engineOpenGL PUBLIC glad mathlib stb)

target_include_directories(engineOpenGL INTERFACE Core/ OpenGL/)
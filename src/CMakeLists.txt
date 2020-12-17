# engine core library
set(CORE_HEADERS
    Util/fileHandling.h
    Core/ECS/componentTable.h
    Core/ECS/registry.h
    Core/ECS/util.h
    Core/Math/math.h
    Core/Components/Tag/tag.h
    Core/Components/Material/material.h
    Core/Components/Geometry/geometry.h
    Core/Components/Transform/transform.h
    Core/Components/Camera/camera.h
    Core/Components/Light/light.h
)

set(CORE_SOURCES
    Util/fileHandling.cpp
    Core/Math/math.cpp
    Core/Components/Tag/tag.cpp
    Core/Components/Material/material.cpp
    Core/Components/Geometry/geometry.cpp
    Core/Components/Transform/transform.cpp
    Core/Components/Camera/camera.cpp
    Core/Components/Light/light.cpp
)
add_subdirectory("${EXTERN_DIR}/mathlib" "${BUILD_DIR}/external/mathlib")

add_library(engineCore ${CORE_SOURCES} ${CORE_HEADERS})

target_link_libraries(engineCore PUBLIC mathlib)

target_include_directories(engineCore INTERFACE Core/)

# engine OpengGL specific library

set(OPENGL_HEADERS
    OpenGL/Components/Render/render.h
    OpenGL/Shader/shader.h
    OpenGL/Renderer/renderer.h
    OpenGL/Systems/CameraTracker/cameraTracker.h
    OpenGL/Systems/LightsTracker/lightsTracker.h
)

set(OPENGL_SOURCES
    OpenGL/Components/Render/render.cpp
    OpenGL/Shader/shader.cpp
    OpenGL/Renderer/renderer.cpp
    OpenGL/Systems/CameraTracker/cameraTracker.cpp
    OpenGL/Systems/LightsTracker/lightsTracker.cpp
)

add_library(engineOpenGL ${OPENGL_SOURCES} ${OPENGL_HEADERS})

target_link_libraries(engineOpenGL PUBLIC glad mathlib)

target_include_directories(engineOpenGL INTERFACE Core/ OpenGL/)
# engine core library
set(CORE_HEADERS
    Core/ECS/componentTable.h
    Core/ECS/registry.h
    Core/ECS/util.h
    Core/Math/math.h
    Core/Components/Tag/tag.h
    Core/Components/Material/material.h
    Core/Components/Geometry/geometry.h
    Core/Components/Transform/transform.h
)

set(CORE_SOURCES
    Core/Math/math.cpp
    Core/Components/Tag/tag.cpp
    Core/Components/Material/material.cpp
    Core/Components/Geometry/geometry.cpp
    Core/Components/Transform/transform.cpp
)
add_subdirectory("${EXTERN_DIR}/mathlib" "${BUILD_DIR}/external/mathlib")

add_library(engineCore ${CORE_SOURCES} ${CORE_HEADERS})

target_link_libraries(engineCore PUBLIC mathlib)

target_include_directories(engineCore INTERFACE Core/)

# engine OpengGL specific library

set(OPENGL_HEADERS
    OpenGL/Components/Render/render.h
)

set(OPENGL_SOURCES
    OpenGL/Components/Render/render.cpp
)

add_library(engineOpenGL ${OPENGL_SOURCES} ${OPENGL_HEADERS})

target_link_libraries(engineOpenGL PUBLIC glad mathlib)

target_include_directories(engineOpenGL INTERFACE Core/ OpenGL/)
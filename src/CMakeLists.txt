set(HEADERS
    Core/ECS/componentTable.h
    Core/ECS/registry.h
    Core/ECS/util.h
    Core/Math/math.h
    Core/Components/Tag/tag.h
    Core/Components/Material/material.h
    Core/Components/Geometry/geometry.h
)

set(SOURCES
    Core/Math/math.cpp
    Core/Components/Tag/tag.cpp
    Core/Components/Material/material.cpp
    Core/Components/Geometry/geometry.cpp
)
add_subdirectory("${EXTERN_DIR}/mathlib" "${BUILD_DIR}/external/mathlib")

add_library(engineCore ${SOURCES} ${HEADERS})

target_link_libraries(engineCore PUBLIC mathlib)

target_include_directories(engineCore INTERFACE Core/)
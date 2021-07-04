add_subdirectory("${EXTERN_DIR}/glad" "${BUILD_DIR}/external/glad")
add_subdirectory("${EXTERN_DIR}/glfw" "${BUILD_DIR}/external/glfw")

# imgui
set(IMGUI_SOURCES
    "${EXTERN_DIR}/imgui/imgui.cpp"
    "${EXTERN_DIR}/imgui/imgui_demo.cpp"
    "${EXTERN_DIR}/imgui/imgui_draw.cpp"
    "${EXTERN_DIR}/imgui/imgui_widgets.cpp"
    "${EXTERN_DIR}/imgui/backends/imgui_impl_opengl3.cpp"
    "${EXTERN_DIR}/imgui/backends/imgui_impl_glfw.cpp"
    "${EXTERN_DIR}/imgui/misc/cpp/imgui_stdlib.cpp"
)

set(IMGUI_INCLUDE_DIR "${EXTERN_DIR}/imgui")

add_library(modelerLib 
    modeler/glfw/window.cpp
    modeler/glad/opengl.cpp
    modeler/imgui/imgui.cpp
    modeler/imgui/Util/objectLoader.cpp
    modeler/imgui/Util/fileBrowser.cpp
    modeler/imgui/Util/errorModal.cpp
    modeler/imgui/Window/Raytracing/raytracingWindow.cpp
    modeler/imgui/Nodes/Entity/entity.cpp
    modeler/imgui/Nodes/Camera/camera.cpp
    modeler/imgui/Nodes/Geometry/geometryNode.cpp
    modeler/imgui/Nodes/OpenGLMaterial/openGLMaterial.cpp
    modeler/imgui/Nodes/OpenGLShader/openGLShader.cpp
    modeler/imgui/Nodes/Transform/transform.cpp
    modeler/imgui/Nodes/Light/light.cpp
    modeler/imgui/Nodes/Texture/texture.cpp
)

target_link_libraries(modelerLib glfw glad engineCore engineOpenGL engineRaytracing)
target_include_directories(modelerLib PUBLIC ${IMGUI_INCLUDE_DIR} ${SRC_DIR})

add_executable(Modeler modeler/modeler.cpp ${IMGUI_SOURCES})
target_include_directories(Modeler PRIVATE ${SRC_DIR})
target_link_libraries(Modeler modelerLib)
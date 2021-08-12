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

set(NLOHMANN-JSON_INCLUDE_DIR "${EXTERN_DIR}/nlohmann-json")

add_library(modelerLib 
    modeler/glfw/window.cpp
    modeler/glad/opengl.cpp
    modeler/imgui/imgui.cpp
    modeler/imgui/Util/objectLoader.cpp
    modeler/imgui/Util/SceneLoading/sceneLoader.cpp
    modeler/imgui/Util/SceneLoading/serialization.cpp
    modeler/imgui/Util/SceneLoading/parsing.cpp
    modeler/imgui/Util/fileBrowser.cpp
    modeler/imgui/Util/errorModal.cpp
    modeler/imgui/Window/Main/mainViewPort.cpp
    modeler/imgui/Window/Main/postProcesser.cpp
    modeler/imgui/Window/Raytracing/raytracingWindow.cpp
    modeler/imgui/Window/Templates/imguiWindow.cpp
    modeler/imgui/Window/Templates/componentWindow.cpp
    modeler/imgui/Window/Entity/entity.cpp
    modeler/imgui/Window/Camera/camera.cpp
    modeler/imgui/Window/Geometry/geometryNode.cpp
    modeler/imgui/Window/OpenGLMaterial/openGLMaterial.cpp
    modeler/imgui/Window/Transform/transform.cpp
    modeler/imgui/Window/Light/light.cpp
)

target_link_libraries(modelerLib glfw glad engineCore engineOpenGL engineRaytracing)
target_include_directories(modelerLib PUBLIC ${IMGUI_INCLUDE_DIR} ${SRC_DIR} ${NLOHMANN-JSON_INCLUDE_DIR})

add_executable(Modeler modeler/modeler.cpp ${IMGUI_SOURCES})
target_include_directories(Modeler PRIVATE ${SRC_DIR})
target_link_libraries(Modeler modelerLib)
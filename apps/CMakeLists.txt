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
)

set(IMGUI_INCLUDE_DIR "${EXTERN_DIR}/imgui")

add_executable(Modeler modeler.cpp ${IMGUI_SOURCES})
target_include_directories(Modeler PRIVATE ${IMGUI_INCLUDE_DIR} ${SRC_DIR})
target_link_libraries(Modeler glfw glad engineCore)
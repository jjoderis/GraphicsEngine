#version 330 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out int index;
void main() {
    FragColor = vec4(0.9, 0.1, 0.5, 1.0);
    index = -1;
}
#version 330 core
layout(location = 0) in vec3 aPos;
uniform mat4 mvp_model;
uniform mat4 mvp_view;
uniform mat4 mvp_proj;
void main() {
    gl_Position = mvp_proj * mvp_view  *mvp_model*vec4(aPos, 1);
}
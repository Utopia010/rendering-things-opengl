#version 330 core
layout(location=0) in vec3 aPos;

out vec3 VerDir;
uniform mat4 mvp_view;
uniform mat4 mvp_proj;
void main() {
    VerDir = aPos;
    vec4 pos = mvp_proj * mvp_view *vec4(aPos, 1);
    gl_Position = pos;
}
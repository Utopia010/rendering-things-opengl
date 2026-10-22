#version 330 core

layout(location = 0) in vec3 aPos;      // 顶点位置
layout(location = 1) in vec3 aNorm;      // 顶点位置
uniform mat4 mvp_model;
uniform mat4 mvp_view;
uniform mat4 mvp_proj;

out vec3 vPos;
out vec3 vNorm;

void main() {
    vec4 pos = mvp_model*vec4(aPos, 1.0);
    vPos = pos.xyz;
    vNorm = mat3(transpose(inverse(mvp_model)))*aNorm;
    gl_Position=mvp_proj * mvp_view * pos;
}
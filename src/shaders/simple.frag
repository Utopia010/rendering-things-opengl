#version 330 core


out vec4 FragColor;                     // 最终输出颜色

void main() {
    vec3 color = vec3(0.3,0.7,0.4)*(1-gl_FragCoord.z);
    FragColor = vec4(color, 1.0f);
}
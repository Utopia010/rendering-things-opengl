#version 330 core
uniform sampler2D skybox;
in vec3 VerDir;
out vec4 OutColor;
const float PI = 3.14159265359;
uniform float dividand;
vec2 getUV(vec3 dir) {
    vec2 degree = vec2(atan(dir.z, dir.x), asin(dir.y));
    degree.x /= 2*PI;
    degree.y /= PI;
    return degree+0.5;
}
void main() {
    vec2 uv = getUV(normalize(VerDir));
    vec3 color = texture(skybox, uv).rgb;
    color /= color+vec3(dividand);
    color = pow(color, vec3(1.0 / 2.2));
    OutColor = vec4(color / (color + vec3(1.0)), 0.5);
}
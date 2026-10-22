#version 330 core
out vec4 FragColor;                     // 最终输出颜色
in vec3 vNorm;
in vec3 vPos;
uniform vec3 cameraPos;
uniform vec3 lightPos;
uniform vec3 lightColor;
// teapot
uniform vec3 teapotAlbedo;
uniform float teapotMetallic;
uniform float teapotRoughness;
uniform float teapotAo;

// constant
const float PI = 3.14159265359;
// float DistributionGGX(float roughness, vec3 N, vec3 H) {
//     float upper = roughness*roughness;
//     float lower = pow(dot(N, H), 2.0)*(upper-1)+1;
//     lower = lower*lower*PI;
//     return upper/lower;
// }
// float GeometrySmith(vec3 N, vec3 V, float roughness){
//     float upper = max(dot(N,V), 0.0);
//     float k = pow(roughness+1, 2)/8.0;
//     float lower = upper*(1-k) + k;
//     return upper/lower;
// }
float DistributionGGX(float roughness, vec3 N, vec3 H) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float d = NdotH * NdotH * (a2 - 1.0) + 1.0;
    return a2 / (PI * d * d);
}

float GeometrySchlickGGX(float NdotX, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotX / (NdotX * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    return GeometrySchlickGGX(max(dot(N, V), 0.0), roughness)
         * GeometrySchlickGGX(max(dot(N, L), 0.0), roughness);
}
vec3 fresnelSchlick(vec3 F0, vec3 H, vec3 V){
    return F0 + (1-F0)*pow(1.0 - clamp(dot(H, V), 0.0, 1.0), 5.0);
}
void main() {
    vec3 V = normalize(cameraPos - vPos);
    vec3 L = normalize(lightPos - vPos);
    vec3 N = normalize(vNorm);
    vec3 F0 = mix(vec3(0.04), teapotAlbedo, teapotMetallic);
    vec3 H = normalize(V+L);
    // 衰减
    float dis= length(lightPos - vPos);
    float attenuation = 1.0 / (dis * dis);
    vec3 radiance = lightColor * attenuation;
    // 计算DFG
    float D = DistributionGGX(teapotRoughness, N, H); // D：微表面法线分布
    float G   = GeometrySmith(N, V,L, teapotRoughness); // G：几何遮蔽
    vec3 F    = fresnelSchlick(F0, H, V);

    vec3 kD = (vec3(1.0) - F) * (1.0 - teapotMetallic);
    vec3 fc = (D * G * F) /
        (4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001);
    vec3 fl = (kD * teapotAlbedo)/PI;
    vec3 fr = fl+fc;
    float cos_theta = max(dot(N, L), 0.0);
    vec3 Lo = fr * radiance * cos_theta;

    vec3 ambient = vec3(0.03)*teapotAlbedo *teapotAo;

    vec3 color  = ambient + Lo;
    color = color / (color + vec3(0.1));
    color = pow(color, vec3(1.0 / 2.2)); 
    FragColor = vec4(color, 1.0);
}
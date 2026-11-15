#version 330 core
uniform samplerCube skybox;
in vec3 skyboxCoordinate;
out vec4 OutColor;
void main() {
    OutColor = texture(skybox, skyboxCoordinate);
    
}
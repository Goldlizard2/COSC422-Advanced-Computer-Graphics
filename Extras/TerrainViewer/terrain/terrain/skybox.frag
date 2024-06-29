#version 330 core

in vec3 FragPos;

out vec4 FragColor;

uniform samplerCube skybox;

void main() {
    FragColor = 1.35 * texture(skybox, FragPos);
}
#version 330 core

layout(location = 0) in vec3 apos;

out vec3 FragPos; 

uniform mat4 mvpMatrix; 

void main() {
    FragPos = apos;  
    gl_Position = mvpMatrix * vec4(apos, 1.0);
}
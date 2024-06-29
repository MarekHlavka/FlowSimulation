#version 460

layout(location=0)in vec3 position;

uniform mat4 view;
uniform mat4 model;
uniform mat4 proj;

void main(){
    vec4 worldPos = model * vec4(position, 1.0);
    gl_Position = proj * view * worldPos;
}
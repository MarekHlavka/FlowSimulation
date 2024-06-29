#version 460

layout(location=0)in vec3 position;
layout(location=1)in vec3 test;
layout(location=2)in float col;

uniform mat4 view;
uniform mat4 model;
uniform mat4 proj;

out vec3 pos;
flat out int InstanceID;
out float color;

void main(){
    vec4 worldPos = model * vec4(position, 1.0);
    gl_Position = proj * view * worldPos;
    gl_Position.xyz += test;
    pos = gl_Position.xyz;
    color = col;
    InstanceID = gl_InstanceID;
}
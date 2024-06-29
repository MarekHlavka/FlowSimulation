#version 460

layout(location=0)in vec3 position;
layout(location=1)in vec3 translate;
layout(location=2)in vec3 v;

uniform mat4 view;
uniform mat4 model;
uniform mat4 proj;
uniform uint col;

out vec3 pos;
out float velocity;
flat out int InstanceID;

void main(){
    vec4 worldPos = model * vec4(position, 1.0);
    gl_Position = proj * view * worldPos;
    gl_Position.xyz += translate;
    velocity = 0.0;
    if(col==1){
        velocity = length(v);
    }
    InstanceID = gl_InstanceID;
}
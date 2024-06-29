#version 460

out vec4 fColor;

in vec3 pos;
in float velocity;
flat in int InstanceID;

void main(){
    fColor=vec4(mix(vec3(0, 0.4, 0.6),vec3(1,0,0),velocity),1);
}
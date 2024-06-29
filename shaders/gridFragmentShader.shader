#version 460

out vec4 fColor;

in float color;
in vec3 pos;
flat in int InstanceID;

void main(){
    fColor = vec4(color,color,color,1);

}
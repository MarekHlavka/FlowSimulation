#version 460

uniform vec3 particle0;
uniform vec3 particle1;
uniform vec3 particle2;
uniform vec3 particle3;
uniform vec3 particle4;
uniform vec3 particle5;

out vec4 fColor;

in vec3 pos;

void main(){
    // TODO: particles are 0 - 1
    float dst = 0;
    vec2 loc_pos = vec2(pos.x*2, pos.y);
    vec3 color = vec3(0.5,0.5,0.5);
    if(distance(loc_pos, particle0.xy)/sqrt(8) <= 0.4){color.x += 0.15;}
    if(distance(loc_pos, particle1.xy)/sqrt(8) <= 0.4){color.x += 0.15;}
    if(distance(loc_pos, particle2.xy)/sqrt(8) <= 0.4){color.x += 0.15;}
    if(distance(loc_pos, particle3.xy)/sqrt(8) <= 0.4){color.x += 0.15;}
    if(distance(loc_pos, particle4.xy)/sqrt(8) <= 0.4){color.x += 0.15;}
    if(distance(loc_pos, particle5.xy)/sqrt(8) <= 0.4){color.x += 0.15;}
    fColor = vec4(color, 1);
}
#version 430 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout (binding = 0) buffer PP{ float data[];} particlePositions;
layout (binding = 1) buffer PD{ float data[];} particleDirections;

uniform uint particleCnt;
uniform float radius;
uniform vec2 mouse;

float dnstSlope(float x){

  float value = max(0.0, 0.3 - x);
  return value/2;
}

void main() {
    vec2 pos = vec2(
        particlePositions.data[3*gl_GlobalInvocationID.x],
        particlePositions.data[3*gl_GlobalInvocationID.x + 1]);
    
    float dst = distance(pos, mouse);
    vec2 direction = (pos-mouse)/dst;
    float slope = dnstSlope(dst);
    
    vec2 move = direction * slope;

    particleDirections.data[2*gl_GlobalInvocationID.x] += move.x;
    particleDirections.data[2*gl_GlobalInvocationID.x + 1] += move.y;
}
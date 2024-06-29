#version 430 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout (binding = 0) buffer PP{ float data[];} particlePositions;
layout (binding = 1) buffer PD{ float data[];} particleDensities;

uniform uint particleCnt;
uniform float radius;

float PI = 3.141592653589793238;
float volume = (PI * pow(radius,4)) / 2;

float func(float x){
  float value = max(0.0, radius - x);
  return (value * value * value) / volume;
}

void main() {
  float dnst = 0;

  vec2 point = vec2(
    particlePositions.data[3*gl_GlobalInvocationID.x],
    particlePositions.data[3*gl_GlobalInvocationID.x + 1]
  );

  for(int i = 0; i < particleCnt; i++){
    vec2 loc_point = vec2(
      particlePositions.data[3*i],
      particlePositions.data[3*i+1]
    );
    dnst += func(distance(point, loc_point));
  }
  particleDensities.data[gl_GlobalInvocationID.x] = dnst;
}

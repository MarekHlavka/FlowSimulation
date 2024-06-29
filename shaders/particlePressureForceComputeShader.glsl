#version 430 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout (binding = 0) buffer PP{ float data[];} particlePositions;
layout (binding = 1) buffer PD{ float data[];} particleDensities;
layout (binding = 2) buffer PPF{ float data[];} particlePressureForces;

uniform float particleCnt;
uniform float radius;
uniform float targetDensity;
uniform float pressureMult;

float PI = 3.141592653589793238;
float volume = (PI * pow(radius,4)) / 2;

float dnstSlope(float x){
  float value = max(0.0, radius - x);
  if(x>radius){
    return 0.0;
  }
  return 3*value*value -(radius/100.0);
  //return (6*value*value)/(PI*pow(radius,4));
}

float dnstToPressure(float dnst){
  float dnstErr = dnst - targetDensity;
  float pressure = dnstErr * pressureMult;
  return pressure;
}

float calcSharedPressure(float a, float b){
  float pressureA = dnstToPressure(a);
  float pressureB = dnstToPressure(b);
  return (pressureA + pressureB)/2;
}

void main() {

  vec2 pos = vec2(
    particlePositions.data[3*gl_GlobalInvocationID.x],
    particlePositions.data[3*gl_GlobalInvocationID.x + 1]);

  float dnst = particleDensities.data[gl_GlobalInvocationID.x];

  vec2 pressure = vec2(0.0, 0.0);

  for(int i = 0; i < particleCnt; i++){

    if(gl_GlobalInvocationID.x == i){
      continue;
    }

    vec2 loc_pos = vec2(
      particlePositions.data[3*i],
      particlePositions.data[3*i + 1]);

    float dst = distance(pos, loc_pos);
    vec2 direction = (dst <= 0)? (loc_pos/10.0):((loc_pos - pos)/dst);
    float slope = dnstSlope(dst);
    float loc_dnst = particleDensities.data[i];

    // Missing particelProperties
    float sharedPressure = calcSharedPressure(loc_dnst, dnst);

    pressure += -sharedPressure*(direction * slope) / loc_dnst;
  }

  particlePressureForces.data[2*gl_GlobalInvocationID.x] = pressure.x;
  particlePressureForces.data[2*gl_GlobalInvocationID.x + 1] = pressure.y;
}

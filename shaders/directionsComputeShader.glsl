#version 430 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout (binding = 0) buffer PD{ float data[];} particleDirections;
layout (binding = 1) buffer PDN{ float data[];} particleDensities;
layout (binding = 2) buffer PPF{ float data[];} particlePressureForces;

uniform uint particleCnt;
uniform float radius;

void main() {
    vec2 pressureForce = vec2(
        particlePressureForces.data[2*gl_GlobalInvocationID.x],
        particlePressureForces.data[2*gl_GlobalInvocationID.x + 1]);

    float dnst = particleDensities.data[gl_GlobalInvocationID.x];

    vec2 pressureAcceleration = pressureForce / dnst;

    particleDirections.data[2*gl_GlobalInvocationID.x] += pressureAcceleration.x;
    particleDirections.data[2*gl_GlobalInvocationID.x + 1] += pressureAcceleration.y;
}
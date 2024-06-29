#version 430 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout (binding = 0) buffer PP{ float data[];} particlePositions;
layout (binding = 1) buffer PD{ float data[];} particleDirections;
layout (binding = 2) buffer NPP{ float data[];} nextParticlePositions;

uniform float gravityMult;

void main() {
    vec2 position = vec2(
        particlePositions.data[3*gl_GlobalInvocationID.x],
        particlePositions.data[3*gl_GlobalInvocationID.x + 1]);
    
    vec2 direction = vec2(
        particleDirections.data[2*gl_GlobalInvocationID.x],
        particleDirections.data[2*gl_GlobalInvocationID.x + 1]);

    particleDirections.data[2*gl_GlobalInvocationID.x + 1] += -1 * gravityMult * 1/60.0;

    nextParticlePositions.data[3*gl_GlobalInvocationID.x] = position.x + direction.x * 1 / 60.0;
    nextParticlePositions.data[3*gl_GlobalInvocationID.x + 1] = position.y + direction.y * 1 / 60.0;
}
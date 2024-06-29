#version 430 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout (binding = 0) buffer PP{ float data[];} particlePositions;
layout (binding = 1) buffer PD{ float data[];} particleDirections;

uniform float pull;
uniform float collisionDamp;
uniform float x_border;
uniform float y_border;

void main() {

    vec2 direction = vec2(
        particleDirections.data[2*gl_GlobalInvocationID.x],
        particleDirections.data[2*gl_GlobalInvocationID.x + 1]);

    vec3 position = vec3(
        particlePositions.data[3*gl_GlobalInvocationID.x],
        particlePositions.data[3*gl_GlobalInvocationID.x + 1],
        particlePositions.data[3*gl_GlobalInvocationID.x + 2]);

    position.x = position.x + (direction.x * pull); 
    position.y = position.y + (direction.y * pull);

    // FIXME ADD border bounce flip
    
    // X border
    if(position.x >= x_border || position.x <= -x_border){
        particleDirections.data[2*gl_GlobalInvocationID.x] *= -collisionDamp;
        if(position.x > x_border){position.x = x_border;}
        if(position.x < -x_border){position.x = -x_border;}
    }
    // Y border
    if(position.y >= y_border || position.y <= -y_border){
        particleDirections.data[2*gl_GlobalInvocationID.x + 1] *= -collisionDamp;
        if(position.y > y_border){position.y = y_border;}
        if(position.y < -y_border){position.y = -y_border;}
    }


    particlePositions.data[3*gl_GlobalInvocationID.x] = position.x;
    particlePositions.data[3*gl_GlobalInvocationID.x + 1] = position.y;
}

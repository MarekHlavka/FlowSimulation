#version 430 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout (binding = 0) buffer PP{ float data[];} particlePositions;
layout (binding = 1) buffer PD{ float data[];} particleDirections;
layout (binding = 2) buffer NPD{ float data[];} nextParticleDirections;

uniform uint particle_cnt;
uniform float gravity;
uniform uint grid_cnt;
uniform uint gravity_check;
uniform float pull_force;
uniform float radius;

float PI = 3.141592653589793238;
float volume = (PI * pow(radius,4)) / 2;

float dens(float x){
  float value = max(0.0, radius - x);
  return (value * value * value) / volume;
}

float dens_koef(float x){

  float value = max(0.0, radius - x);
  return -3*value*value;
  //return (6*value*value)/(PI*pow(radius,4));
}

void main() {
    vec2 pos = vec2(
        particlePositions.data[3*gl_GlobalInvocationID.x],
        particlePositions.data[3*gl_GlobalInvocationID.x+1]);
    
    vec2 direction = vec2(
        particleDirections.data[2*gl_GlobalInvocationID.x],
        particleDirections.data[2*gl_GlobalInvocationID.x+1]);

    float x_diff = 0.0;
    float y_diff = 0.0;
    

    for(int i = 0; i < particle_cnt; i++){

        if(gl_GlobalInvocationID.x == i){
            continue;
        }

        vec2 loc_pos = vec2(
            particlePositions.data[3*i.x],
            particlePositions.data[3*i.x+1]);

        vec2 loc_direction = vec2(
            particleDirections.data[2*i],
            particleDirections.data[2*i+1]);

        vec2 next_position = loc_pos + pull_force * loc_direction / 60.0;

        float x_sign = (next_position.x - pos.x)/3;
        float y_sign = (next_position.y - pos.y)/3;

        float dst = distance(loc_pos, pos);


        /// ADD SHARED FORCE <- ->


        x_diff = x_diff + dens_koef(dst)*(x_sign);
        y_diff = y_diff + dens_koef(dst)*(y_sign);


    }
    direction = vec2(x_diff, y_diff)*pull_force + direction;
    
    if(gravity_check == 1){
        direction.y = direction.y + gravity;
    }

    nextParticleDirections.data[2*gl_GlobalInvocationID.x] = direction.x * 0.9;
    nextParticleDirections.data[2*gl_GlobalInvocationID.x+1] = direction.y * 0.9;
}
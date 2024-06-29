#version 430 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout (binding = 0) buffer GP{ float data[];} gridPosition;
layout (binding = 1) buffer GV{ float data[];} gridValue;
layout (binding = 2) buffer PP{ float data[];} particlePositions;
layout (binding = 3) buffer PD{ float data[];} particleDirections;

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

float func(float x){
  /// float value = max(0.0, radius - x);
  /// return (value * value * value);

  float value = max(0.0, radius - x);
  return 3*value*value;
  //return (6*value*value)/(PI*pow(radius,4));
}

void main() {
    vec2 pos = vec2(
        particlePositions.data[3*gl_GlobalInvocationID.x],
        particlePositions.data[3*gl_GlobalInvocationID.x+1]);

    float x_diff = 0.0;
    float y_diff = 0.0;
    float val = 0.0;

    // Calculate density on point
    for(int i = 0; i < particle_cnt; i++){
        vec2 loc_pos = vec2(
            particlePositions.data[3*i],
            particlePositions.data[3*i+1]
        );

///////// IF DST == 0 ADD RANDOM VECTOR
///////// IF DST == 0 ADD RANDOM VECTOR
///////// IF DST == 0 ADD RANDOM VECTOR
///////// IF DST == 0 ADD RANDOM VECTOR
///////// IF DST == 0 ADD RANDOM VECTOR

        val += func(distance(pos, loc_pos));
    }

    vec2 direction = vec2(
        particleDirections.data[2*gl_GlobalInvocationID.x],
        particleDirections.data[2*gl_GlobalInvocationID.x+1]);

    for(int i = 0; i < grid_cnt; i++){
        vec2 loc_pos = vec2(
            gridPosition.data[3*i],
            gridPosition.data[3*i+1]);
        float loc_val = gridValue.data[i];

        float x_sign = sign(loc_pos.x - pos.x);
        float y_sign = sign(loc_pos.y - pos.y);

        float dst = distance(loc_pos, pos);

        x_diff = x_diff + func(dst)*(x_sign * (loc_val - val));
        y_diff = y_diff + func(dst)*(y_sign * (loc_val - val));
    
    }
    
    direction = direction + pull_force * vec2(-x_diff, -y_diff);
    
    if(gravity_check == 1){
        direction.y = direction.y + gravity;
    }
    particleDirections.data[2*gl_GlobalInvocationID.x] = direction.x * 0.9;
    particleDirections.data[2*gl_GlobalInvocationID.x+1] = direction.y * 0.9;// + gravity_vec.y;
}
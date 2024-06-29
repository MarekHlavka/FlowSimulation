#version 430 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout (binding = 0) buffer GridVals{ float data[];} gridVals;
layout (binding = 1) buffer GridVectors{ float data[];} gridVectors;

uniform uint width;

float func(float x){
  float e = 2.7182818284;
  return (1/(1+pow(e,(abs(-8*x)-2))));
}


void main() {
  float line_delta = mod(gl_GlobalInvocationID.x, width); // left / right
  float whole_lines = floor(gl_GlobalInvocationID.x/width); // up / down

  float local_val = gridVals.data[gl_GlobalInvocationID.x];

  float x_diff = 0;
  float y_diff = 0;
  bool x_in_border = false;
  bool y_in_border = false;
  // Making 3x3 neighborhood
  // FIXME Redo to Calculation based on ditance and no only neighborhood
  for(int x = -1; x <= 1; x++){ // line_delta
    
    x_in_border = line_delta + x >= 0 && line_delta + x < width; // border check in x dimension
    for(int y = -1; y <= 1; y++){ // lines diff

        y_in_border = whole_lines + y >= 0 && whole_lines + y < width;

        if(x_in_border && y_in_border){
          float diff = local_val - (gridVals.data[gl_GlobalInvocationID.x + (width*y) + x]);
          x_diff = x_diff + (diff*x);
          y_diff = y_diff + (diff*y);
        }
        else{
          x_diff = x_diff + (0.2*x);
          y_diff = y_diff + (0.2*y);
        }
      }
    //else{
    //  x_diff = x_diff + 1*x;
    //  if(!(whole_lines + y >= 0 && whole_lines + y < width)){
    //    y_diff = y_diff + 1*y;
    //  }
    //}
  }
  // FIXME remove normalization
  vec2 norm = normalize(vec2(x_diff, y_diff));

  gridVectors.data[2*gl_GlobalInvocationID.x] = y_diff;
  gridVectors.data[2*gl_GlobalInvocationID.x + 1] = -x_diff;
}

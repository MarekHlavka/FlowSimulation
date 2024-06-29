#version 430 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout (binding = 0) buffer Buffer{ uint test_data[];} data;

uniform uint wow;

void main() {
  data.test_data[gl_GlobalInvocationID.x] = 12 + gl_GlobalInvocationID.x / wow;
}

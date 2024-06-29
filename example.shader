#version 430 core

struct exampleData{
    int x;
  float y;
};

layout(binding = 0, std430) buffer exampleDataBuff {
  exampleData ED[];
};

void main() {
  ED[gl_GlobalInvocationID.x].y *= ED[gl_GlobalInvocationID.x].x;
}
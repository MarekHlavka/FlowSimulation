#include <glm/glm.hpp>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <stdlib.h>

void generateCircle(uint radius_points, float* output, float radius);
void generateTransposeInBorders(glm::vec2 left_down, glm::vec2 up_right, int point_count, float* output);
void generateDirections(int point_count, float* output);
void generateArrow(float* output, float width);
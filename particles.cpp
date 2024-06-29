#include "particles.hpp"

/// @brief Generate verts for circle object
/// @param radius_points Number of segments, which is circle divided to
/// @param output output array
/// @param radius radius of circle
void generateCircle(uint radius_points, float* output, float radius){
  float center[] = {0.0f, 0.0f, 0.0f};

  float angle = (2 * M_PI) / radius_points;

  int iteration_verts = 9;

  for(uint i = 0; i < radius_points; i++){
    output[i * iteration_verts] = center[0];
    output[i * iteration_verts + 1] = center[1];
    output[i * iteration_verts + 2] = center[2];

    output[i * iteration_verts + 6] = radius * (float)glm::cos((i) * angle);
    output[i * iteration_verts + 7] = radius * (float)glm::sin((i) * angle);
    output[i * iteration_verts + 8] = 0.0f;

    output[i * iteration_verts + 3] = radius * (float)glm::cos((i + 1) * angle);
    output[i * iteration_verts + 4] = radius * (float)glm::sin((i + 1) * angle);
    output[i * iteration_verts + 5] = 0.0f;
  
  }
}

/// @brief Genretate 3D positions (as 3 floats) randomly in borders
/// @param x_borders borders on X axis (left/right)
/// @param y_borders borders on Y axis (down/up)
/// @param point_count number of generated positions
/// @param output output array
void generateTransposeInBorders(glm::vec2 x_borders, glm::vec2 y_borders, int point_count, float* output){

    float x_range = abs(x_borders.y - x_borders.x)/4.f;
    float y_range = abs(y_borders.y - y_borders.x)/4.f;

    for(int i = 0; i < point_count; i++){

        float x = ((rand() % ((int)round(x_range*100))) / 100.f) - (x_range/2.f);
        float y = ((rand() % ((int)round(y_range*100))) / 100.f) - (y_range/2.f);

        output[3*i] = x;
        output[3*i + 1] = y;
        output[3*i + 2] = 0.f;
    }
}

/// @brief Generate directions to in random
/// @param point_count Number of directions "vectors"
/// @param output output array
void generateDirections(int point_count, float* output){

    for (int i = 0; i < point_count; i ++){
        float x = ((rand() % 100) - 50) / 1000.f;
        float y = ((rand() % 100) - 50) / 1000.f;

        output[2*i] = 0.f;//x;
        output[2*i+1] = 0.f;//y;
    }

}
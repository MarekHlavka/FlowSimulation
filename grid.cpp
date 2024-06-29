#include "grid.hpp"

void generateGridPoints(glm::vec2 x_borders, glm::vec2 y_borders, glm::vec2 counts, float* output_pos){
  int gridWidth = counts.x;
  int gridWidthHalf = glm::floor(gridWidth/2.f);
  int gridHeight = counts.y;
  int gridHeightHalf = glm::floor(gridHeight/2.f);

  float xDiff = abs(x_borders.x - x_borders.y)/(float)gridWidth;
  float yDiff = abs(y_borders.x - y_borders.y)/(float)gridHeight;
  float xPos = -gridWidthHalf*xDiff;
  for(int i = 0; i <gridWidth ; i++){
    float yPos = gridHeightHalf*yDiff;
    for(int j = 0; j <gridHeight*3; j+=3){
      output_pos[(i*gridHeight * 3) + j]     = xPos + xDiff/2.f;
      output_pos[(i*gridHeight * 3) + j + 1] = yPos - yDiff/2.f;
      output_pos[(i*gridHeight * 3) + j + 2] = 0;
      yPos -= yDiff;
    }
    xPos += xDiff;
  }
}
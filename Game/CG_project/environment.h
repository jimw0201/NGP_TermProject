#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <gl/glm/glm.hpp>
#include <gl/glm/ext.hpp>
#include <vector>

extern float obstacle_xz[5][2];
extern float FINISH_OFFSET_X;
extern float FINISH_OFFSET_Z;
extern float PARKING_X_MIN, PARKING_X_MAX, PARKING_Z_MIN, PARKING_Z_MAX;

void Environment_Init();
void Environment_SetupStage(int stage);

glm::mat4 Environment_GetObstacleMatrix(int index);
glm::mat4 Environment_GetFinishRectMatrix();


void Environment_UpdateParkingStatus(const std::vector<std::pair<float, float>>& carCorners);

#endif
#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <gl/glm/glm.hpp>
#include <gl/glm/ext.hpp>
#include <vector>

// 장애물: [주차장 인덱스 0~3][해당 주차장의 장애물 슬롯 0~4][x/z]
extern float obstacle_xz[4][5][2];
extern float obstacle_ry[4][5];         // 회전 각도
extern glm::vec3 obstacle_scale[4][5];  // 크기

// 각 주차장의 도착 위치(색깔 박스 중심)
extern float FINISH_OFFSET_X[4];
extern float FINISH_OFFSET_Z[4];

// 각 주차장의 판정 범위
extern float PARKING_X_MIN[4];
extern float PARKING_X_MAX[4];
extern float PARKING_Z_MIN[4];
extern float PARKING_Z_MAX[4];

void Environment_Init();
void Environment_SetupStage(int stage);

glm::mat4 Environment_GetObstacleMatrix(int parkingIdx, int obsIdx);
glm::mat4 Environment_GetFinishRectMatrix(int index);

void Environment_UpdateParkingStatus(const std::vector<std::pair<float, float>>& carCorners);

#endif
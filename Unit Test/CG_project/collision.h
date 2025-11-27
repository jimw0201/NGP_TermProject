#ifndef COLLISION_H
#define COLLISION_H

#include <vector>
#include <utility>

// 1. 점이 다각형 내부에 있는지 검사
bool isPointInsidePolygon(const std::vector<std::pair<float, float>>& polygon, float x, float z);

// 2. 두 선분의 교차 검사
bool doLinesIntersect(float x1, float z1, float x2, float z2, float x3, float z3, float x4, float z4);

// 3. 벽 충돌 검사
bool checkCollisionWalls(const std::vector<std::pair<float, float>>& carCorners, float wallX, float wallZ, float wallWidth, float wallHeight);

// 4. 장애물 충돌 검사
bool checkCollisionObstacle(const std::vector<std::pair<float, float>>& carCorners);

// 5. 차 vs 차 충돌 검사
bool checkCollisionCars(
    const std::vector<std::pair<float, float>>& carCornersA,
    const std::vector<std::pair<float, float>>& carCornersB);


#endif
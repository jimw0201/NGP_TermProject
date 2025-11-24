#include "collision.h"
#include "environment.h" 
#include "mesh.h"        

// 1. 점이 다각형 내부에 있는지 검사
bool isPointInsidePolygon(const std::vector<std::pair<float, float>>& polygon, float x, float z)
{
	int intersections = 0;
	int n = polygon.size();
	for (int i = 0; i < n; ++i)
	{
		auto p1 = polygon[i];
		auto p2 = polygon[(i + 1) % n];

		if ((p1.second > z) != (p2.second > z))
		{
			float intersectionX = p1.first + (z - p1.second) * (p2.first - p1.first) / (p2.second - p1.second);
			if (intersectionX > x)
			{
				intersections++;
			}
		}
	}
	return intersections % 2 == 1;
}

// 2. 선분 교차 검사
bool doLinesIntersect(float x1, float z1, float x2, float z2, float x3, float z3, float x4, float z4)
{
	auto cross = [](float ax, float ay, float bx, float by)
		{
			return ax * by - ay * bx;
		};

	float d1 = cross(x3 - x1, z3 - z1, x4 - x1, z4 - z1);
	float d2 = cross(x3 - x2, z3 - z2, x4 - x2, z4 - z2);
	float d3 = cross(x1 - x3, z1 - z3, x2 - x3, z2 - z3);
	float d4 = cross(x1 - x4, z1 - z4, x2 - x4, z2 - z4);

	return (d1 * d2 < 0 && d3 * d4 < 0);
}

// 3. 벽 충돌 검사
bool checkCollisionWalls(const std::vector<std::pair<float, float>>& carCorners, float wallX, float wallZ, float wallWidth, float wallHeight)
{
	float wallMinX = wallX - wallWidth / 2;
	float wallMaxX = wallX + wallWidth / 2;
	float wallMinZ = wallZ - wallHeight / 2;
	float wallMaxZ = wallZ + wallHeight / 2;

	for (const auto& corner : carCorners)
	{
		float cornerX = corner.first;
		float cornerZ = corner.second;

		if (cornerX > wallMinX && cornerX < wallMaxX &&
			cornerZ > wallMinZ && cornerZ < wallMaxZ)
		{
			return true;
		}
	}
	return false;
}

bool checkCollisionObstacle(const std::vector<std::pair<float, float>>& carCorners)
{
    for (int p = 0; p < 4; ++p)          // 주차장 4개
    {
        for (int i = 0; i < 5; ++i)      // 각 주차장의 장애물 슬롯 5개
        {
            float ox = obstacle_xz[p][i][0];
            float oz = obstacle_xz[p][i][1];

            // (100,100)으로 치워둔 슬롯은 패스
            if (ox == 100.0f && oz == 100.0f)
                continue;

            float obstacleMinX = ox - OBSTACLE_WIDTH;
            float obstacleMaxX = ox + OBSTACLE_WIDTH;
            float obstacleMinZ = oz - OBSTACLE_HEIGHT;
            float obstacleMaxZ = oz + OBSTACLE_HEIGHT;

            // 꼭짓점 포함 여부
            for (const auto& corner : carCorners)
            {
                if (obstacleMinX <= corner.first && corner.first <= obstacleMaxX &&
                    obstacleMinZ <= corner.second && corner.second <= obstacleMaxZ)
                {
                    return true;
                }
            }

            // (아래 다각형 / 선분 교차 부분도 ox, oz 기반으로 그대로 쓰면 됨)
            std::vector<std::pair<float, float>> obstacleCorners = {
                {obstacleMinX, obstacleMinZ},
                {obstacleMaxX, obstacleMinZ},
                {obstacleMaxX, obstacleMaxZ},
                {obstacleMinX, obstacleMaxZ}
            };

            for (const auto& corner : obstacleCorners)
            {
                if (isPointInsidePolygon(carCorners, corner.first, corner.second))
                    return true;
            }

            int carSize = carCorners.size();
            int obstacleSize = obstacleCorners.size();
            for (int i = 0; i < carSize; ++i)
            {
                for (int j = 0; j < obstacleSize; ++j)
                {
                    if (doLinesIntersect(
                        carCorners[i].first, carCorners[i].second,
                        carCorners[(i + 1) % carSize].first, carCorners[(i + 1) % carSize].second,
                        obstacleCorners[j].first, obstacleCorners[j].second,
                        obstacleCorners[(j + 1) % obstacleSize].first, obstacleCorners[(j + 1) % obstacleSize].second))
                    {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

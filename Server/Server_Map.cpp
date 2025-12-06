#include "GameLogic.h"

static const float kFinishSize = 1.0f;
static const float kFHeight = 0.75f;

// 전역 맵 데이터 실제 메모리
Obstacle_Info g_obstacles[PARKING_COUNT][OBSTACLE_SLOT_COUNT];
ParkingArea g_parkingAreas[PARKING_COUNT];
FinishPoint g_finishPoints[PARKING_COUNT];

void Server_LoadMap(int stage)
{
	for (int i = 0; i < PARKING_COUNT; ++i)
	{
		for (int j = 0; j < OBSTACLE_SLOT_COUNT; ++j)
		{
			g_obstacles[i][j].x = 100.0f;
			g_obstacles[i][j].z = 100.0f;
			g_obstacles[i][j].rotYDeg = 0.0f;
			g_obstacles[i][j].scaleX = 1.0f;
			g_obstacles[i][j].scaleZ = 1.0f;
		}
		g_parkingAreas[i].xMin = 0.0f;
		g_parkingAreas[i].xMax = 0.0f;
		g_parkingAreas[i].zMax = 0.0f;
		g_parkingAreas[i].zMin = 0.0f;

		g_finishPoints[i].x = 0.0f;
		g_finishPoints[i].z = 0.0f;
	}

	float FINISH_OFFSET_X[PARKING_COUNT] = { 0.0f, 0.0f, 0.0f, 0.0f };
	float FINISH_OFFSET_Z[PARKING_COUNT] = { 0.0f, 0.0f, 0.0f, 0.0f };

	auto setParkingArea = [&](int i, bool horizontal)
	{
		if (horizontal)
		{
			// 가로형
			g_parkingAreas[i].xMin = -kFinishSize / 2.0f + FINISH_OFFSET_X[i];
			g_parkingAreas[i].xMax = kFinishSize / 2.0f + FINISH_OFFSET_X[i];
			g_parkingAreas[i].zMin = -kFinishSize * kFHeight + FINISH_OFFSET_Z[i];
			g_parkingAreas[i].zMax = kFinishSize * kFHeight + FINISH_OFFSET_Z[i];
		}
		else
		{
			// 세로형
			g_parkingAreas[i].xMin = -kFinishSize * kFHeight + FINISH_OFFSET_X[i];
			g_parkingAreas[i].xMax = kFinishSize * kFHeight + FINISH_OFFSET_X[i];
			g_parkingAreas[i].zMin = -kFinishSize / 2.0f + FINISH_OFFSET_Z[i];
			g_parkingAreas[i].zMax = kFinishSize / 2.0f + FINISH_OFFSET_Z[i];
		}
		g_finishPoints[i].x = FINISH_OFFSET_X[i];
		g_finishPoints[i].z = FINISH_OFFSET_Z[i];
	};

	// 스테이지 1
	if (stage == 1)
	{
		// 주차 구역 위치
		FINISH_OFFSET_X[0] = -1.05f * 1.5f;
		FINISH_OFFSET_Z[0] = 4.5f;

		FINISH_OFFSET_X[1] = -4.5f;
		FINISH_OFFSET_Z[1] = -1.05f * 1.5f;

		FINISH_OFFSET_X[2] = 4.5f;
		FINISH_OFFSET_Z[2] = 1.05f * 1.5f;

		FINISH_OFFSET_X[3] = 1.05f * 1.5f;
		FINISH_OFFSET_Z[3] = -4.5f;

		for (int i = 0; i < 4; ++i) {
			if (i == 0 || i == 3)
			{
				setParkingArea(i, true);
			}
			else
			{
				setParkingArea(i, false);
			}
		}

		// 장애물 배치
		g_obstacles[0][0].x = FINISH_OFFSET_X[0] - 1.05f;
		g_obstacles[0][0].z = FINISH_OFFSET_Z[0];

		g_obstacles[0][1].x = FINISH_OFFSET_X[0] + 1.05f;
		g_obstacles[0][1].z = FINISH_OFFSET_Z[0];

		g_obstacles[0][2].x = FINISH_OFFSET_X[0] + 1.05f * 2.0f;
		g_obstacles[0][2].z = FINISH_OFFSET_Z[0];

		g_obstacles[0][3].x = FINISH_OFFSET_X[0] + 1.05f * 3.0f;
		g_obstacles[0][3].z = FINISH_OFFSET_Z[0];

		g_obstacles[0][4].x = FINISH_OFFSET_X[0] + 1.05f * 4.0f;
		g_obstacles[0][4].z = FINISH_OFFSET_Z[0];
		

		g_obstacles[1][0].x = FINISH_OFFSET_X[1];
		g_obstacles[1][0].z = FINISH_OFFSET_Z[1] - 1.05f;
		g_obstacles[1][0].rotYDeg = 90.0f;

		g_obstacles[1][1].x = FINISH_OFFSET_X[1];
		g_obstacles[1][1].z = FINISH_OFFSET_Z[1] + 1.05f;
		g_obstacles[1][1].rotYDeg = 90.0f;

		g_obstacles[1][2].x = FINISH_OFFSET_X[1];
		g_obstacles[1][2].z = FINISH_OFFSET_Z[1] + 1.05f * 2.0f;
		g_obstacles[1][2].rotYDeg = 90.0f;

		g_obstacles[1][3].x = FINISH_OFFSET_X[1];
		g_obstacles[1][3].z = FINISH_OFFSET_Z[1] + 1.05f * 3.0f;
		g_obstacles[1][3].rotYDeg = 90.0f;

		g_obstacles[1][4].x = FINISH_OFFSET_X[1];
		g_obstacles[1][4].z = FINISH_OFFSET_Z[1] + 1.05f * 4.0f;
		g_obstacles[1][4].rotYDeg = 90.0f;


		g_obstacles[2][0].x = FINISH_OFFSET_X[2];
		g_obstacles[2][0].z = FINISH_OFFSET_Z[2] - 1.05f * 4.0f;
		g_obstacles[2][0].rotYDeg = 90.0f;

		g_obstacles[2][1].x = FINISH_OFFSET_X[2];
		g_obstacles[2][1].z = FINISH_OFFSET_Z[2] - 1.05f * 3.0f;
		g_obstacles[2][1].rotYDeg = 90.0f;

		g_obstacles[2][2].x = FINISH_OFFSET_X[2];
		g_obstacles[2][2].z = FINISH_OFFSET_Z[2] - 1.05f * 2.0f;
		g_obstacles[2][2].rotYDeg = 90.0f;

		g_obstacles[2][3].x = FINISH_OFFSET_X[2];
		g_obstacles[2][3].z = FINISH_OFFSET_Z[2] - 1.05f;
		g_obstacles[2][3].rotYDeg = 90.0f;

		g_obstacles[2][4].x = FINISH_OFFSET_X[2];
		g_obstacles[2][4].z = FINISH_OFFSET_Z[2] + 1.05f;
		g_obstacles[2][4].rotYDeg = 90.0f;


		g_obstacles[3][0].x = FINISH_OFFSET_X[3] - 1.05f * 4.0f;
		g_obstacles[3][0].z = FINISH_OFFSET_Z[3];

		g_obstacles[3][1].x = FINISH_OFFSET_X[3] - 1.05f * 3.0f;
		g_obstacles[3][1].z = FINISH_OFFSET_Z[3];

		g_obstacles[3][2].x = FINISH_OFFSET_X[3] - 1.05f * 2.0f;
		g_obstacles[3][2].z = FINISH_OFFSET_Z[3];

		g_obstacles[3][3].x = FINISH_OFFSET_X[3] - 1.05f;
		g_obstacles[3][3].z = FINISH_OFFSET_Z[3];

		g_obstacles[3][4].x = FINISH_OFFSET_X[3] + 1.05f;
		g_obstacles[3][4].z = FINISH_OFFSET_Z[3];
	}

	// 2스테이지
	else if (stage == 2)
	{
		// 차 1번 주차 구역
		FINISH_OFFSET_X[0] = 1.55f * 1.5f;
		FINISH_OFFSET_Z[0] = 4.5f;

		// 차 2번 주차 구역
		FINISH_OFFSET_X[1] = 1.55f * 1.5f;
		FINISH_OFFSET_Z[1] = -4.5f;

		// 차 3번 주차 구역
		FINISH_OFFSET_X[2] = -1.55f * 1.5f;
		FINISH_OFFSET_Z[2] = 4.5f;

		// 차 4번 주차 구역
		FINISH_OFFSET_X[3] = -1.55f * 1.5f;
		FINISH_OFFSET_Z[3] = -4.5f;

		// 모두 세로형
		for (int i = 0; i < 4; ++i)
		{
			setParkingArea(i, false);
		}

		g_obstacles[0][0].x = FINISH_OFFSET_X[0] - 1.55f;
		g_obstacles[0][0].z = FINISH_OFFSET_Z[0];
		g_obstacles[0][0].rotYDeg = 90.0f;

		g_obstacles[0][1].x = FINISH_OFFSET_X[0] + 1.55f;
		g_obstacles[0][1].z = FINISH_OFFSET_Z[0];
		g_obstacles[0][1].rotYDeg = 90.0f;


		g_obstacles[1][0].x = FINISH_OFFSET_X[1] - 1.55f;
		g_obstacles[1][0].z = FINISH_OFFSET_Z[1];
		g_obstacles[1][0].rotYDeg = 90.0f;

		g_obstacles[1][1].x = FINISH_OFFSET_X[1] + 1.55f;
		g_obstacles[1][1].z = FINISH_OFFSET_Z[1];
		g_obstacles[1][1].rotYDeg = 90.0f;


		g_obstacles[2][0].x = FINISH_OFFSET_X[2] - 1.55f;
		g_obstacles[2][0].z = FINISH_OFFSET_Z[2];
		g_obstacles[2][0].rotYDeg = 90.0f;

		g_obstacles[2][1].x = FINISH_OFFSET_X[2] + 1.55f;
		g_obstacles[2][1].z = FINISH_OFFSET_Z[2];
		g_obstacles[2][1].rotYDeg = 90.0f;


		g_obstacles[3][0].x = FINISH_OFFSET_X[3] - 1.55f;
		g_obstacles[3][0].z = FINISH_OFFSET_Z[3];
		g_obstacles[3][0].rotYDeg = 90.0f;

		g_obstacles[3][1].x = FINISH_OFFSET_X[3] + 1.55f;
		g_obstacles[3][1].z = FINISH_OFFSET_Z[3];
		g_obstacles[3][1].rotYDeg = 90.0f;
	}

	// 3스테이지
	else if (stage == 3)
	{
		// 1. 주차 공간 배치
		FINISH_OFFSET_X[0] = -4.5f;
		FINISH_OFFSET_Z[0] = -4.5f;

		FINISH_OFFSET_X[1] = -4.5f;
		FINISH_OFFSET_Z[1] = 4.5f;

		FINISH_OFFSET_X[2] = 4.5f;
		FINISH_OFFSET_Z[2] = -4.5f;

		FINISH_OFFSET_X[3] = 4.5f;
		FINISH_OFFSET_Z[3] = 4.5f;

		// 2. 주차 판정 범위
		for (int i = 0; i < 4; ++i)
		{
			setParkingArea(i, true);
		}

		// 3. 장애물 (긴 벽 2개)
		g_obstacles[0][0].x = 0.0f;
		g_obstacles[0][0].z = -4.5f;
		g_obstacles[0][0].scaleX = 2.0f;
		g_obstacles[0][0].scaleZ = 6.0f;

		g_obstacles[0][1].x = 0.0f;
		g_obstacles[0][1].z = 4.5f;
		g_obstacles[0][1].scaleX = 2.0f;
		g_obstacles[0][1].scaleZ = 6.0f;
	}

	// 그 외 값 들어오면 기본적으로 1스테이지로 사용
	else
	{
		Server_LoadMap(1);
	}

}
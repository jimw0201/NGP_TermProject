#include "environment.h"
#include "mesh.h"      
#include "game_state.h"

float FINISH_OFFSET_X = 0.0f;
float FINISH_OFFSET_Z = 0.0f;

float obstacle_xz[5][2] = {
	{FINISH_OFFSET_X - 1.05f, FINISH_OFFSET_Z},
	{FINISH_OFFSET_X + 1.05f, FINISH_OFFSET_Z},
	{FINISH_OFFSET_X - 1.05f * 2, FINISH_OFFSET_Z},
	{FINISH_OFFSET_X + 1.05f * 2, FINISH_OFFSET_Z},
	{100.0f, 100.0f}
};

float PARKING_X_MIN = -FINISH_SIZE / 2 + FINISH_OFFSET_X;
float PARKING_X_MAX = FINISH_SIZE / 2 + FINISH_OFFSET_X;
float PARKING_Z_MIN = -FINISH_SIZE * fheight + FINISH_OFFSET_Z;
float PARKING_Z_MAX = FINISH_SIZE * fheight + FINISH_OFFSET_Z;


void Environment_Init()
{
	Environment_SetupStage(1);
}

void Environment_SetupStage(int stage)
{
    // 1스테이지
    if (stage == 1)
    {
        FINISH_OFFSET_X = 0.0f;
        FINISH_OFFSET_Z = 0.0f;

        PARKING_X_MIN = -FINISH_SIZE / 2.0f + FINISH_OFFSET_X;
        PARKING_X_MAX = FINISH_SIZE / 2.0f + FINISH_OFFSET_X;
        PARKING_Z_MIN = -FINISH_SIZE * fheight + FINISH_OFFSET_Z;
        PARKING_Z_MAX = FINISH_SIZE * fheight + FINISH_OFFSET_Z;

        obstacle_xz[0][0] = FINISH_OFFSET_X - 1.05f;
        obstacle_xz[0][1] = FINISH_OFFSET_Z;

        obstacle_xz[1][0] = FINISH_OFFSET_X + 1.05f;
        obstacle_xz[1][1] = FINISH_OFFSET_Z;

        obstacle_xz[2][0] = FINISH_OFFSET_X - 1.05f * 2;
        obstacle_xz[2][1] = FINISH_OFFSET_Z;

        obstacle_xz[3][0] = FINISH_OFFSET_X + 1.05f * 2;
        obstacle_xz[3][1] = FINISH_OFFSET_Z;

        obstacle_xz[4][0] = 100.0f;
        obstacle_xz[4][1] = 100.0f;
    }
    // 2스테이지
    else if (stage == 2)
    {
        FINISH_OFFSET_X = 3.0f;
        FINISH_OFFSET_Z = 0.0f;

        PARKING_X_MIN = -FINISH_SIZE / 2.0f + FINISH_OFFSET_X;
        PARKING_X_MAX = FINISH_SIZE / 2.0f + FINISH_OFFSET_X;
        PARKING_Z_MIN = -FINISH_SIZE * fheight + FINISH_OFFSET_Z;
        PARKING_Z_MAX = FINISH_SIZE * fheight + FINISH_OFFSET_Z;

        obstacle_xz[0][0] = FINISH_OFFSET_X;
        obstacle_xz[0][1] = FINISH_OFFSET_Z + 1.55f;

        obstacle_xz[1][0] = FINISH_OFFSET_X;
        obstacle_xz[1][1] = FINISH_OFFSET_Z - 1.55f;

        obstacle_xz[2][0] = FINISH_OFFSET_X - 1.05f;
        obstacle_xz[2][1] = FINISH_OFFSET_Z - 1.55f;

        obstacle_xz[3][0] = FINISH_OFFSET_X - 1.05f * 2.0f;
        obstacle_xz[3][1] = FINISH_OFFSET_Z - 1.55f;

        obstacle_xz[4][0] = 100.0f;
        obstacle_xz[4][1] = 100.0f;
    }
    // 3스테이지
    else if (stage == 3)
    {
        FINISH_OFFSET_X = -2.0f;
        FINISH_OFFSET_Z = -4.0f;

        PARKING_X_MIN = -FINISH_SIZE / 2 + FINISH_OFFSET_X;
        PARKING_X_MAX = FINISH_SIZE / 2 + FINISH_OFFSET_X;
        PARKING_Z_MIN = -FINISH_SIZE * fheight + FINISH_OFFSET_Z;
        PARKING_Z_MAX = FINISH_SIZE * fheight + FINISH_OFFSET_Z;

        obstacle_xz[0][0] = FINISH_OFFSET_X - 1.05f;
        obstacle_xz[0][1] = FINISH_OFFSET_Z;

        obstacle_xz[1][0] = FINISH_OFFSET_X - 1.05f;
        obstacle_xz[1][1] = FINISH_OFFSET_Z + 1.55f;

        obstacle_xz[2][0] = FINISH_OFFSET_X;
        obstacle_xz[2][1] = FINISH_OFFSET_Z + 1.55f;

        obstacle_xz[3][0] = FINISH_OFFSET_X + 1.05f;
        obstacle_xz[3][1] = FINISH_OFFSET_Z + 1.55f;

        obstacle_xz[4][0] = FINISH_OFFSET_X + 1.05f;
        obstacle_xz[4][1] = FINISH_OFFSET_Z + 1.55f * 2.0f;
    }
}

glm::mat4 Environment_GetObstacleMatrix(int index)
{
	glm::mat4 T = glm::mat4(1.0f);
	glm::vec3 position(obstacle_xz[index][0], fy, obstacle_xz[index][1]);
	T = glm::translate(T, position);
	return T;
}

glm::mat4 Environment_GetFinishRectMatrix()
{
	glm::mat4 T = glm::mat4(1.0f);
	T = glm::translate(T, glm::vec3(FINISH_OFFSET_X, fy, FINISH_OFFSET_Z));
	return T;
}

void Environment_UpdateParkingStatus(const std::vector<std::pair<float, float>>& carCorners)
{
	bool newIsParked = false;
	int checkCount = 0;
	for (const auto& corner : carCorners)
	{
		float cornerX = corner.first;
		float cornerZ = corner.second;
		if (PARKING_X_MIN <= cornerX && cornerX <= PARKING_X_MAX &&
			PARKING_Z_MIN <= cornerZ && cornerZ <= PARKING_Z_MAX)
		{
			checkCount++;
		}
	}
	if (checkCount >= 4)
	{
		newIsParked = true;
	}

	if (newIsParked != GameState_IsParked())
	{
		GameState_SetParked(newIsParked);
	}
}
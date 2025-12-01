#include "car.h"
#include "mesh.h"

// 차량 상태(위치, 회전, 속도 등)
static CarState g_cars[kCarCount];
static float g_frontWheelRotY[kCarCount];

// 입력 상태: 일단 4대가 같이 쓰게 함
static bool isAcceleratingForward;
static bool isAcceleratingBackward;
static bool isBraking;

static const float MAX_SPEED = 0.01f;
static const float acceleration = 0.001f;
static const float deceleration = 0.005f;
static const float friction = 0.0001f;

void Car_InitForStage(int stage);

void Car_UpdateSpeed(const CarInput& input, int carIndex)
{
	float& speed = g_cars[carIndex].speed;
	GearState currentGear = input.gear;

	// PARK / NEUTRAL 은 가속 불가
	if (currentGear == PARK || currentGear == NEUTRAL)
	{
		speed = 0.0f;
		return;
	}

	// 후진
	if (currentGear == REVERSE && input.accelBackward)
	{
		speed -= acceleration;
		if (speed < -MAX_SPEED)
			speed = -MAX_SPEED;
	}

	// 전진
	if (currentGear == DRIVE && input.accelForward)
	{
		speed += acceleration;
		if (speed > MAX_SPEED)
			speed = MAX_SPEED;
	}

	// 브레이크
	if (input.brake)
	{
		if (speed > 0.0f)
		{
			speed -= deceleration;
			if (speed < 0.0f) speed = 0.0f;
		}
		else if (speed < 0.0f)
		{
			speed += deceleration;
			if (speed > 0.0f) speed = 0.0f;
		}
	}

	// 자연 감속
	if (!input.accelForward && !input.accelBackward && !input.brake)
	{
		if (speed > 0.0f)
		{
			speed -= friction;
			if (speed < 0.0f) speed = 0.0f;
		}
		else if (speed < 0.0f)
		{
			speed += friction;
			if (speed > 0.0f) speed = 0.0f;
		}
	}
}

void Car_UpdateSpeed(const CarInput& input)
{
	Car_UpdateSpeed(input, 0);
}


void Car_Init()
{
	Car_InitForStage(1);
}

// 스테이지별 시작 위치/회전 설정
void Car_InitForStage(int stage)
{
	for (int i = 0; i < kCarCount; ++i)
	{
		// 높이는 대부분 고정이니 공통으로
		g_cars[i].dy = 0.125f;
		g_cars[i].speed = 0.0f;
		g_cars[i].wheelRotX = 0.0f;

		g_frontWheelRotY[i] = 0.0f;
	}

	switch (stage)
	{
	case 1:
		// 스테이지 1 플레이어 배치
		g_cars[0].dx = 4.5f; g_cars[0].dz = -4.5f; g_cars[0].rotY = 0.0f;
		g_cars[1].dx = 4.5f; g_cars[1].dz = 4.5f; g_cars[1].rotY = -90.0f;
		g_cars[2].dx = -4.5f; g_cars[2].dz = 4.5f; g_cars[2].rotY = 180.0f;
		g_cars[3].dx = -4.5f; g_cars[3].dz = -4.5f; g_cars[3].rotY = 90.0f;
		break;

	case 2:
		// 스테이지 2 플레이어 배치
		g_cars[0].dx = 0.0f; g_cars[0].dz = -6.0f; g_cars[0].rotY = 0.0f;
		g_cars[1].dx = 2.0f; g_cars[1].dz = -6.0f; g_cars[1].rotY = 0.0f;
		g_cars[2].dx = -2.0f; g_cars[2].dz = -6.0f; g_cars[2].rotY = 0.0f;
		g_cars[3].dx = 0.0f; g_cars[3].dz = -8.0f; g_cars[3].rotY = 0.0f;
		break;

	case 3:
		// 스테이지 3 플레이어 배치
		g_cars[0].dx = -6.0f; g_cars[0].dz = -4.0f; g_cars[0].rotY = 90.0f;
		g_cars[1].dx = -6.0f; g_cars[1].dz = 0.0f; g_cars[1].rotY = 90.0f;
		g_cars[2].dx = -6.0f; g_cars[2].dz = 4.0f; g_cars[2].rotY = 90.0f;
		g_cars[3].dx = -2.0f; g_cars[3].dz = -4.0f; g_cars[3].rotY = 0.0f;
		break;

	default:
		Car_InitForStage(1);
		break;
	}

	//front_wheels_rotateY = 0.0f;
	isAcceleratingForward = false;
	isAcceleratingBackward = false;
	isBraking = false;
}


// 차체의 변환 - 이를 기준으로 헤드라이트, 바퀴 등의 위치가 정해진다.
glm::mat4 Car_Body(int carIndex)
{
    glm::mat4 T = glm::mat4(1.0f);
    glm::mat4 Ry = glm::mat4(1.0f);

	Ry = glm::rotate(Ry, glm::radians(g_cars[carIndex].rotY), glm::vec3(0.0, 1.0, 0.0));
	T = glm::translate(T,
		glm::vec3(g_cars[carIndex].dx, g_cars[carIndex].dy, g_cars[carIndex].dz));

    return T * Ry;
}

glm::mat4 Car_Body()
{
    return Car_Body(0);
}

glm::mat4 Headlights(int left_right, int carIndex)
{
	glm::mat4 T = glm::mat4(1.0f);
	glm::mat4 Ry = glm::mat4(1.0f);
	//glm::mat4 S = glm::mat4(1.0f);

	if (left_right == 0)
	{
		T = glm::translate(T, glm::vec3(-CAR_SIZE / 3, CAR_SIZE / 8, CAR_SIZE));
	}
	else if (left_right == 1)
	{
		T = glm::translate(T, glm::vec3(CAR_SIZE / 3, CAR_SIZE / 8, CAR_SIZE));
	}

	return Car_Body(carIndex) * T;
}

glm::mat4 Headlights(int left_right)
{
	return Headlights(left_right, 0);
}

// 바퀴 변환 - 앞바퀴 회전
glm::mat4 Wheels(int num, int carIndex)
{
	glm::mat4 T2 = glm::mat4(1.0f);
	if (num == 1) //앞 기준 왼쪽 앞
	{
		T2 = glm::translate(T2, glm::vec3(-(CAR_SIZE / 2 + WHEEL_SIZE / 4), 0.0f, CAR_SIZE * 0.5f));
	}
	if (num == 2) //오른쪽 앞
	{
		T2 = glm::translate(T2, glm::vec3(CAR_SIZE / 2 + WHEEL_SIZE / 4, 0.0f, CAR_SIZE * 0.5f));
	}
	if (num == 3) //왼쪽 뒤
	{
		T2 = glm::translate(T2, glm::vec3(-(CAR_SIZE / 2 + WHEEL_SIZE / 4), 0.0f, -CAR_SIZE * 0.5f));
	}
	if (num == 4)  //오른쪽 뒤
	{
		T2 = glm::translate(T2, glm::vec3(CAR_SIZE / 2 + WHEEL_SIZE / 4, 0.0f, -CAR_SIZE * 0.5f));
	}
	return Car_Body(carIndex) * T2;
}

glm::mat4 Wheels(int num)
{
	return Wheels(num, 0);
}

glm::mat4 Wheel_rects(int num, int carIndex)
{
	glm::mat4 T = glm::mat4(1.0f);
	glm::mat4 T2 = glm::mat4(1.0f);
	glm::mat4 Ry = glm::mat4(1.0f);

	if (num == 1 || num == 2)
	{
		float steerY = Car_GetFrontWheelRotationY(carIndex);
		Ry = glm::rotate(Ry, glm::radians(steerY), glm::vec3(0.0f, 1.0f, 0.0f));
	}
	if (num == 1) //앞 기준 왼쪽 앞
	{
		T = glm::translate(T, glm::vec3(-(0.001f), 0.0f, 0.0f));
		T2 = glm::translate(T2, glm::vec3(-(CAR_SIZE / 2 + WHEEL_SIZE / 4), 0.0f, CAR_SIZE * 0.5f));
	}
	if (num == 2) //오른쪽 앞
	{
		T = glm::translate(T, glm::vec3((0.001f), 0.0f, 0.0f));
		T2 = glm::translate(T2, glm::vec3(CAR_SIZE / 2 + WHEEL_SIZE / 4, 0.0f, CAR_SIZE * 0.5f));
	}
	if (num == 3) //왼쪽 뒤
	{
		T = glm::translate(T, glm::vec3(-(0.001f), 0.0f, 0.0f));
		T2 = glm::translate(T2, glm::vec3(-(CAR_SIZE / 2 + WHEEL_SIZE / 4), 0.0f, -CAR_SIZE * 0.5f));
	}
	if (num == 4)  //오른쪽 뒤
	{
		T = glm::translate(T, glm::vec3((0.001f), 0.0f, 0.0f));
		T2 = glm::translate(T2, glm::vec3(CAR_SIZE / 2 + WHEEL_SIZE / 4, 0.0f, -CAR_SIZE * 0.5f));
	}

	glm::mat4 Rx = glm::mat4(1.0f);
	Rx = glm::rotate(Rx, glm::radians(g_cars[carIndex].wheelRotX), glm::vec3(1.0, 0.0, 0.0));

	return Car_Body(carIndex) * T2 * Ry * Rx * T;
}

glm::mat4 Wheel_rects(int num)
{
	return Wheel_rects(num, 0);
}

glm::mat4 Wheel_on_000(int num, int type, int carIndex) //num은 4개 바퀴의 번호, type은 실린더, 뚜껑 객체 종류
{
	glm::mat4 T = glm::mat4(1.0f);
	glm::mat4 Ry = glm::mat4(1.0f);
	glm::mat4 Ry2 = glm::mat4(1.0f);
	//glm::mat4 S = glm::mat4(1.0f);

	Ry = glm::rotate(Ry, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));

	if (num == 1 || num == 2)
	{
		float steerY = Car_GetFrontWheelRotationY(carIndex);
		Ry2 = glm::rotate(Ry2, glm::radians(steerY), glm::vec3(0.0f, 1.0f, 0.0f));
	}
	if (type == 0)		//바퀴
	{
		T = glm::translate(T, glm::vec3(0.0f, 0.0f, -WHEEL_SIZE / 4));
	}
	else if (type == 1) //바퀴 옆면1
	{
		T = glm::translate(T, glm::vec3(0.0f, 0.0f, -WHEEL_SIZE / 4));
	}
	else if (type == 2) //바퀴 옆면2
	{
		T = glm::translate(T, glm::vec3(0.0f, 0.0f, WHEEL_SIZE / 4));
	}

	return Wheels(num, carIndex) * Ry2 * Ry * T;
}

glm::mat4 Wheel_on_000(int num, int type)
{
	return Wheel_on_000(num, type, 0);
}

std::vector<std::pair<float, float>> Car_GetRotatedCorners(float x, float z, float angle)
{
	float halfWidth = CAR_SIZE / 2;
	float halfHeight = CAR_SIZE;

	// 꼭짓점의 상대 좌표
	std::vector<std::pair<float, float>> corners = {
		{-halfWidth, -halfHeight}, // 좌하단
		{halfWidth, -halfHeight},  // 우하단
		{halfWidth, halfHeight},   // 우상단
		{-halfWidth, halfHeight}   // 좌상단
	};

	// 회전 각도(라디안)
	float radians = glm::radians(-angle);

	// 회전된 꼭짓점 좌표
	std::vector<std::pair<float, float>> rotatedCorners;
	for (const auto& corner : corners)
	{
		float rotatedX = corner.first * cos(radians) - corner.second * sin(radians);
		float rotatedZ = corner.first * sin(radians) + corner.second * cos(radians);
		rotatedCorners.emplace_back(x + rotatedX, z + rotatedZ);
	}
	return rotatedCorners;
}

std::vector<std::pair<float, float>> Car_GetRotatedCorners(int carIndex)
{
	return Car_GetRotatedCorners(
		g_cars[carIndex].dx,
		g_cars[carIndex].dz,
		g_cars[carIndex].rotY
	);
}

std::vector<std::pair<float, float>> Car_GetRotatedCorners()
{
	return Car_GetRotatedCorners(0);
}

float Car_GetDX(int carIndex) { return g_cars[carIndex].dx; }
float Car_GetDY(int carIndex) { return g_cars[carIndex].dy; }
float Car_GetDZ(int carIndex) { return g_cars[carIndex].dz; }
float Car_GetRotationY(int carIndex) { return g_cars[carIndex].rotY; }
float Car_GetWheelRotationX(int carIndex) { return g_cars[carIndex].wheelRotX; }
float Car_GetSpeed(int carIndex) { return g_cars[carIndex].speed; }

float Car_GetDX() { return Car_GetDX(0); }
float Car_GetDY() { return Car_GetDY(0); }
float Car_GetDZ() { return Car_GetDZ(0); }
float Car_GetRotationY() { return Car_GetRotationY(0); }
float Car_GetWheelRotationX() { return Car_GetWheelRotationX(0); }
float Car_GetSpeed() { return Car_GetSpeed(0); }

//float Car_GetFrontWheelRotationY() { return g_frontWheelRotY[0]; }
bool  Car_IsAcceleratingForward() { return isAcceleratingForward; }
bool  Car_IsAcceleratingBackward() { return isAcceleratingBackward; }
bool  Car_IsBraking() { return isBraking; }

const CarState& Car_GetState(int carIndex)
{
	return g_cars[carIndex];
}

void Car_SetState(int carIndex, const CarState& state)
{
	g_cars[carIndex] = state;
}

void Car_SetPosition(int carIndex, float dx, float dz)
{
	g_cars[carIndex].dx = dx;
	g_cars[carIndex].dz = dz;
}

void Car_SetRotationY(int carIndex, float angle) { g_cars[carIndex].rotY = angle; }
void Car_SetWheelRotationX(int carIndex, float angle) { g_cars[carIndex].wheelRotX = angle; }
void Car_SetSpeed(int carIndex, float speed) { g_cars[carIndex].speed = speed; }

void Car_SetPosition(float dx, float dz) { Car_SetPosition(0, dx, dz); }
void Car_SetRotationY(float angle) { Car_SetRotationY(0, angle); }
void Car_SetWheelRotationX(float angle) { Car_SetWheelRotationX(0, angle); }
void Car_SetSpeed(float speed) { Car_SetSpeed(0, speed); }

void Car_SetFrontWheelRotationY(float angle) { g_frontWheelRotY[0] = angle; }
void Car_SetAcceleratingForward(bool status) { isAcceleratingForward = status; }
void Car_SetAcceleratingBackward(bool status) { isAcceleratingBackward = status; }
void Car_SetBraking(bool status) { isBraking = status; }

int Car_Count()
{
	return kCarCount;
}

// 차별 조향각 (멀티용)
void Car_SetFrontWheelRotationY(int carIndex, float angle)
{
	if (carIndex < 0 || carIndex >= kCarCount) return;
	g_frontWheelRotY[carIndex] = angle;
}

float Car_GetFrontWheelRotationY(int carIndex)
{
	if (carIndex < 0 || carIndex >= kCarCount) return 0.0f;
	return g_frontWheelRotY[carIndex];
}

float Car_GetFrontWheelRotationY()
{
	return Car_GetFrontWheelRotationY(0);
}
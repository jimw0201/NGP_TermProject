#include "car.h"
#include "mesh.h"

static float car_dx, car_dy, car_dz;
static float car_rotateY;

static float front_wheels_rotateY;
static float wheel_rect_rotateX;

static float car_speed;
static bool isAcceleratingForward;
static bool isAcceleratingBackward;
static bool isBraking;

static const float MAX_SPEED = 0.01f;
static const float acceleration = 0.001f;
static const float deceleration = 0.005f;
static const float friction = 0.0001f;

void Car_UpdateSpeed(GearState currentGear)
{
	if (currentGear == PARK || currentGear == NEUTRAL)
	{
		car_speed = 0.0f; // 정지
	}

	// 후진 처리
	if (currentGear == REVERSE && isAcceleratingBackward)
	{
		car_speed -= acceleration;
		if (car_speed < -MAX_SPEED)
			car_speed = -MAX_SPEED;
	}

	// 전진 처리
	if (currentGear == DRIVE && isAcceleratingForward)
	{
		car_speed += acceleration;
		if (car_speed > MAX_SPEED)
			car_speed = MAX_SPEED;
	}

	if (isBraking)
	{
		if (car_speed > 0.0f)
		{
			car_speed -= deceleration;
			if (car_speed < 0.0f)
				car_speed = 0.0f;
		}
		else if (car_speed < 0.0f)
		{
			car_speed += deceleration;
			if (car_speed > 0.0f)
				car_speed = 0.0f;
		}
	}

	if (!isAcceleratingForward && !isAcceleratingBackward && !isBraking)
	{
		// 자연 감속
		if (car_speed > 0.0f)
		{
			car_speed -= friction;
			if (car_speed < 0.0f)
				car_speed = 0.0f;
		}
		else if (car_speed < 0.0f)
		{
			car_speed += friction;
			if (car_speed > 0.0f)
				car_speed = 0.0f;
		}
	}
}

void Car_Init()
{
    car_dx = 0.0f;
    car_dy = WHEEL_SIZE;
    car_dz = -3.0f;
    car_rotateY = 0.0f;

    front_wheels_rotateY = 0.0f;
    wheel_rect_rotateX = 0.0f;

    car_speed = 0.0f;
    isAcceleratingForward = false;
    isAcceleratingBackward = false;
    isBraking = false;
}

// 차체의 변환 - 이를 기준으로 헤드라이트, 바퀴 등의 위치가 정해진다.
glm::mat4 Car_Body()
{
	glm::mat4 T = glm::mat4(1.0f);
	glm::mat4 Ry = glm::mat4(1.0f);
	//glm::mat4 S = glm::mat4(1.0f);

	Ry = glm::rotate(Ry, glm::radians(car_rotateY), glm::vec3(0.0, 1.0, 0.0));
	T = glm::translate(T, glm::vec3(car_dx, car_dy, car_dz));

	return T * Ry;
}
glm::mat4 Headlights(int left_right)
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

	return Car_Body() * T;
}

// 바퀴 변환 - 앞바퀴 회전
glm::mat4 Wheels(int num)
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
	return Car_Body() * T2;
}
glm::mat4 Wheel_rects(int num)
{
	glm::mat4 T = glm::mat4(1.0f);
	glm::mat4 T2 = glm::mat4(1.0f);
	glm::mat4 Ry = glm::mat4(1.0f);

	if (num == 1 || num == 2)
	{
		//앞바퀴들에게 회전 변환 추가 적용
		Ry = glm::rotate(Ry, glm::radians(front_wheels_rotateY), glm::vec3(0.0, 1.0, 0.0));
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
	Rx = glm::rotate(Rx, glm::radians(wheel_rect_rotateX), glm::vec3(1.0, 0.0, 0.0));

	return Car_Body() * T2 * Ry * Rx * T;
}
glm::mat4 Wheel_on_000(int num, int type) //num은 4개 바퀴의 번호, type은 실린더, 뚜껑 객체 종류
{
	glm::mat4 T = glm::mat4(1.0f);
	glm::mat4 Ry = glm::mat4(1.0f);
	glm::mat4 Ry2 = glm::mat4(1.0f);
	//glm::mat4 S = glm::mat4(1.0f);

	Ry = glm::rotate(Ry, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
	if (num == 1 || num == 2)
	{
		//앞바퀴들에게 회전 변환 추가 적용
		Ry2 = glm::rotate(Ry2, glm::radians(front_wheels_rotateY), glm::vec3(0.0, 1.0, 0.0));
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

	return Wheels(num) * Ry2 * Ry * T;
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

std::vector<std::pair<float, float>> Car_GetRotatedCorners()
{
	return Car_GetRotatedCorners(car_dx, car_dz, car_rotateY);
}



float Car_GetDX() { return car_dx; }
float Car_GetDY() { return car_dy; }
float Car_GetDZ() { return car_dz; }
float Car_GetRotationY() { return car_rotateY; }
float Car_GetFrontWheelRotationY() { return front_wheels_rotateY; }
float Car_GetWheelRotationX() { return wheel_rect_rotateX; }
float Car_GetSpeed() { return car_speed; }
bool  Car_IsAcceleratingForward() { return isAcceleratingForward; }
bool  Car_IsAcceleratingBackward() { return isAcceleratingBackward; }
bool  Car_IsBraking() { return isBraking; }

void Car_SetPosition(float dx, float dz)
{
	car_dx = dx;
	car_dz = dz;
}

void Car_SetRotationY(float angle) { car_rotateY = angle; }
void Car_SetFrontWheelRotationY(float angle) { front_wheels_rotateY = angle; }
void Car_SetWheelRotationX(float angle) { wheel_rect_rotateX = angle; }
void Car_SetSpeed(float speed) { car_speed = speed; }
void Car_SetAcceleratingForward(bool status) { isAcceleratingForward = status; }
void Car_SetAcceleratingBackward(bool status) { isAcceleratingBackward = status; }
void Car_SetBraking(bool status) { isBraking = status; }
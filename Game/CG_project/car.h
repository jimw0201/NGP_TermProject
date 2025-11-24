#ifndef CAR_H
#define CAR_H

#include <gl/glm/glm.hpp>
#include <gl/glm/ext.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>
#include <vector>

#include "game_state.h"

// Â÷·® ¼ö
static const int kCarCount = 4;

void Car_Init();

glm::mat4 Car_Body(int carIndex);
glm::mat4 Headlights(int left_right, int carIndex);
glm::mat4 Wheels(int num, int carIndex);
glm::mat4 Wheel_rects(int num, int carIndex);
glm::mat4 Wheel_on_000(int num, int type, int carIndex);

glm::mat4 Car_Body();
glm::mat4 Headlights(int left_right);
glm::mat4 Wheels(int num);
glm::mat4 Wheel_rects(int num);
glm::mat4 Wheel_on_000(int num, int type);

float Car_GetDX(int carIndex);
float Car_GetDY(int carIndex);
float Car_GetDZ(int carIndex);
float Car_GetRotationY(int carIndex);
float Car_GetWheelRotationX(int carIndex);
float Car_GetSpeed(int carIndex);

float Car_GetDX();
float Car_GetDY();
float Car_GetDZ();
float Car_GetRotationY();
float Car_GetWheelRotationX();
float Car_GetSpeed();

float Car_GetFrontWheelRotationY();
bool  Car_IsAcceleratingForward();
bool  Car_IsAcceleratingBackward();
bool  Car_IsBraking();

void Car_SetPosition(int carIndex, float dx, float dz);
void Car_SetRotationY(int carIndex, float angle);
void Car_SetWheelRotationX(int carIndex, float angle);
void Car_SetSpeed(int carIndex, float speed);

void Car_SetPosition(float dx, float dz);
void Car_SetRotationY(float angle);
void Car_SetWheelRotationX(float angle);
void Car_SetSpeed(float speed);

void Car_SetFrontWheelRotationY(float angle);
void Car_SetAcceleratingForward(bool status);
void Car_SetAcceleratingBackward(bool status);
void Car_SetBraking(bool status);

std::vector<std::pair<float, float>> Car_GetRotatedCorners(float x, float z, float angle);
std::vector<std::pair<float, float>> Car_GetRotatedCorners(int carIndex);
std::vector<std::pair<float, float>> Car_GetRotatedCorners();

int Car_Count();

void Car_UpdateSpeed(const CarInput& input, int carIndex);
void Car_UpdateSpeed(const CarInput& input);

#endif

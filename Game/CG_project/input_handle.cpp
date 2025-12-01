#include "input_handle.h"
#include "game_state.h"
#include "car.h"
#include "network_client.h"
#include <iostream>
#include <cmath>
#include <algorithm> 
#include <ctime>     

#define M_PI 3.14159265358979323846

static int lastMouseX = -1, lastMouseY = -1;
static bool is_mouse_on_camera = false;
static bool is_mouse_on_handle = false;

// 핸들 관련
static float handle_rotateZ = 0.0f;
static float lastAngle = 0.0f;
static float cumulativeAngle = 0.0f;
static const float HANDLE_RETURN_SPEED = 3.0f;

// 카메라 관련
static float c_dx = 0.0f;
static float c_dy = 1.0f;
static float c_dz = -3.0f;
static float c_rotateY = 0.0f;

static bool g_keyW = false;
static bool g_keySpace = false;
static bool g_keyQ = false;
static bool g_keyE = false;
static bool g_keyP = false;

bool Input_IsKeyWDown() { return g_keyW; }
bool Input_IsKeySpaceDown() { return g_keySpace; }
bool Input_IsKeyQDown() { return g_keyQ; }
bool Input_IsKeyEDown() { return g_keyE; }


void Input_Init()
{
    c_dx = 0.0f;
    c_dy = 1.0f;
    c_dz = -3.0f;
    c_rotateY = 0.0f;
    Input_ResetHandle();
}

void Input_ResetHandle()
{
    handle_rotateZ = 0.0f;
    lastAngle = 0.0f;
    cumulativeAngle = 0.0f;
    is_mouse_on_handle = false;
    is_mouse_on_camera = false;
}

// --- Getters ---
float Input_GetCameraDX() { return c_dx; }
float Input_GetCameraDY() { return c_dy; }
float Input_GetCameraDZ() { return c_dz; }
float Input_GetCameraRotateY() { return c_rotateY; }
float Input_GetHandleRotation() { return handle_rotateZ; }
bool  Input_IsMouseOnHandle() { return is_mouse_on_handle; }


// 핸들 복원 로직
void Input_UpdateHandleReturn()
{
    if (!is_mouse_on_handle)
    {
        if (handle_rotateZ > 0.0f)
        {
            handle_rotateZ = std::max(0.0f, handle_rotateZ - HANDLE_RETURN_SPEED);
        }
        else if (handle_rotateZ < 0.0f)
        {
            handle_rotateZ = std::min(0.0f, handle_rotateZ + HANDLE_RETURN_SPEED);
        }
        cumulativeAngle = handle_rotateZ;

        // 바퀴 회전 동기화
        if (!Network_IsConnected())
        {
            Car_SetFrontWheelRotationY((handle_rotateZ / 900.0f) * 30.0f);
        }
    }
}

void Keyboard(unsigned char key, int x, int y)
{
    if (GameState_IsClear())
    {
        if (key == 'n')
        {
            GameState_NextStage(); // 다음 스테이지로 이동
        }
    }
    if (!GameState_IsClear())
    {
        if (key == 27) // ESC
        {
            if (GameState_IsPaused())
            {
                GameState_UpdatePauseTime(GameState_GetPauseTime() + time(nullptr) - GameState_GetTempTime());
                GameState_SetPaused(false);
            }
            else
            {
                GameState_UpdateTempTime(time(nullptr));
                GameState_SetPaused(true);
            }
        }
        if (key == 'i')
        {
            if (GameState_IsInvincible())
            {
                GameState_SetInvincible(false);
                std::cout << "Debug: Invincible Mode OFF" << std::endl;
            }
            else
            {
                GameState_SetInvincible(true);
                std::cout << "Debug: Invincible Mode ON" << std::endl;
            }
        }
    }

    if (!GameState_IsPaused())
    {
        switch (key)
        {
        case 'q':
            g_keyQ = true;
            if (GameState_GetCurrentGear() > PARK)
                GameState_SetCurrentGear(static_cast<GearState>(GameState_GetCurrentGear() - 1));
            if (GameState_GetCurrentGear() == PARK && GameState_IsParked())
            {
                GameState_SetPaused(true);
                GameState_SetClear(true);
            }
            break;
        case 'e':
            g_keyE = true;
            if (GameState_GetCurrentGear() < DRIVE)
                GameState_SetCurrentGear(static_cast<GearState>(GameState_GetCurrentGear() + 1));
            break;
        case 'w':
            g_keyW = true;
            if (GameState_GetCurrentGear() == DRIVE) Car_SetAcceleratingForward(true);
            else if (GameState_GetCurrentGear() == REVERSE) Car_SetAcceleratingBackward(true);
            break;
        case ' ':
            g_keySpace = true;
            Car_SetBraking(true);
            break;
        }
    }
    glutPostRedisplay();
}

void KeyboardUp(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'w':
        g_keyW = false;
        Car_SetAcceleratingForward(false);
        Car_SetAcceleratingBackward(false);
        break;
    case ' ':
        g_keySpace = false;
        Car_SetBraking(false);
        break;
    case 'q':
        g_keyQ = false;
        break;
    case 'e':
        g_keyE = false;
        break;
    }
    glutPostRedisplay();
}

void MouseButton(int button, int state, int x, int y)
{
    if (!GameState_IsPaused())
    {
        if (button == GLUT_LEFT_BUTTON)
        {
            if (state == GLUT_DOWN)
            {
                if (x > 600 && y > 300) // 핸들 영역
                {
                    lastAngle = 0.0f;
                    is_mouse_on_handle = true;
                }
                else
                {
                    is_mouse_on_camera = true;
                    lastMouseX = x;
                }
            }
            else if (state == GLUT_UP)
            {
                if (is_mouse_on_handle) { lastAngle = 0.0f; is_mouse_on_handle = false; }
                if (is_mouse_on_camera) { is_mouse_on_camera = false; lastMouseX = -1; }
            }
        }
        else if (button == 3) { c_dz -= 0.1f; glutPostRedisplay(); }
        else if (button == 4) { c_dz += 0.1f; glutPostRedisplay(); }
    }
}

void MouseMotion(int x, int y)
{
    if (is_mouse_on_handle)
    {
        int dx = x - 750;
        int dy = y - 450;
        float currentAngle = float(-atan2(dx, -dy)) * (180.0f / M_PI);
        float deltaAngle = currentAngle - lastAngle;

        if (deltaAngle > 180.0f) deltaAngle -= 360.0f;
        else if (deltaAngle < -180.0f) deltaAngle += 360.0f;

        if (-900.0f <= handle_rotateZ && handle_rotateZ <= 900.0f)
        {
            cumulativeAngle += deltaAngle;
            // 클램핑
            if (cumulativeAngle > 900.0f) cumulativeAngle = 900.0f;
            else if (cumulativeAngle < -900.0f) cumulativeAngle = -900.0f;
            handle_rotateZ = cumulativeAngle;
        }
        lastAngle = currentAngle;

        if (!Network_IsConnected())
        {
            Car_SetFrontWheelRotationY((handle_rotateZ / 900.0f) * 30.0f);
        }
    }
    if (is_mouse_on_camera)
    {
        if (lastMouseX == -1) { lastMouseX = x; return; }
        int dx = x - lastMouseX;
        c_rotateY += dx * 0.1f;
        lastMouseX = x;
    }
    glutPostRedisplay();
}
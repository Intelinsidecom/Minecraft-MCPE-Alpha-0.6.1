#include "SDL2Controller.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "../../platform/log.h"
#include "../../client/player/input/SDL2TurnInput.h"
#include "../../client/player/input/SDL2MoveInput.h"
#include "../../client/gui/screens/crafting/WorkbenchScreen.h"
#include "../../world/item/crafting/Recipe.h"
#include "../../client/Minecraft.h"
#include "../../App.h"

#include <SDL2/SDL.h>
#include <windows.h>
extern "C" void SDL_SetMainReady(void);


extern App* g_app;

bool g_controllerConnected = false;
extern float g_leftStickX;
extern float g_leftStickY;
extern float g_rightStickX;
extern float g_rightStickY;
extern bool g_rightStickPressed;
bool g_controllerAPressed = false;
bool g_controllerBPressed = false;
bool g_controllerXPressed = false;
bool g_controllerYPressed = false;
static int s_lastPlaceTick = 0;
static const int PLACE_COOLDOWN_TICKS = 4;

bool SDL2Controller::_initialized = false;
void* SDL2Controller::_gameController = nullptr;
int SDL2Controller::_controllerId = -1;
int SDL2Controller::_tickCounter = 0;

void SDL2Controller::tick()
{
    _tickCounter++;
}

bool SDL2Controller::init()
{
    if (_initialized)
    {
        return true;
    }

    SDL_SetMainReady();

    if (SDL_Init(SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK) < 0)
    {
        LOGW("SDL2Controller: Failed to initialize SDL GameController: %s", SDL_GetError());
        return false;
    }

    _initialized = true;
    LOGI("SDL2Controller: Initialized successfully");

    poll();
    return true;
}

void SDL2Controller::shutdown()
{
    if (_gameController)
    {
        SDL_GameControllerClose((SDL_GameController*)_gameController);
        _gameController = nullptr;
        _controllerId = -1;
    }

    if (_initialized)
    {
        SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK);
        _initialized = false;
    }
}

bool SDL2Controller::isControllerConnected()
{
    return _gameController != nullptr;
}

void SDL2Controller::poll()
{
    static int s_startupRetryCount = 0;
    static bool s_startupComplete = false;

    if (!_initialized)
    {
        return;
    }

    SDL_GameControllerUpdate();

    if (!_gameController)
    {
        int numJoysticks = SDL_NumJoysticks();
        for (int i = 0; i < numJoysticks; i++)
        {
            if (SDL_IsGameController(i))
            {
                _gameController = SDL_GameControllerOpen(i);
                if (_gameController)
                {
                    _controllerId = i;
                    g_controllerConnected = true;
                    s_startupComplete = true;
                    LOGI("SDL2Controller: Connected controller %d: %s", i, SDL_GameControllerName((SDL_GameController*)_gameController));
                    break;
                }
            }
        }

        if (!_gameController && !s_startupComplete && s_startupRetryCount < 180)
        {
            s_startupRetryCount++;
            if (s_startupRetryCount % 30 == 0)
            {
                SDL_JoystickUpdate();
            }
        }
        else if (s_startupRetryCount >= 180)
        {
            s_startupComplete = true;
        }
    }
    else
    {
        if (!SDL_GameControllerGetAttached((SDL_GameController*)_gameController))
        {
            LOGI("SDL2Controller: Controller disconnected");
            SDL_GameControllerClose((SDL_GameController*)_gameController);
            _gameController = nullptr;
            _controllerId = -1;
            g_controllerConnected = false;
            
            Controller::feed(0, Controller::STATE_RELEASE, 0.0f, 0.0f);
            Controller::feed(1, Controller::STATE_RELEASE, 0.0f, 0.0f);
            return;
        }
    }

    if (!_gameController)
    {
        return;
    }



    Sint16 leftX = SDL_GameControllerGetAxis((SDL_GameController*)_gameController, SDL_CONTROLLER_AXIS_LEFTX);
    Sint16 leftY = SDL_GameControllerGetAxis((SDL_GameController*)_gameController, SDL_CONTROLLER_AXIS_LEFTY);
    Sint16 rightX = SDL_GameControllerGetAxis((SDL_GameController*)_gameController, SDL_CONTROLLER_AXIS_RIGHTX);
    Sint16 rightY = SDL_GameControllerGetAxis((SDL_GameController*)_gameController, SDL_CONTROLLER_AXIS_RIGHTY);
    Sint16 triggerLeft = SDL_GameControllerGetAxis((SDL_GameController*)_gameController, SDL_CONTROLLER_AXIS_TRIGGERLEFT);
    Sint16 triggerRight = SDL_GameControllerGetAxis((SDL_GameController*)_gameController, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);

    float normLeftX = normalizeAxis(leftX);
    float normLeftY = normalizeAxis(leftY);
    float normRightX = normalizeAxis(rightX);
    float normRightY = normalizeAxis(rightY);

    const float deadzone = 0.1f;
    
    if (normLeftX < deadzone && normLeftX > -deadzone) normLeftX = 0.0f;
    if (normLeftY < deadzone && normLeftY > -deadzone) normLeftY = 0.0f;
    if (normRightX < deadzone && normRightX > -deadzone) normRightX = 0.0f;
    if (normRightY < deadzone && normRightY > -deadzone) normRightY = 0.0f;

    int leftState = (normLeftX != 0.0f || normLeftY != 0.0f) ? Controller::STATE_MOVE : Controller::STATE_RELEASE;
    int rightState = (normRightX != 0.0f || normRightY != 0.0f) ? Controller::STATE_MOVE : Controller::STATE_RELEASE;


    g_leftStickX = normLeftX;
    g_leftStickY = -normLeftY;
    g_rightStickX = normRightX;
    g_rightStickY = -normRightY;
    SDL2TurnInput::feedCamera(normRightX, -normRightY);
    Controller::feed(0, leftState, normLeftX, -normLeftY);
    g_rightStickPressed = SDL_GameControllerGetButton((SDL_GameController*)_gameController, SDL_CONTROLLER_BUTTON_RIGHTSTICK);
    static bool wasAPressed = false;
    bool isAPressed = SDL_GameControllerGetButton((SDL_GameController*)_gameController, SDL_CONTROLLER_BUTTON_A);
    g_controllerAPressed = isAPressed;
    if (isAPressed && !wasAPressed)
        Keyboard::feed(Keyboard::KEY_SPACE, 1);
    else if (!isAPressed && wasAPressed)
        Keyboard::feed(Keyboard::KEY_SPACE, 0);
    wasAPressed = isAPressed;
    static bool wasBPressed = false;
    bool isBPressed = SDL_GameControllerGetButton((SDL_GameController*)_gameController, SDL_CONTROLLER_BUTTON_B);
    g_controllerBPressed = isBPressed;
    if (isBPressed && !wasBPressed)
        Keyboard::feed(Keyboard::KEY_ESCAPE, 1);
    else if (!isBPressed && wasBPressed)
        Keyboard::feed(Keyboard::KEY_ESCAPE, 0);
    wasBPressed = isBPressed;
    static bool wasXPressed = false;
    bool isXPressed = SDL_GameControllerGetButton((SDL_GameController*)_gameController, SDL_CONTROLLER_BUTTON_X);
    g_controllerXPressed = isXPressed;
    if (isXPressed && !wasXPressed)
        Keyboard::feed(Keyboard::KEY_E, 1);
    else if (!isXPressed && wasXPressed)
        Keyboard::feed(Keyboard::KEY_E, 0);
    wasXPressed = isXPressed;

    static bool wasYPressed = false;
    bool isYPressed = SDL_GameControllerGetButton((SDL_GameController*)_gameController, SDL_CONTROLLER_BUTTON_Y);
    g_controllerYPressed = isYPressed;
    if (isYPressed && !wasYPressed)
    {
        if (g_app)
        {
            Minecraft* minecraft = static_cast<Minecraft*>(g_app);
            if (!minecraft->isCreativeMode())
            {
                if (minecraft->screen)
                    minecraft->setScreen(NULL);
                else
                    minecraft->setScreen(new WorkbenchScreen(Recipe::SIZE_2X2));
            }
        }
    }
    wasYPressed = isYPressed;
    static bool wasRightTriggerPressed = false;
    bool isRightTriggerPressed = triggerRight > 16000;
    if (isRightTriggerPressed && !wasRightTriggerPressed)
        Mouse::feed(MouseAction::ACTION_LEFT, 1, 0, 0);
    else if (!isRightTriggerPressed && wasRightTriggerPressed)
        Mouse::feed(MouseAction::ACTION_LEFT, 0, 0, 0);
    wasRightTriggerPressed = isRightTriggerPressed;
    static bool wasLeftTriggerPressed = false;
    static int leftTriggerTimer = 0;
    static bool leftTriggerActive = false;
    const int TRIGGER_THRESHOLD = 16000;
    const int PULSE_DURATION = 2;
    const int PLACE_COOLDOWN = 12;

    bool isLeftTriggerPressed = triggerLeft > TRIGGER_THRESHOLD;

    if (!leftTriggerActive && leftTriggerTimer > 0)
        leftTriggerTimer--;
    if (isLeftTriggerPressed && !wasLeftTriggerPressed && leftTriggerTimer == 0)
    {
        Mouse::feed(MouseAction::ACTION_RIGHT, 1, 0, 0);
        leftTriggerActive = true;
        leftTriggerTimer = PULSE_DURATION;
    }
    else if (leftTriggerActive && (--leftTriggerTimer <= 0 || !isLeftTriggerPressed))
    {
        Mouse::feed(MouseAction::ACTION_RIGHT, 0, 0, 0);
        leftTriggerActive = false;
        leftTriggerTimer = PLACE_COOLDOWN;
    }

    wasLeftTriggerPressed = isLeftTriggerPressed;

    static bool wasLBPressed = false;
    bool isLBPressed = SDL_GameControllerGetButton((SDL_GameController*)_gameController, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
    if (isLBPressed && !wasLBPressed) {
        Mouse::feed(MouseAction::ACTION_WHEEL, 1, 0, 0, 0, 1);
    }
    wasLBPressed = isLBPressed;

    static bool wasRBPressed = false;
    bool isRBPressed = SDL_GameControllerGetButton((SDL_GameController*)_gameController, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
    if (isRBPressed && !wasRBPressed) {
        Mouse::feed(MouseAction::ACTION_WHEEL, 1, 0, 0, 0, -1);
    }
    wasRBPressed = isRBPressed;

    if (SDL_GameControllerGetButton((SDL_GameController*)_gameController, SDL_CONTROLLER_BUTTON_DPAD_UP))
        Keyboard::feed(Keyboard::KEY_W, 1);
    else
        Keyboard::feed(Keyboard::KEY_W, 0);

    if (SDL_GameControllerGetButton((SDL_GameController*)_gameController, SDL_CONTROLLER_BUTTON_DPAD_DOWN))
        Keyboard::feed(Keyboard::KEY_S, 1);
    else
        Keyboard::feed(Keyboard::KEY_S, 0);

    if (SDL_GameControllerGetButton((SDL_GameController*)_gameController, SDL_CONTROLLER_BUTTON_DPAD_LEFT))
        Keyboard::feed(Keyboard::KEY_A, 1);
    else
        Keyboard::feed(Keyboard::KEY_A, 0);

    if (SDL_GameControllerGetButton((SDL_GameController*)_gameController, SDL_CONTROLLER_BUTTON_DPAD_RIGHT))
        Keyboard::feed(Keyboard::KEY_D, 1);
    else
        Keyboard::feed(Keyboard::KEY_D, 0);

    static bool wasStartPressed = false;
    bool isStartPressed = SDL_GameControllerGetButton((SDL_GameController*)_gameController, SDL_CONTROLLER_BUTTON_START);
    if (isStartPressed && !wasStartPressed)
        Keyboard::feed(Keyboard::KEY_ESCAPE, 1);
    else if (!isStartPressed && wasStartPressed)
        Keyboard::feed(Keyboard::KEY_ESCAPE, 0);
    wasStartPressed = isStartPressed;

    static bool wasBackPressed = false;
    bool isBackPressed = SDL_GameControllerGetButton((SDL_GameController*)_gameController, SDL_CONTROLLER_BUTTON_BACK);
    if (isBackPressed && !wasBackPressed)
        Keyboard::feed(Keyboard::KEY_TAB, 1);
    else if (!isBackPressed && wasBackPressed)
        Keyboard::feed(Keyboard::KEY_TAB, 0);
    wasBackPressed = isBackPressed;

    static bool wasLSPressed = false;
    bool isLSPressed = SDL_GameControllerGetButton((SDL_GameController*)_gameController, SDL_CONTROLLER_BUTTON_LEFTSTICK);
    if (isLSPressed && !wasLSPressed)
        Keyboard::feed(Keyboard::KEY_LSHIFT, 1);
    else if (!isLSPressed && wasLSPressed)
        Keyboard::feed(Keyboard::KEY_LSHIFT, 0);
    wasLSPressed = isLSPressed;

    static bool wasRSPressed = false;
    bool isRSPressed = SDL_GameControllerGetButton((SDL_GameController*)_gameController, SDL_CONTROLLER_BUTTON_RIGHTSTICK);
    if (isRSPressed && !wasRSPressed)
        Keyboard::feed(Keyboard::KEY_F5, 1);
    else if (!isRSPressed && wasRSPressed)
        Keyboard::feed(Keyboard::KEY_F5, 0);
    wasRSPressed = isRSPressed;

    static bool hotbarButtonPressed[9] = {false};
    
    if (SDL_GameControllerGetButton((SDL_GameController*)_gameController, SDL_CONTROLLER_BUTTON_GUIDE))
    {
        if (!hotbarButtonPressed[0])
        {
            Keyboard::feed('1', 1);
            hotbarButtonPressed[0] = true;
        }
    }
    else
    {
        if (hotbarButtonPressed[0])
        {
            Keyboard::feed('1', 0);
            hotbarButtonPressed[0] = false;
        }
    }
}

float SDL2Controller::normalizeAxis(short value)
{
    return value / 32768.0f;
}

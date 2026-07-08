#include "ControllerNavigation.h"
#include "components/GuiElement.h"
#include "components/Button.h"
#include "Screen.h"
#include "../../client/Minecraft.h"
#include "../../platform/input/Mouse.h"
#include "../../platform/input/Keyboard.h"
#include "../../platform/log.h"
#include <cmath>

ControllerNavigation g_controllerNavigation;

ControllerNavigation::ControllerNavigation()
    : _focusedIndex(-1)
    , _screenWidth(0)
    , _screenHeight(0)
    , _guiMode(false)
    , _navRepeatTimer(0)
    , _lastNavTick(0)
    , _tickCounter(0)
    , _lastLeftStickX(0)
    , _lastLeftStickY(0)
{
    for (int i = 0; i < NAV_COUNT; i++)
        _navActive[i] = false;
}

ControllerNavigation::~ControllerNavigation()
{
}

void ControllerNavigation::init(int screenWidth, int screenHeight)
{
    _screenWidth = screenWidth;
    _screenHeight = screenHeight;
    _focusedIndex = -1;
    _guiMode = true;
    clearElements();
}

void ControllerNavigation::registerElement(GuiElement* element)
{
    if (element && element->active && element->visible)
    {
        _elements.push_back(element);
        // If this is the first element, focus it
        if (_focusedIndex < 0 && !_elements.empty())
        {
            _focusedIndex = 0;
            updateMousePosition(nullptr);
            updateButtonSelection();
        }
    }
}

void ControllerNavigation::registerButton(Button* button)
{
    if (button && button->active && button->visible)
    {
        int elementIndex = (int)_elements.size();
        _buttons.push_back(button);
        _buttonElementIndices.push_back(elementIndex);
        registerElement(button);
    }
}

void ControllerNavigation::clearElements()
{
    _elements.clear();
    _buttons.clear();
    _buttonElementIndices.clear();
    _focusedIndex = -1;
}

GuiElement* ControllerNavigation::getFocusedElement() const
{
    if (_focusedIndex >= 0 && _focusedIndex < (int)_elements.size())
        return _elements[_focusedIndex];
    return nullptr;
}

void ControllerNavigation::updateButtonSelection()
{
    // Update selected state on all buttons
    // Check if focused index matches any button's element index
    for (int i = 0; i < (int)_buttons.size(); i++)
    {
        if (_buttons[i])
        {
            _buttons[i]->selected = (_buttonElementIndices[i] == _focusedIndex);
        }
    }
}

void ControllerNavigation::handleNavigation(ControllerNavDirection direction)
{
    if (_elements.empty())
        return;

    int newIndex = findNearestElement(direction);
    if (newIndex >= 0 && newIndex != _focusedIndex)
    {
        _focusedIndex = newIndex;
        updateMousePosition(nullptr);
        updateButtonSelection();
    }
}

void ControllerNavigation::handleSelect(Minecraft* minecraft)
{
    if (_focusedIndex < 0 || _focusedIndex >= (int)_elements.size())
        return;
    if (!minecraft || !minecraft->screen)
        return;

    GuiElement* element = _elements[_focusedIndex];
    if (!element || !element->active || !element->visible)
        return;

    // Get element center position in screen space
    int screenX = element->x + element->width / 2;
    int screenY = element->y + element->height / 2;
    
    // Convert screen coordinates to window coordinates
    // Screen::mouseEvent transforms: xm = e.x * width / minecraft->width
    // So we need: e.x = xm * minecraft->width / width
    int windowX = screenX * minecraft->width / minecraft->screen->width;
    int windowY = (screenY + 1) * minecraft->height / minecraft->screen->height;

    // Move mouse to element position first (for hover effect)
    Mouse::feed(MouseAction::ACTION_MOVE, 0, windowX, windowY);
    
    // Simulate mouse click at element position
    // This will trigger the button through Screen's normal mouse handling
    Mouse::feed(MouseAction::ACTION_LEFT, 1, windowX, windowY);
    Mouse::feed(MouseAction::ACTION_LEFT, 0, windowX, windowY);
}

void ControllerNavigation::handleBack(Minecraft* minecraft)
{
    // Simulate Escape key press
    Keyboard::feed(Keyboard::KEY_ESCAPE, 1);
    Keyboard::feed(Keyboard::KEY_ESCAPE, 0);
}

void ControllerNavigation::handleScroll(int scrollAmount)
{
    // Simulate mouse wheel scroll
    Mouse::feed(MouseAction::ACTION_WHEEL, 1, 0, 0, 0, scrollAmount);
}

void ControllerNavigation::tick(Minecraft* minecraft, float leftStickX, float leftStickY, 
                                float rightStickY, bool aPressed, bool bPressed,
                                bool wasAPressed, bool wasBPressed)
{
    _tickCounter++;

    if (!_guiMode || _elements.empty())
        return;

    // Handle navigation with left stick
    bool navUp = leftStickY > NAV_DEADZONE / 100.0f;
    bool navDown = leftStickY < -NAV_DEADZONE / 100.0f;
    bool navLeft = leftStickX < -NAV_DEADZONE / 100.0f;
    bool navRight = leftStickX > NAV_DEADZONE / 100.0f;

    // Check if stick just moved in a direction (for initial press)
    bool navJustUp = navUp && _lastLeftStickY <= NAV_DEADZONE / 100.0f;
    bool navJustDown = navDown && _lastLeftStickY >= -NAV_DEADZONE / 100.0f;
    bool navJustLeft = navLeft && _lastLeftStickX >= -NAV_DEADZONE / 100.0f;
    bool navJustRight = navRight && _lastLeftStickX <= NAV_DEADZONE / 100.0f;

    if (navJustUp)
    {
        handleNavigation(NAV_UP);
        _navRepeatTimer = _tickCounter + NAV_REPEAT_DELAY;
    }
    else if (navJustDown)
    {
        handleNavigation(NAV_DOWN);
        _navRepeatTimer = _tickCounter + NAV_REPEAT_DELAY;
    }
    else if (navJustLeft)
    {
        handleNavigation(NAV_LEFT);
        _navRepeatTimer = _tickCounter + NAV_REPEAT_DELAY;
    }
    else if (navJustRight)
    {
        handleNavigation(NAV_RIGHT);
        _navRepeatTimer = _tickCounter + NAV_REPEAT_DELAY;
    }
    // Handle repeat for held directions
    else if (_tickCounter >= _navRepeatTimer)
    {
        if (navUp)
            handleNavigation(NAV_UP);
        else if (navDown)
            handleNavigation(NAV_DOWN);
        else if (navLeft)
            handleNavigation(NAV_LEFT);
        else if (navRight)
            handleNavigation(NAV_RIGHT);
        
        if (navUp || navDown || navLeft || navRight)
            _navRepeatTimer = _tickCounter + NAV_REPEAT_RATE;
    }

    // Handle A button press (select)
    if (aPressed && !wasAPressed)
    {
        handleSelect(minecraft);
    }

    // Handle B button press (back)
    if (bPressed && !wasBPressed)
    {
        handleBack(minecraft);
    }

    // Handle scroll with right stick Y
    if (rightStickY > 0.3f)
    {
        if (_tickCounter % 4 == 0) // Scroll every 4 ticks
            handleScroll(-1); // Scroll down
    }
    else if (rightStickY < -0.3f)
    {
        if (_tickCounter % 4 == 0) // Scroll every 4 ticks
            handleScroll(1); // Scroll up
    }

    // Update mouse position to follow focused element
    updateMousePosition(minecraft);

    // Store last stick positions
    _lastLeftStickX = leftStickX;
    _lastLeftStickY = leftStickY;
}

void ControllerNavigation::updateMousePosition(Minecraft* minecraft)
{
    if (_focusedIndex < 0 || _focusedIndex >= (int)_elements.size())
        return;

    GuiElement* element = _elements[_focusedIndex];
    if (!element)
        return;

    // Get element center position in screen space
    int screenX = element->x + element->width / 2;
    int screenY = element->y + element->height / 2;
    
    // If we have minecraft context, convert to window coordinates
    // Screen::mouseEvent transforms: xm = e.x * width / minecraft->width
    // So we need: e.x = xm * minecraft->width / width
    int windowX = screenX;
    int windowY = screenY;
    if (minecraft && minecraft->screen)
    {
        windowX = screenX * minecraft->width / minecraft->screen->width;
        windowY = (screenY + 1) * minecraft->height / minecraft->screen->height;
    }

    Mouse::feed(MouseAction::ACTION_MOVE, 0, windowX, windowY);
}

float ControllerNavigation::getDistanceToElement(int elementIndex, ControllerNavDirection direction)
{
    if (elementIndex < 0 || elementIndex >= (int)_elements.size())
        return -1.0f;

    GuiElement* current = getFocusedElement();
    GuiElement* target = _elements[elementIndex];

    if (!current || !target || current == target)
        return -1.0f;

    float currentCX = current->x + current->width / 2.0f;
    float currentCY = current->y + current->height / 2.0f;
    float targetCX = target->x + target->width / 2.0f;
    float targetCY = target->y + target->height / 2.0f;

    float dx = targetCX - currentCX;
    float dy = targetCY - currentCY;

    // Check if element is in the correct direction
    switch (direction)
    {
        case NAV_UP:
            if (dy >= 0) return -1.0f; // Must be above
            break;
        case NAV_DOWN:
            if (dy <= 0) return -1.0f; // Must be below
            break;
        case NAV_LEFT:
            if (dx >= 0) return -1.0f; // Must be to the left
            break;
        case NAV_RIGHT:
            if (dx <= 0) return -1.0f; // Must be to the right
            break;
        default:
            return -1.0f;
    }

    // Return Euclidean distance
    return std::sqrt(dx * dx + dy * dy);
}

int ControllerNavigation::findNearestElement(ControllerNavDirection direction)
{
    if (_elements.empty() || _focusedIndex < 0)
        return -1;

    float bestDistance = -1.0f;
    int bestIndex = -1;

    for (int i = 0; i < (int)_elements.size(); i++)
    {
        if (i == _focusedIndex)
            continue;

        float dist = getDistanceToElement(i, direction);
        if (dist >= 0.0f && (bestIndex < 0 || dist < bestDistance))
        {
            bestDistance = dist;
            bestIndex = i;
        }
    }

    // If no element found in direction, wrap around to find closest in that general direction
    if (bestIndex < 0)
    {
        for (int i = 0; i < (int)_elements.size(); i++)
        {
            if (i == _focusedIndex)
                continue;

            float dist = getDistanceToElement(i, direction);
            if (dist >= 0.0f && (bestIndex < 0 || dist < bestDistance))
            {
                bestDistance = dist;
                bestIndex = i;
            }
        }
    }

    return bestIndex >= 0 ? bestIndex : _focusedIndex;
}

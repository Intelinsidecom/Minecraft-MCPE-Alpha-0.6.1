#ifndef NET_MINECRAFT_CLIENT_GUI__ControllerNavigation_H__
#define NET_MINECRAFT_CLIENT_GUI__ControllerNavigation_H__

#include <vector>

class GuiElement;
class Button;
class Minecraft;
class Screen;

// Navigation directions for controller input
enum ControllerNavDirection {
    NAV_UP = 0,
    NAV_DOWN,
    NAV_LEFT,
    NAV_RIGHT,
    NAV_COUNT
};

// Manages controller-based navigation for GUI screens
// Uses invisible focus - elements show hover state when focused
class ControllerNavigation
{
public:
    ControllerNavigation();
    ~ControllerNavigation();

    // Initialize with screen dimensions
    void init(int screenWidth, int screenHeight);

    // Register a focusable element (buttons, slots, etc.)
    void registerElement(GuiElement* element);
    void registerButton(Button* button);
    void clearElements();

    // Handle navigation input
    void handleNavigation(ControllerNavDirection direction);
    void handleSelect(Minecraft* minecraft);
    void handleBack(Minecraft* minecraft);
    void handleScroll(int scrollAmount);

    // Check if A button should be handled by navigation (true) or game (false)
    bool isInGUIMode() const { return _guiMode; }
    void setGUIMode(bool guiMode) { _guiMode = guiMode; }

    // Get currently focused element
    int getFocusedIndex() const { return _focusedIndex; }
    GuiElement* getFocusedElement() const;

    // Update - called every tick to handle stick input with repeat delay
    void tick(Minecraft* minecraft, float leftStickX, float leftStickY, float rightStickY,
              bool aPressed, bool bPressed, bool wasAPressed, bool wasBPressed);

    // Set mouse position to center of focused element (for hover effect)
    void updateMousePosition(Minecraft* minecraft);

    // Update button selection states for visual feedback
    void updateButtonSelection();

private:
    std::vector<GuiElement*> _elements;
    std::vector<Button*> _buttons;
    std::vector<int> _buttonElementIndices;  // Maps button index to element index
    int _focusedIndex;
    int _screenWidth;
    int _screenHeight;
    bool _guiMode;

    // Input repeat handling (like console edition's 300ms initial, 100ms repeat)
    int _navRepeatTimer;
    int _lastNavTick;
    static const int NAV_REPEAT_DELAY = 12;  // ~300ms at 40 ticks/sec
    static const int NAV_REPEAT_RATE = 4;    // ~100ms at 40 ticks/sec
    static const int NAV_DEADZONE = 15;    // Stick threshold for navigation (out of 100)

    int _tickCounter;

    // Find nearest element in a direction from current focus
    int findNearestElement(ControllerNavDirection direction);
    float getDistanceToElement(int elementIndex, ControllerNavDirection direction);
    bool isElementInDirection(int elementIndex, ControllerNavDirection direction);

    // Navigation state
    bool _navActive[NAV_COUNT];
    float _lastLeftStickX;
    float _lastLeftStickY;
};

// Global navigation instance (singleton for simplicity)
extern ControllerNavigation g_controllerNavigation;

#endif /*NET_MINECRAFT_CLIENT_GUI__ControllerNavigation_H__*/

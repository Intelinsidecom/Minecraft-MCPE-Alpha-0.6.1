#ifndef APPPLATFORM_UWP_H__
#define APPPLATFORM_UWP_H__

#include "AppPlatform.h"
#include "platform/log.h"
#include "client/renderer/gles.h"
#include <EGL/egl.h>
#include "world/level/storage/FolderMethods.h"
#include <png.h>
#include <cmath>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <mutex>

// Forward declarations for EGL types if not already defined
#ifndef EGL_EGLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES 1
#endif

// UWP Headers
#include <wrl/client.h>
#include <wrl/wrappers/corewrappers.h>
#include <windows.foundation.h>
#include <windows.applicationmodel.core.h>
#include <windows.ui.core.h>
#include <windows.ui.viewmanagement.h>
#include <windows.graphics.display.h>
#include <windows.storage.h>
#include <windows.storage.streams.h>
#include <windows.system.threading.h>
#include <windows.devices.input.h>
#include <wrl/event.h>

using namespace Windows::ApplicationModel::Core;
using namespace Windows::UI::Core;
using namespace Windows::UI::ViewManagement;
using namespace Windows::Graphics::Display;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace Windows::Foundation;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;

// Input profile - determined once at initialization, never changes at runtime
// This eliminates the need for expensive isMobileDevice() checks on every input event
enum class InputProfile {
    Desktop_MouseKeyboard,  // Desktop PC with mouse/keyboard (Win32-style immediate input)
    Desktop_Touch,          // Desktop with touchscreen (hybrid)
    Mobile_TouchOnly,       // Phone/tablet with only touch (buffered input)
    Mobile_WithMouse        // Phone/tablet with physical mouse (hybrid)
};

class AppPlatform_uwp : public AppPlatform
{
public:
    AppPlatform_uwp();
    virtual ~AppPlatform_uwp();

    // UWP-specific initialization
    void setCoreWindow(CoreWindow^ window);
    void setDisplayInfo(DisplayInformation^ info);
    void initializeEGL();
    void registerInputHandlers();
    
    // Input profile - cached at initialization for fast access
    InputProfile getInputProfile() const { return _inputProfile; }
    bool isDesktopProfile() const { return _inputProfile == InputProfile::Desktop_MouseKeyboard || _inputProfile == InputProfile::Desktop_Touch; }

    // Override base platform methods
    virtual void grabMouse() override;
    virtual void releaseMouse() override;
    virtual bool isMouseGrabbed() const;

    virtual BinaryBlob readAssetFile(const std::string& filename) override;
    virtual TextureData loadTexture(const std::string& filename_, bool textureFolder) override;
    virtual void saveScreenshot(const std::string& filename, int glWidth, int glHeight) override;

    virtual int getScreenWidth() override;
    virtual int getScreenHeight() override;
    virtual float getPixelsPerMillimeter() override;

    virtual bool supportsTouchscreen() override;
    virtual bool supportsNonTouchscreen() override;
    virtual bool hasBuyButtonWhenInvalidLicense() override;
    virtual int checkLicense() override;
    
    // UWP-specific device detection
    bool isMobileDevice() const;
    bool isTabletDevice() const;

    virtual std::string getDateString(int s) override;

    // UWP-specific methods
    CoreWindow^ getCoreWindow() const { return _coreWindow; }
    DisplayInformation^ getDisplayInfo() const { return _displayInfo; }
    EGLDisplay getEGLDisplay() const { return _eglDisplay; }
    EGLSurface getEGLSurface() const { return _eglSurface; }
    EGLContext getEGLContext() const { return _eglContext; }
    
    // Game initialization state
    void setGameInitialized(bool initialized) { _gameInitialized = initialized; }
    bool isGameInitialized() const { return _gameInitialized; }
    
    std::string getLocalStoragePath() { return wstringToStd(_localFolderPath); }
    std::string getTempStoragePath() { return wstringToStd(_tempFolderPath); }
    std::string getPackagePath() { return wstringToStd(_packagePath); }

    std::mutex& getGameMutex() { return _gameMutex; }

    // UWP Input Buffering - process input on game thread instead of UI thread
    void processBufferedInput();  // Call this from game thread
    void clearInputBuffer();      // Clear pending input events

    // Input event handlers
    void onPointerPressed(PointerEventArgs^ args);
    void onPointerMoved(PointerEventArgs^ args);
    void onPointerReleased(PointerEventArgs^ args);
    void onPointerWheelChanged(PointerEventArgs^ args);
    void onKeyDown(KeyEventArgs^ args);
    void onKeyUp(KeyEventArgs^ args);
    void onCharacterReceived(CharacterReceivedEventArgs^ args);
    void onMouseMoved(Windows::Devices::Input::MouseDevice^ sender, Windows::Devices::Input::MouseEventArgs^ args);
    
    // UWP FIX: VisibleBoundsChanged handler for navigation bar changes
    void onVisibleBoundsChanged(Windows::UI::ViewManagement::ApplicationView^ sender, Platform::Object^ args);

private:
    // UWP Core objects
    CoreWindow^ _coreWindow;
    DisplayInformation^ _displayInfo;
    
    // EGL objects
    EGLDisplay _eglDisplay;
    EGLSurface _eglSurface;
    EGLContext _eglContext;
    EGLConfig _eglConfig;
    
    // Mouse state
    bool _mouseGrabbed;
    bool _ignoreNextMove;
    Windows::Devices::Input::MouseDevice^ _mouseDevice;
    
    // Screen dimensions
    int _screenWidth;
    int _screenHeight;
    float _pixelsPerMillimeter;
    bool _touchPresent;
    bool _mousePresent;
    bool _keyboardPresent;
    bool _gameInitialized;
    
    // UWP FIX: Navigation bar offset for Lumia phones with hardware buttons
    // VisibleBounds may be offset from CoreWindow when nav bar is present
    int _visibleBoundsOffsetX;
    int _visibleBoundsOffsetY;
    
    // UWP FIX: Touch coordinate scaling when nav bar reduces visible area
    // If nav bar is on right/left, visible width < full width, need to scale touch X
    double _touchScaleX;
    double _touchScaleY;
    
    // Storage paths
    std::wstring _localFolderPath;
    std::wstring _tempFolderPath;
    std::wstring _packagePath;
    
    // Helper methods
    void initializeStoragePaths();
    std::string platformStringToStd(Platform::String^ platformStr);
    Platform::String^ stdStringToPlatform(const std::string& stdStr);
    std::string extractFileName(const std::string& path);
    std::wstring stringToWString(const std::string& s);
    std::string wstringToStd(const std::wstring& ws);
    
    // PNG loading helpers
    static void pngReadFromBuffer(png_structp pngPtr, png_bytep data, png_size_t length);
    std::vector<unsigned char> readFileToBuffer(const std::string& path);
    TextureData loadPNGTexture(const std::string& filename);
    
    // Screenshot helper
    void savePNGScreenshot(const std::string& filename, int width, int height, unsigned char* pixelData);
    
    // Input helpers
    int mapVirtualKeyToKeyCode(int virtualKey);
    void updateScreenDimensions();

    // Event tokens for cleanup
    ::Windows::Foundation::EventRegistrationToken _orientationToken;
    ::Windows::Foundation::EventRegistrationToken _dpiToken;
    ::Windows::Foundation::EventRegistrationToken _visibleBoundsToken;
    bool _isUpdatingDimensions;
    std::mutex _gameMutex;
    
    // Cached input profile - determined once at initialization
    InputProfile _inputProfile;
    
    // Mobile touch tracking for camera delta calculation
    short _lastTouchX;
    short _lastTouchY;
    bool _hasLastTouchPosition;
    
    // Input event buffering for thread-safe input handling on UWP
    // NOTE: Only used for Mobile_TouchOnly profile. Desktop uses immediate input.
    struct InputEvent {
        bool isMouse;
        char actionButtonId;
        char buttonData;
        short x, y;
        short dx, dy;
        unsigned char pointerId;
    };
    std::vector<InputEvent> _inputBuffer;
    std::mutex _inputBufferMutex;
    
    // Shared pointer ID mapping for multitouch (shared across all input processing branches)
    char _pointerIdMap[256];
    bool _pointerIdActive[4];
    bool _pointerMapInitialized;
};

#endif /*APPPLATFORM_UWP_H__*/

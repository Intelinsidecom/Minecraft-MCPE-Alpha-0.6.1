#include "AppPlatform_uwp.h"
#include "platform/input/Mouse.h"
#include "platform/input/Keyboard.h"
#include "platform/input/Multitouch.h"
#include "platform/input/Controller.h"
#include "client/Minecraft.h"

// UWP-specific: Force static initialization of input systems
// This must happen BEFORE any input events can be processed
#ifdef _UWP
namespace UWPInputInit {
    // Explicitly reference static symbols to force their initialization
    // This ensures the static Mouse::_instance and Multitouch statics are
    // constructed before any code tries to use them
    struct StaticInitializer {
        StaticInitializer() {
            // Force static initialization by calling methods on the static objects
            // This will cause the static constructors to run if they haven't already
            Mouse::reset();
            Multitouch::reset();
            
            // Additional safety: explicitly touch the vectors
            Mouse::feed(0, 0, 0, 0);
            Multitouch::feed(0, 0, 0, 0, 0);
        }
    };
    
    // This global object will be constructed before main()
    // and will force the input statics to initialize
    StaticInitializer g_uwpInputInitializer;
}
#endif

// Define OPENGL_ES for UWP to use GLES headers
#define OPENGL_ES
#include "client/renderer/gles.h"
#include "client/renderer/GLESLoader.h"

// ANGLE EGL extension constants for fast present path
// (defined directly to avoid header dependency issues)
#define EGL_PLATFORM_ANGLE_ANGLE              0x3202
#define EGL_PLATFORM_ANGLE_TYPE_ANGLE           0x3203
#define EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE     0x3208
#define EGL_EXPERIMENTAL_PRESENT_PATH_ANGLE     0x33A4
#define EGL_EXPERIMENTAL_PRESENT_PATH_FAST_ANGLE 0x33A9
#define EGL_EXPERIMENTAL_PRESENT_PATH_COPY_ANGLE 0x33AA
#define EGL_ANGLE_SURFACE_RENDER_TO_BACK_BUFFER 0x320C

// GPU Performance optimizations
#define EGL_PLATFORM_ANGLE_DEBUG_LAYERS_ENABLED_ANGLE   0x3451
#define EGL_CONTEXT_PROGRAM_BINARY_CACHE_ENABLED_ANGLE  0x3459
#define EGL_PLATFORM_ANGLE_ENABLE_AUTOMATIC_TRIM_ANGLE  0x320F

// Function pointer typedef for eglGetPlatformDisplayEXT
typedef EGLDisplay (EGLAPIENTRYP PFNEGLGETPLATFORMDISPLAYEXTPROC)(EGLenum platform, void *native_display, const EGLint *attrib_list);

// Define path separator for UWP
#ifdef _UWP
#define PATH_SEPARATOR "\\"
#else
#define PATH_SEPARATOR "/"
#endif

// UWP Headers - order matters
#include <windows.foundation.h>
#include <windows.applicationmodel.core.h>
#include <windows.ui.core.h>
#include <windows.graphics.display.h>
#include <windows.storage.h>
#include <windows.storage.streams.h>
#include <windows.system.threading.h>
#include <wrl/client.h>
#include <wrl/wrappers/corewrappers.h>
#include <wrl/event.h>
#include <ppltasks.h>
#include <shcore.h>
#include <robuffer.h>
#include <algorithm>
#include <vector>

using namespace Windows::Storage;
using namespace Windows::System;
using namespace Windows::ApplicationModel;
using namespace Windows::Foundation;
using namespace concurrency;
using namespace Microsoft::WRL;

AppPlatform_uwp::AppPlatform_uwp()
    : _coreWindow(nullptr)
    , _displayInfo(nullptr)
    , _eglDisplay(EGL_NO_DISPLAY)
    , _eglSurface(EGL_NO_SURFACE)
    , _eglContext(EGL_NO_CONTEXT)
    , _eglConfig(nullptr)
    , _mouseGrabbed(false)
    , _ignoreNextMove(false)
    , _screenWidth(0)
    , _screenHeight(0)
    , _pixelsPerMillimeter(0.0f)
    , _touchPresent(false)
    , _mousePresent(false)
    , _keyboardPresent(false)
    , _isUpdatingDimensions(false)
    , _gameInitialized(false)
    , _pointerMapInitialized(false)
    , _visibleBoundsOffsetX(0)
    , _visibleBoundsOffsetY(0)
    , _touchScaleX(1.0)
    , _touchScaleY(1.0)
    , _inputProfile(InputProfile::Desktop_MouseKeyboard)  // Default, will be determined below
    , _lastTouchX(0)
    , _lastTouchY(0)
    , _hasLastTouchPosition(false)
{
    // Initialize pointer ID mapping arrays
    for (int i = 0; i < 256; i++) _pointerIdMap[i] = -1;
    for (int i = 0; i < 4; i++) _pointerIdActive[i] = false;
    _orientationToken.Value = 0;
    _dpiToken.Value = 0;

    // Force landscape for all touch devices on UWP
    auto touch = ref new Windows::Devices::Input::TouchCapabilities();
    if (touch->TouchPresent > 0)
    {
        Windows::Graphics::Display::DisplayInformation::AutoRotationPreferences = 
            Windows::Graphics::Display::DisplayOrientations::Landscape |
            Windows::Graphics::Display::DisplayOrientations::LandscapeFlipped;
    }
    
    // Initialize input systems early to prevent crashes
    Mouse::reset();
    Multitouch::reset();
    
    // Detect device capabilities
    _touchPresent = touch->TouchPresent > 0;
    
    auto mouse = ref new Windows::Devices::Input::MouseCapabilities();
    _mousePresent = mouse->MousePresent > 0;
    
    // Also check keyboard - most desktop PCs have keyboards
    auto keyboard = ref new Windows::Devices::Input::KeyboardCapabilities();
    _keyboardPresent = keyboard->KeyboardPresent > 0;
    
    // DETERMINE INPUT PROFILE ONCE - this eliminates expensive runtime checks
    // 
    // DESKTOP INDICATORS (in order of reliability):
    // 1. No touch + has mouse/keyboard = Desktop PC
    // 2. Has keyboard + large screen (> 1024px width) = Desktop/Laptop
    // 3. Has mouse (even with touch) = Desktop with touchscreen
    //
    // MOBILE INDICATORS:
    // 1. Has touch only (no mouse, no keyboard) = Phone/Tablet
    // 2. Small screen (< 900px) + touch = Mobile
    
    bool hasDesktopInput = _mousePresent || _keyboardPresent;
    bool hasOnlyTouch = _touchPresent && !hasDesktopInput;
    
    if (!_touchPresent && (_mousePresent || _keyboardPresent)) {
        // Pure desktop - no touchscreen, has mouse and/or keyboard
        _inputProfile = InputProfile::Desktop_MouseKeyboard;
    }
    else if (hasOnlyTouch) {
        // Touch only with no desktop input = Mobile
        _inputProfile = InputProfile::Mobile_TouchOnly;
    }
    else if (_touchPresent && (_mousePresent || _keyboardPresent)) {
        // Both touch AND desktop input = Desktop with touchscreen
        // This covers: laptops with touchscreens, desktop with touch monitors
        _inputProfile = InputProfile::Desktop_Touch;
    }
    else {
        // Fallback: No touch, no mouse, no keyboard detected
        // Could be: Xbox, TV, or exotic setup - assume desktop for safety
        _inputProfile = InputProfile::Desktop_MouseKeyboard;
    }
    
    initializeStoragePaths();
    updateScreenDimensions();
}

AppPlatform_uwp::~AppPlatform_uwp()
{
    if (_eglContext != EGL_NO_CONTEXT)
    {
        eglDestroyContext(_eglDisplay, _eglContext);
    }
    if (_eglSurface != EGL_NO_SURFACE)
    {
        eglDestroySurface(_eglDisplay, _eglSurface);
    }
    if (_eglDisplay != EGL_NO_DISPLAY)
    {
        eglTerminate(_eglDisplay);
    }
}

std::string AppPlatform_uwp::extractFileName(const std::string& path)
{
    // Find the last path separator
    size_t lastSlash = path.find_last_of("/\\");
    if (lastSlash != std::string::npos)
    {
        return path.substr(lastSlash + 1);
    }
    return path; // No separator found, return the original path
}

std::string AppPlatform_uwp::wstringToStd(const std::wstring& ws)
{
    if (ws.empty()) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), (int)ws.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), (int)ws.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

std::wstring AppPlatform_uwp::stringToWString(const std::string& s)
{
    if (s.empty()) return std::wstring();
    int len = (int)s.length();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), len, NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), len, &wstrTo[0], size_needed);
    return wstrTo;
}

void AppPlatform_uwp::setCoreWindow(CoreWindow^ window)
{
    _coreWindow = window;
    
    // NOTE: Input handlers are now registered AFTER game initialization
    // to avoid static initialization order crashes
    // This is done in registerInputHandlers() which is called after m_app->init()
    
    // Initialize MouseDevice for relative movement
    _mouseDevice = Windows::Devices::Input::MouseDevice::GetForCurrentView();
    _mouseDevice->MouseMoved += ref new TypedEventHandler<Windows::Devices::Input::MouseDevice^, Windows::Devices::Input::MouseEventArgs^>(
        [this](Windows::Devices::Input::MouseDevice^ sender, Windows::Devices::Input::MouseEventArgs^ args) { onMouseMoved(sender, args); });

    // Initialize EGL once we have a window
    initializeEGL();
    updateScreenDimensions();
}

void AppPlatform_uwp::registerInputHandlers()
{
    if (!_coreWindow)
    {
        return;
    }
    
    // Use cached _inputProfile instead of expensive isMobileDevice() calls
    // This is determined ONCE at initialization, not on every event
    if (_inputProfile == InputProfile::Mobile_TouchOnly)
    {
        // Mobile only: Enable pointer capture for immediate touch (no edge gesture delay)
        _coreWindow->SetPointerCapture();
    }
    
    // Set up input event handlers - WIN32-STYLE: No filtering, no extra checks
    _coreWindow->PointerPressed += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(
        [this](CoreWindow^ sender, PointerEventArgs^ args) { 
            // Only mark handled on mobile to prevent edge gesture delay
            if (_inputProfile == InputProfile::Mobile_TouchOnly)
                args->Handled = true;
            onPointerPressed(args); 
        });
    
    _coreWindow->PointerMoved += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(
        [this](CoreWindow^ sender, PointerEventArgs^ args) { onPointerMoved(args); });
    
    _coreWindow->PointerReleased += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(
        [this](CoreWindow^ sender, PointerEventArgs^ args) { onPointerReleased(args); });
    
    _coreWindow->PointerWheelChanged += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(
        [this](CoreWindow^ sender, PointerEventArgs^ args) { onPointerWheelChanged(args); });
    
    _coreWindow->KeyDown += ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(
        [this](CoreWindow^ sender, KeyEventArgs^ args) { onKeyDown(args); });
    
    _coreWindow->KeyUp += ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(
        [this](CoreWindow^ sender, KeyEventArgs^ args) { onKeyUp(args); });
    
    _coreWindow->CharacterReceived += ref new TypedEventHandler<CoreWindow^, CharacterReceivedEventArgs^>(
        [this](CoreWindow^ sender, CharacterReceivedEventArgs^ args) { onCharacterReceived(args); });
}

void AppPlatform_uwp::setDisplayInfo(DisplayInformation^ info)
{
    // Clean up previous event handlers if they exist
    if (_displayInfo != nullptr)
    {
        if (_orientationToken.Value != 0) _displayInfo->OrientationChanged -= _orientationToken;
        if (_dpiToken.Value != 0) _displayInfo->DpiChanged -= _dpiToken;
        _orientationToken.Value = 0;
        _dpiToken.Value = 0;
    }
    
    // UWP FIX: Clean up VisibleBoundsChanged handler
    if (_visibleBoundsToken.Value != 0)
    {
        auto appView = Windows::UI::ViewManagement::ApplicationView::GetForCurrentView();
        appView->VisibleBoundsChanged -= _visibleBoundsToken;
        _visibleBoundsToken.Value = 0;
    }

    _displayInfo = info;
    updateScreenDimensions();
    
    // Handle display orientation changes
    _orientationToken = _displayInfo->OrientationChanged += ref new TypedEventHandler<DisplayInformation^, Platform::Object^>(
        [this](DisplayInformation^ sender, Platform::Object^ args) { updateScreenDimensions(); });
    
    // Handle DPI changes
    _dpiToken = _displayInfo->DpiChanged += ref new TypedEventHandler<DisplayInformation^, Platform::Object^>(
        [this](DisplayInformation^ sender, Platform::Object^ args) { updateScreenDimensions(); });
    
    // UWP FIX: Subscribe to VisibleBoundsChanged to handle navigation bar expand/collapse
    auto appView = Windows::UI::ViewManagement::ApplicationView::GetForCurrentView();
    _visibleBoundsToken = appView->VisibleBoundsChanged += ref new TypedEventHandler<ApplicationView^, Platform::Object^>(
        [this](ApplicationView^ sender, Platform::Object^ args) { updateScreenDimensions(); });
}

void AppPlatform_uwp::initializeEGL()
{
    if (!_coreWindow)
    {
        return;
    }

    // EGL configuration attributes - GPU OPTIMIZED
    // Using RGB565 (16-bit color) to reduce GPU memory bandwidth by 50%
    EGLint configAttributes[] = {
        EGL_RED_SIZE, 5,
        EGL_GREEN_SIZE, 6,
        EGL_BLUE_SIZE, 5,
        EGL_ALPHA_SIZE, 0,
        EGL_DEPTH_SIZE, 16,
        EGL_STENCIL_SIZE, 0,
        EGL_SAMPLE_BUFFERS, 0,
        EGL_SAMPLES, 0,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_NONE
    };

    // EGL context attributes - GPU OPTIMIZED
    // Enable shader program binary cache to avoid recompilation
    EGLint contextAttributes[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_CONTEXT_PROGRAM_BINARY_CACHE_ENABLED_ANGLE, EGL_TRUE,
        EGL_NONE
    };

    // Initialize EGL with ANGLE platform display extension for fast present path
    // Get eglGetPlatformDisplayEXT function pointer
    PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT = 
        (PFNEGLGETPLATFORMDISPLAYEXTPROC)eglGetProcAddress("eglGetPlatformDisplayEXT");
    
    if (eglGetPlatformDisplayEXT)
    {
        // EGL platform attributes - GPU OPTIMIZED
        // Disable debug layers (major GPU overhead), enable automatic trim, use copy path
        EGLint platformAttributes[] = {
            EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
            EGL_EXPERIMENTAL_PRESENT_PATH_ANGLE, EGL_EXPERIMENTAL_PRESENT_PATH_COPY_ANGLE,
            EGL_PLATFORM_ANGLE_DEBUG_LAYERS_ENABLED_ANGLE, EGL_FALSE,
            EGL_PLATFORM_ANGLE_ENABLE_AUTOMATIC_TRIM_ANGLE, EGL_TRUE,
            EGL_NONE
        };
        
        _eglDisplay = eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE, 
                                                 EGL_DEFAULT_DISPLAY, 
                                                 platformAttributes);
    }
    else
    {
        // Fallback to legacy eglGetDisplay if platform extension not available
        _eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    }
    
    if (_eglDisplay == EGL_NO_DISPLAY)
    {
        return;
    }

    EGLint major, minor;
    if (!eglInitialize(_eglDisplay, &major, &minor))
    {
        return;
    }

    // Choose configuration
    EGLint numConfigs;
    if (!eglChooseConfig(_eglDisplay, configAttributes, &_eglConfig, 1, &numConfigs) || numConfigs == 0)
    {
        return;
    }

    // Create surface
    _eglSurface = eglCreateWindowSurface(_eglDisplay, _eglConfig, reinterpret_cast<EGLNativeWindowType>(_coreWindow), nullptr);
    if (_eglSurface == EGL_NO_SURFACE)
    {
        return;
    }

    // Create context
    _eglContext = eglCreateContext(_eglDisplay, _eglConfig, EGL_NO_CONTEXT, contextAttributes);
    if (_eglContext == EGL_NO_CONTEXT)
    {
        return;
    }

    // Make context current
    if (!eglMakeCurrent(_eglDisplay, _eglSurface, _eglSurface, _eglContext))
    {
        return;
    }
    
    // Load GLES functions immediately after making context current
    LoadGLESFunctions();
}

void AppPlatform_uwp::grabMouse()
{
    // Use cached profile - never grab on mobile-only devices
    if (_inputProfile == InputProfile::Mobile_TouchOnly)
    {
        return;
    }
    
    // On hybrid devices with touch but no mouse present, don't grab
    if (!_mousePresent && _touchPresent)
    {
        return;
    }
    _mouseGrabbed = true;
    _ignoreNextMove = true; // Ignore the first delta after grabbing
    if (_coreWindow)
    {
        _coreWindow->PointerCursor = nullptr;
    }
}

void AppPlatform_uwp::releaseMouse()
{
    _mouseGrabbed = false;
    if (_coreWindow)
    {
        _coreWindow->PointerCursor = ref new CoreCursor(CoreCursorType::Arrow, 0);
    }
}

bool AppPlatform_uwp::isMouseGrabbed() const
{
    return _mouseGrabbed;
}

std::vector<unsigned char> AppPlatform_uwp::readFileToBuffer(const std::string& path)
{
    std::vector<unsigned char> buffer;
    std::wstring wPath = stringToWString(path);
    
    HANDLE hFile = CreateFile2(wPath.c_str(), GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, nullptr);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        FILE_STANDARD_INFO info;
        if (GetFileInformationByHandleEx(hFile, FileStandardInfo, &info, sizeof(info)))
        {
            buffer.resize(static_cast<size_t>(info.EndOfFile.QuadPart));
            DWORD bytesRead;
            if (!ReadFile(hFile, buffer.data(), static_cast<DWORD>(buffer.size()), &bytesRead, nullptr))
            {
                buffer.clear();
            }
        }
        CloseHandle(hFile);
    }
    return buffer;
}

BinaryBlob AppPlatform_uwp::readAssetFile(const std::string& filename)
{
    BinaryBlob blob;
    std::string normalizedPath = filename;
    std::replace(normalizedPath.begin(), normalizedPath.end(), '/', '\\');
    
    // Strip leading ../ or ..\ from the path as CreateFile2 is very sensitive to them in UWP
    while (normalizedPath.substr(0, 3) == "..\\" || normalizedPath.substr(0, 3) == "../") {
        normalizedPath = normalizedPath.substr(3);
    }
    
    std::string justFileName = extractFileName(normalizedPath);
    
    std::vector<std::string> candidates;
    candidates.push_back(justFileName);
    candidates.push_back(normalizedPath);
    candidates.push_back("data\\" + justFileName);
    candidates.push_back("shaders\\" + justFileName);
    candidates.push_back("assets\\" + justFileName);
    candidates.push_back("images\\" + justFileName);
    candidates.push_back("data\\shaders\\" + justFileName);
    candidates.push_back("assets\\shaders\\" + justFileName);

    // De-duplicate candidates while preserving order
    std::vector<std::string> pathsToTry;
    for (const auto& c : candidates)
    {
        if (!c.empty() && std::find(pathsToTry.begin(), pathsToTry.end(), c) == pathsToTry.end())
            pathsToTry.push_back(c);
    }

    for (const auto& tryRelativePath : pathsToTry)
    {
        // UNIFIED FIX: Try relative path first (UWP native preference for package content)
        std::vector<unsigned char> buffer = readFileToBuffer(tryRelativePath);
        std::string usedPath = "relative:" + tryRelativePath;

        if (buffer.empty()) {
            // Fallback to absolute path using _packagePath
            std::string fullPath = wstringToStd(_packagePath);
            if (!fullPath.empty() && fullPath.back() != '\\')
                fullPath += "\\";
            fullPath += tryRelativePath;
            buffer = readFileToBuffer(fullPath);
            usedPath = fullPath;
        }

        if (!buffer.empty())
        {
            blob.size = static_cast<int>(buffer.size());
            blob.data = new unsigned char[blob.size];
            memcpy(blob.data, buffer.data(), blob.size);
            return blob;
        }
    }

    return BinaryBlob();
}

TextureData AppPlatform_uwp::loadTexture(const std::string& filename_, bool textureFolder)
{
    TextureData out;
    
    std::string filename;
    if (textureFolder)
    {
        filename = "images/" + filename_;
    }
    else
    {
        filename = filename_;
    }
    
    return loadPNGTexture(filename);
}

void AppPlatform_uwp::saveScreenshot(const std::string& filename, int glWidth, int glHeight)
{
    // Read pixels from framebuffer
    unsigned char* pixelData = new unsigned char[glWidth * glHeight * 4];
    glReadPixels(0, 0, glWidth, glHeight, GL_RGBA, GL_UNSIGNED_BYTE, pixelData);
    
    // Flip vertically (OpenGL has origin at bottom-left)
    unsigned char* flippedData = new unsigned char[glWidth * glHeight * 4];
    for (int y = 0; y < glHeight; y++)
    {
        for (int x = 0; x < glWidth; x++)
        {
            int srcIndex = ((glHeight - 1 - y) * glWidth + x) * 4;
            int dstIndex = (y * glWidth + x) * 4;
            flippedData[dstIndex + 0] = pixelData[srcIndex + 0];
            flippedData[dstIndex + 1] = pixelData[srcIndex + 1];
            flippedData[dstIndex + 2] = pixelData[srcIndex + 2];
            flippedData[dstIndex + 3] = pixelData[srcIndex + 3];
        }
    }
    
    savePNGScreenshot(filename, glWidth, glHeight, flippedData);
    
    delete[] pixelData;
    delete[] flippedData;
}

int AppPlatform_uwp::getScreenWidth()
{
    return _screenWidth;
}

int AppPlatform_uwp::getScreenHeight()
{
    return _screenHeight;
}

float AppPlatform_uwp::getPixelsPerMillimeter()
{
    return _pixelsPerMillimeter;
}

bool AppPlatform_uwp::supportsTouchscreen()
{
    // Only return true for actual mobile/touch-first devices
    // Desktop with touchscreen (laptops, touch monitors) should NOT show mobile controls
    // Mobile-only devices need the mobile UI
    return _inputProfile == InputProfile::Mobile_TouchOnly || _inputProfile == InputProfile::Mobile_WithMouse;
}

bool AppPlatform_uwp::isMobileDevice() const
{
    // Use cached profile - fast, no recalculation
    return _inputProfile == InputProfile::Mobile_TouchOnly || _inputProfile == InputProfile::Mobile_WithMouse;
}

bool AppPlatform_uwp::isTabletDevice() const
{
    // Tablet device: touch + larger screen (> 800px width and height)
    return _touchPresent && _screenWidth > 800 && _screenHeight > 800;
}

bool AppPlatform_uwp::supportsNonTouchscreen()
{
    // Check both mouse AND keyboard - either one indicates desktop input capability
    // UWP's MouseCapabilities sometimes returns 0 even when mouse is present
    return _mousePresent || _keyboardPresent;
}

bool AppPlatform_uwp::hasBuyButtonWhenInvalidLicense()
{
    return false; // Store handles licensing
}

int AppPlatform_uwp::checkLicense()
{
    // UWP store handles licensing
    return 0; // Valid license
}

std::string AppPlatform_uwp::getDateString(int s)
{
    std::stringstream ss;
    ss << s << " s (UTC)";
    return ss.str();
}

// Helper function to safely check if input systems are initialized
// This tries to safely verify the static objects are ready
static bool AreInputSystemsReady()
{
    try {
        // Try to access the input systems in a safe way
        // If they're not initialized, these will throw or crash
        Mouse::rewind();
        Multitouch::rewind();
        return true;
    } catch (...) {
        return false;
    }
}

void AppPlatform_uwp::onPointerPressed(PointerEventArgs^ args)
{
    if (!_gameInitialized || !_coreWindow) return;
    
    auto point = args->CurrentPoint;
    auto position = point->Position;
    int pointerId = point->PointerId;
    auto props = point->Properties;
    float scale = _displayInfo ? (_displayInfo->LogicalDpi / 96.0f) : 1.0f;
    
    // MOBILE: Buffer for later processing on game thread (avoids race conditions)
    if (_inputProfile == InputProfile::Mobile_TouchOnly)
    {
        short x = static_cast<short>((position.X * _touchScaleX) * scale + _visibleBoundsOffsetX);
        short y = static_cast<short>((position.Y * _touchScaleY) * scale + _visibleBoundsOffsetY);
        unsigned char pointerIdByte = static_cast<unsigned char>(pointerId & 0xFF);
        
        std::lock_guard<std::mutex> bufLock(_inputBufferMutex);
        _inputBuffer.push_back({true, MouseAction::ACTION_LEFT, 1, x, y, 0, 0, pointerIdByte});
        return;
    }
    
    // CRITICAL FIX: When mouse is grabbed (game mode), ONLY feed button state, NOT absolute coordinates
    // Feeding absolute coordinates while grabbed causes camera to snap (interprets as massive delta)
    if (_mouseGrabbed)
    {
        // Game mode: Feed only button state, no coordinates
        // Relative movement is handled exclusively by onMouseMoved
        if (props->IsLeftButtonPressed)
        {
            Mouse::feed(MouseAction::ACTION_LEFT, 1, 0, 0);  // No coords when grabbed!
        }
        if (props->IsRightButtonPressed)
        {
            Mouse::feed(MouseAction::ACTION_RIGHT, 1, 0, 0);  // No coords when grabbed!
        }
        return;
    }
    
    // DESKTOP GUI mode (not grabbed): Feed absolute coordinates for cursor position
    short x = static_cast<short>(position.X * scale);
    short y = static_cast<short>(position.Y * scale);
    
    if (props->IsLeftButtonPressed)
    {
        Mouse::feed(MouseAction::ACTION_LEFT, 1, x, y);
        Multitouch::feed(1, 1, x, y, 0);
    }
    if (props->IsRightButtonPressed)
    {
        Mouse::feed(MouseAction::ACTION_RIGHT, 1, x, y);
        Multitouch::feed(1, 1, x, y, 0);
    }
}

void AppPlatform_uwp::onPointerMoved(PointerEventArgs^ args)
{
    if (!_gameInitialized || !_coreWindow) return;
    
    auto point = args->CurrentPoint;
    auto position = point->Position;
    int pointerId = point->PointerId;
    float scale = _displayInfo ? (_displayInfo->LogicalDpi / 96.0f) : 1.0f;
    
    // MOBILE: Buffer for later processing on game thread (avoids race conditions)
    if (_inputProfile == InputProfile::Mobile_TouchOnly)
    {
        short x = static_cast<short>((position.X * _touchScaleX) * scale + _visibleBoundsOffsetX);
        short y = static_cast<short>((position.Y * _touchScaleY) * scale + _visibleBoundsOffsetY);
        unsigned char pointerIdByte = static_cast<unsigned char>(pointerId & 0xFF);
        
        std::lock_guard<std::mutex> bufLock(_inputBufferMutex);
        _inputBuffer.push_back({true, MouseAction::ACTION_MOVE, 0, x, y, 0, 0, pointerIdByte});
        return;
    }
    
    // DESKTOP: When mouse is grabbed, PointerMoved provides absolute coordinates
    // which interfere with the relative movement from MouseDevice. Skip entirely.
    if (_mouseGrabbed)
    {
        return;
    }
    
    // DESKTOP GUI mode (not grabbed): Update cursor position
    short x = static_cast<short>(position.X * scale);
    short y = static_cast<short>(position.Y * scale);
    
    Mouse::feed(MouseAction::ACTION_MOVE, 0, x, y);
    Multitouch::feed(1, 0, x, y, 0);
}

void AppPlatform_uwp::onMouseMoved(Windows::Devices::Input::MouseDevice^ sender, Windows::Devices::Input::MouseEventArgs^ args)
{
    if (!_mouseGrabbed || _ignoreNextMove) {
        _ignoreNextMove = false;
        return;
    }

    auto delta = args->MouseDelta;
    if (delta.X != 0 || delta.Y != 0)
    {
        // Cap deltas to prevent wild snaps at low FPS
        short dx = static_cast<short>((std::max)(-200, (std::min)(200, delta.X)));
        short dy = static_cast<short>((std::max)(-200, (std::min)(200, delta.Y)));

        // Process relative mouse movement IMMEDIATELY for camera control
        // This eliminates input lag for mouse look (critical for desktop)
        Mouse::feed(MouseAction::ACTION_MOVE, 0, 0, 0, dx, dy);
    }
}

void AppPlatform_uwp::onPointerReleased(PointerEventArgs^ args)
{
    if (!_gameInitialized || !_coreWindow) return;
    
    auto point = args->CurrentPoint;
    auto position = point->Position;
    int pointerId = point->PointerId;
    auto kind = point->Properties->PointerUpdateKind;
    float scale = _displayInfo ? (_displayInfo->LogicalDpi / 96.0f) : 1.0f;
    
    // MOBILE: Buffer for later processing on game thread (avoids race conditions)
    if (_inputProfile == InputProfile::Mobile_TouchOnly)
    {
        short x = static_cast<short>((position.X * _touchScaleX) * scale + _visibleBoundsOffsetX);
        short y = static_cast<short>((position.Y * _touchScaleY) * scale + _visibleBoundsOffsetY);
        unsigned char pointerIdByte = static_cast<unsigned char>(pointerId & 0xFF);
        
        std::lock_guard<std::mutex> bufLock(_inputBufferMutex);
        _inputBuffer.push_back({true, MouseAction::ACTION_LEFT, 0, x, y, 0, 0, pointerIdByte});
        return;
    }
    
    // CRITICAL FIX: When mouse is grabbed (game mode), ONLY feed button state, NOT absolute coordinates
    if (_mouseGrabbed)
    {
        if (kind == Windows::UI::Input::PointerUpdateKind::LeftButtonReleased)
        {
            Mouse::feed(MouseAction::ACTION_LEFT, 0, 0, 0);  // No coords when grabbed!
        }
        else if (kind == Windows::UI::Input::PointerUpdateKind::RightButtonReleased)
        {
            Mouse::feed(MouseAction::ACTION_RIGHT, 0, 0, 0);  // No coords when grabbed!
        }
        return;
    }
    
    // DESKTOP GUI mode (not grabbed): Feed absolute coordinates for cursor position
    short x = static_cast<short>(position.X * scale);
    short y = static_cast<short>(position.Y * scale);
    
    if (kind == Windows::UI::Input::PointerUpdateKind::LeftButtonReleased)
    {
        Mouse::feed(MouseAction::ACTION_LEFT, 0, x, y);
        Multitouch::feed(1, 0, x, y, 0);
    }
    else if (kind == Windows::UI::Input::PointerUpdateKind::RightButtonReleased)
    {
        Mouse::feed(MouseAction::ACTION_RIGHT, 0, x, y);
        Multitouch::feed(1, 0, x, y, 0);
    }
}

void AppPlatform_uwp::onPointerWheelChanged(PointerEventArgs^ args)
{
    if (!_gameInitialized) return;
    
    auto point = args->CurrentPoint;
    int wheelDelta = point->Properties->MouseWheelDelta;
    int scrollDirection = (wheelDelta > 0) ? 1 : -1;
    
    // DESKTOP: Simple coordinate calculation - no touch scaling needed
    auto position = point->Position;
    float scale = _displayInfo ? (_displayInfo->LogicalDpi / 96.0f) : 1.0f;
    short x = static_cast<short>(position.X * scale);
    short y = static_cast<short>(position.Y * scale);
    
    // Wheel events are immediate (no buffering)
    Mouse::feed(MouseAction::ACTION_WHEEL, (char)scrollDirection, x, y);
}

void AppPlatform_uwp::onKeyDown(KeyEventArgs^ args)
{
    // WIN32-STYLE: Direct feed, no checks, no branching
    // This eliminates stuttering caused by isMobileDevice() checks and _gameInitialized checks
    Keyboard::feed((unsigned char)args->VirtualKey, 1);
}

void AppPlatform_uwp::onKeyUp(KeyEventArgs^ args)
{
    // WIN32-STYLE: Direct feed, no checks
    Keyboard::feed((unsigned char)args->VirtualKey, 0);
}

void AppPlatform_uwp::onCharacterReceived(CharacterReceivedEventArgs^ args)
{
    // WIN32-STYLE: Direct feed for printable characters
    if (args->KeyCode >= 32 && args->KeyCode <= 126)
        Keyboard::feedText(args->KeyCode);
}

void AppPlatform_uwp::onVisibleBoundsChanged(Windows::UI::ViewManagement::ApplicationView^ sender, Platform::Object^ args)
{
    updateScreenDimensions();
}

void AppPlatform_uwp::initializeStoragePaths()
{
    try
    {
        // Get storage folders
        ApplicationData^ appData = ApplicationData::Current;
        Package^ package = Package::Current;
        
        _localFolderPath = appData->LocalFolder->Path->Data();
        _tempFolderPath = appData->TemporaryFolder->Path->Data();
        _packagePath = package->InstalledLocation->Path->Data();
        
    }
    catch (Platform::Exception^ e)
    {
        _localFolderPath = L"./local";
        _tempFolderPath = L"./temp";
        _packagePath = L"./package";
    }
}

std::string AppPlatform_uwp::platformStringToStd(Platform::String^ platformStr)
{
    std::wstring wstr(platformStr->Begin());
    return std::string(wstr.begin(), wstr.end());
}

Platform::String^ AppPlatform_uwp::stdStringToPlatform(const std::string& stdStr)
{
    std::wstring wstr(stdStr.begin(), stdStr.end());
    return ref new Platform::String(wstr.c_str());
}

struct PngReadDataStruct {
    const unsigned char* buffer;
    size_t size;
    size_t offset;
};

void AppPlatform_uwp::pngReadFromBuffer(png_structp pngPtr, png_bytep data, png_size_t length)
{
    auto readData = static_cast<PngReadDataStruct*>(png_get_io_ptr(pngPtr));
    if (readData->offset + length > readData->size)
    {
        png_error(pngPtr, "Read Error");
    }
    memcpy(data, readData->buffer + readData->offset, length);
    readData->offset += length;
}


TextureData AppPlatform_uwp::loadPNGTexture(const std::string& filename)
{
    TextureData out;
    std::string normalizedPath = filename;
    std::replace(normalizedPath.begin(), normalizedPath.end(), '/', '\\');
    std::string justFileName = extractFileName(normalizedPath);
    
    std::vector<std::string> candidates;
    candidates.push_back(justFileName);
    candidates.push_back(normalizedPath);
    candidates.push_back("images\\" + justFileName);
    candidates.push_back("assets\\" + justFileName);
    candidates.push_back("data\\" + justFileName);

    // De-duplicate candidates
    std::vector<std::string> pathsToTry;
    for (const auto& c : candidates)
    {
        if (!c.empty() && std::find(pathsToTry.begin(), pathsToTry.end(), c) == pathsToTry.end())
            pathsToTry.push_back(c);
    }

    for (const auto& tryRelativePath : pathsToTry)
    {
        std::string fullPath = wstringToStd(_packagePath);
        if (!fullPath.empty() && fullPath.back() != '\\')
            fullPath += "\\";
        fullPath += tryRelativePath;
        
        std::vector<unsigned char> buffer = readFileToBuffer(fullPath);
        if (!buffer.empty())
        {
            // Initializing memory based PNG reading
            PngReadDataStruct readData = { buffer.data(), buffer.size(), 0 };

            png_structp pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
            if (!pngPtr) { continue; }
            
            png_infop infoPtr = png_create_info_struct(pngPtr);
            if (!infoPtr) { png_destroy_read_struct(&pngPtr, nullptr, nullptr); continue; }
            
            if (setjmp(png_jmpbuf(pngPtr))) { png_destroy_read_struct(&pngPtr, &infoPtr, nullptr); continue; }
            
            // Set custom read function
            png_set_read_fn(pngPtr, &readData, pngReadFromBuffer);
            
            png_read_info(pngPtr, infoPtr);
            
            out.w = png_get_image_width(pngPtr, infoPtr);
            out.h = png_get_image_height(pngPtr, infoPtr);
            
            int bitDepth = png_get_bit_depth(pngPtr, infoPtr);
            int colorType = png_get_color_type(pngPtr, infoPtr);
            
            if (colorType == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(pngPtr);
            if (colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8) png_set_expand_gray_1_2_4_to_8(pngPtr);
            if (png_get_valid(pngPtr, infoPtr, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(pngPtr);
            if (bitDepth == 16) png_set_strip_16(pngPtr);
            if (colorType == PNG_COLOR_TYPE_GRAY || colorType == PNG_COLOR_TYPE_GRAY_ALPHA) png_set_gray_to_rgb(pngPtr);
            
            png_read_update_info(pngPtr, infoPtr);
            int rowBytes = png_get_rowbytes(pngPtr, infoPtr);
            out.data = new unsigned char[rowBytes * out.h];
            out.memoryHandledExternally = false;
            
            png_bytep* rowPointers = new png_bytep[out.h];
            for (int i = 0; i < out.h; i++) rowPointers[i] = out.data + i * rowBytes;
            
            png_read_image(pngPtr, rowPointers);
            
            delete[] rowPointers;
            png_destroy_read_struct(&pngPtr, &infoPtr, nullptr);
 
            return out;
        }
    }
 
    return out;
}



void AppPlatform_uwp::savePNGScreenshot(const std::string& filename, int width, int height, unsigned char* pixelData)
{
    std::wstring wRelativePath = stringToWString(filename);
    std::wstring wFullPath = _localFolderPath;
    if (!wFullPath.empty() && wFullPath.back() != L'\\')
        wFullPath += L"\\";
    wFullPath += wRelativePath;
    
    FILE* file = _wfopen(wFullPath.c_str(), L"wb");
    if (!file)
    {
        return;
    }
    
    png_structp pngPtr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!pngPtr)
    {
        fclose(file);
        return;
    }
    
    png_infop infoPtr = png_create_info_struct(pngPtr);
    if (!infoPtr)
    {
        fclose(file);
        png_destroy_write_struct(&pngPtr, nullptr);
        return;
    }
    
    if (setjmp(png_jmpbuf(pngPtr)))
    {
        fclose(file);
        png_destroy_write_struct(&pngPtr, &infoPtr);
        return;
    }
    
    png_init_io(pngPtr, file);
    
    png_set_IHDR(pngPtr, infoPtr, width, height, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE,
                PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    
    png_write_info(pngPtr, infoPtr);
    
    png_bytep* rowPointers = new png_bytep[height];
    for (int i = 0; i < height; i++)
    {
        rowPointers[i] = pixelData + i * width * 4;
    }
    
    png_write_image(pngPtr, rowPointers);
    png_write_end(pngPtr, nullptr);
    
    delete[] rowPointers;
    png_destroy_write_struct(&pngPtr, &infoPtr);
    fclose(file);
    
}

int AppPlatform_uwp::mapVirtualKeyToKeyCode(int virtualKey)
{
    // Map Windows Virtual Key codes to Minecraft key codes
    switch (virtualKey)
    {
        case 0x08: return 8;   // Backspace
        case 0x09: return 9;   // Tab
        case 0x0D: return 13;  // Enter
        case 0x1B: return 27;  // Escape
        case 0x20: return 32;  // Space
        case 0x25: return 37;  // Left arrow
        case 0x26: return 38;  // Up arrow
        case 0x27: return 39;  // Right arrow
        case 0x28: return 40;  // Down arrow
        case 0x30: return 48;  // 0
        case 0x31: return 49;  // 1
        case 0x32: return 50;  // 2
        case 0x33: return 51;  // 3
        case 0x34: return 52;  // 4
        case 0x35: return 53;  // 5
        case 0x36: return 54;  // 6
        case 0x37: return 55;  // 7
        case 0x38: return 56;  // 8
        case 0x39: return 57;  // 9
        case 0x41: return 65;  // A
        case 0x42: return 66;  // B
        case 0x43: return 67;  // C
        case 0x44: return 68;  // D
        case 0x45: return 69;  // E
        case 0x46: return 70;  // F
        case 0x47: return 71;  // G
        case 0x48: return 72;  // H
        case 0x49: return 73;  // I
        case 0x4A: return 74;  // J
        case 0x4B: return 75;  // K
        case 0x4C: return 76;  // L
        case 0x4D: return 77;  // M
        case 0x4E: return 78;  // N
        case 0x4F: return 79;  // O
        case 0x50: return 80;  // P
        case 0x51: return 81;  // Q
        case 0x52: return 82;  // R
        case 0x53: return 83;  // S
        case 0x54: return 84;  // T
        case 0x55: return 85;  // U
        case 0x56: return 86;  // V
        case 0x57: return 87;  // W
        case 0x58: return 88;  // X
        case 0x59: return 89;  // Y
        case 0x5A: return 90;  // Z
        case 0x60: return 96;  // Numpad 0
        case 0x61: return 97;  // Numpad 1
        case 0x62: return 98;  // Numpad 2
        case 0x63: return 99;  // Numpad 3
        case 0x64: return 100; // Numpad 4
        case 0x65: return 101; // Numpad 5
        case 0x66: return 102; // Numpad 6
        case 0x67: return 103; // Numpad 7
        case 0x68: return 104; // Numpad 8
        case 0x69: return 105; // Numpad 9
        case 0x6A: return 106; // Multiply
        case 0x6B: return 107; // Add
        case 0x6C: return 108; // Separator
        case 0x6D: return 109; // Subtract
        case 0x6E: return 110; // Decimal
        case 0x6F: return 111; // Divide
        case 0x70: return 112; // F1
        case 0x71: return 113; // F2
        case 0x72: return 114; // F3
        case 0x73: return 115; // F4
        case 0x74: return 116; // F5
        case 0x75: return 117; // F6
        case 0x76: return 118; // F7
        case 0x77: return 119; // F8
        case 0x78: return 120; // F9
        case 0x79: return 121; // F10
        case 0x7A: return 122; // F11
        case 0x7B: return 123; // F12
        case 0xA0: return 10;  // Left Shift (KEY_LSHIFT)
        case 0xA1: return 10;  // Right Shift (KEY_LSHIFT)
        case 0x10: return 10;  // Shift (generic, maps to KEY_LSHIFT)
        case 0xBB: return 187; // =
        case 0xBC: return 188; // ,
        case 0xBD: return 189; // -
        case 0xBE: return 190; // .
        case 0xBF: return 191; // /
        case 0xC0: return 192; // `
        case 0xDB: return 219; // [
        case 0xDC: return 220; // \
        case 0xDD: return 221; // ]
        case 0xDE: return 222; // '
        default: return 0;
    }
}

void AppPlatform_uwp::updateScreenDimensions()
{
    if (_isUpdatingDimensions) return;
    _isUpdatingDimensions = true;

    if (_coreWindow && _displayInfo)
    {
        // UWP FIX: Calculate both CoreWindow (full) and VisibleBounds sizes
        // EGL renders to CoreWindow, but touch events come in relative to VisibleBounds
        // when the navigation bar is present on Lumia phones
        auto bounds = _coreWindow->Bounds;
        double fullWidth = bounds.Width;
        double fullHeight = bounds.Height;
        
        // Get visible bounds for offset calculation
        auto appView = Windows::UI::ViewManagement::ApplicationView::GetForCurrentView();
        auto visibleBounds = appView->VisibleBounds;
        
        // Get DPI scaling
        double dpi = _displayInfo->LogicalDpi;
        double scale = dpi / 96.0;
        
        // Use FULL CoreWindow size for screen dimensions (EGL renders here)
        _screenWidth = static_cast<int>(fullWidth * scale);
        _screenHeight = static_cast<int>(fullHeight * scale);
        
        // UWP FIX: Calculate navigation bar offset for touch coordinate adjustment
        // Touch coordinates come in relative to VisibleBounds, but we render to CoreWindow
        // When nav bar is present, VisibleBounds is inset from CoreWindow
        _visibleBoundsOffsetX = static_cast<int>((visibleBounds.X - bounds.X) * scale);
        _visibleBoundsOffsetY = static_cast<int>((visibleBounds.Y - bounds.Y) * scale);
        
        // UWP FIX: Calculate touch scaling factor when nav bar reduces visible area
        // If visible width < full width, we need to scale touch X to match render width
        // This ensures hitboxes align when nav bar is on the right/left in landscape
        double visibleWidthDIPs = visibleBounds.Width;
        double visibleHeightDIPs = visibleBounds.Height;
        _touchScaleX = (visibleWidthDIPs > 0) ? (fullWidth / visibleWidthDIPs) : 1.0;
        _touchScaleY = (visibleHeightDIPs > 0) ? (fullHeight / visibleHeightDIPs) : 1.0;
        
        // Force landscape orientation on mobile-only devices
        if (_inputProfile == InputProfile::Mobile_TouchOnly)
        {
            // Lock to landscape on phones to match Minecraft PE behavior
            Windows::Graphics::Display::DisplayInformation::AutoRotationPreferences = 
                Windows::Graphics::Display::DisplayOrientations::Landscape |
                Windows::Graphics::Display::DisplayOrientations::LandscapeFlipped;
        }
        
        // Normalize pixels per millimeter for GUI scaling
        // Android calculates: (xdpi + ydpi) * 0.5f / 25.4f
        // We use LogicalDpi which already gives us the effective DPI
        float actualPPM = (float)(dpi / 25.4);
        
        // Detect if this is a mobile device based on LOGICAL screen size (not physical pixels)
        // Lumia 950: logical ~640x360, physical 2560x1440, scale=4.0
        bool isMobile = false;
        if (_touchPresent) {
            // Use FULL logical width/height for mobile detection (CoreWindow size)
            int logicalW = static_cast<int>(fullWidth);
            int logicalH = static_cast<int>(fullHeight);
            // Mobile = touch + small logical screen (<= 900 logical pixels)
            if (logicalW <= 900 || logicalH <= 900) {
                isMobile = true;
            }
        }
        
        if (isMobile)
        {
            // Mobile devices: use actual DPI-based PPM with a minimum floor to ensure
            // UI doesn't get too small on very high DPI phones (like Lumia 950 at 564 DPI)
            // Android-style: PPM = DPI / 25.4
            // For Lumia 950: 564 / 25.4 = 22.2 PPM, but we cap to prevent tiny UI
            _pixelsPerMillimeter = (std::max)(6.0f, (std::min)(actualPPM, 12.0f));
        }
        else if (_touchPresent)
        {
            // Tablets: use actual DPI with tighter bounds
            _pixelsPerMillimeter = (std::max)(4.0f, (std::min)(actualPPM, 8.0f));
        }
        else
        {
            // Desktop: use actual DPI but allow smaller UI
            _pixelsPerMillimeter = (std::max)(3.0f, (std::min)(actualPPM, 6.0f));
        }
    }

    _isUpdatingDimensions = false;
}

//
// UWP Input Buffering - Process input on game thread instead of UI thread
// This fixes race conditions on Windows 10 Mobile where UI thread callbacks
// crash when accessing input data structures while game thread is reading them
//

void AppPlatform_uwp::processBufferedInput()
{
    // Safety check
    if (!_gameInitialized) return;
    if (!AreInputSystemsReady()) return;
    
    // WIN32-STYLE: Desktop profiles use immediate input, no buffering needed
    // Only mobile touch-only profile uses buffered input
    if (_inputProfile != InputProfile::Mobile_TouchOnly) return;
    
    // Additional safety: ensure input systems are initialized
    static bool systemsChecked = false;
    static bool systemsReady = false;
    if (!systemsChecked)
    {
        // Try to safely check if mouse system is accessible
        try {
            Mouse::rewind();
            systemsReady = true;
        } catch (...) {
            systemsReady = false;
        }
        systemsChecked = true;
    }
    if (!systemsReady) return;
    
    // Swap buffers - UI thread writes to _inputBuffer, we process and clear it
    std::vector<InputEvent> eventsToProcess;
    {
        std::lock_guard<std::mutex> lock(_inputBufferMutex);
        if (_inputBuffer.empty()) return;
        eventsToProcess.swap(_inputBuffer);
    }
    
    
    // Process all buffered events on the game thread
    for (const auto& ev : eventsToProcess)
    {
        if (ev.isMouse)
        {
            // MOBILE TOUCH FIX: Handle touch events specially
            // ACTION_LEFT (press/release) and ACTION_MOVE need special handling
            // Note: We're already filtered to Mobile_TouchOnly profile, so no need to check again
            if (ev.actionButtonId == MouseAction::ACTION_LEFT || ev.actionButtonId == MouseAction::ACTION_MOVE)
            {
                // MOBILE FIX: Handle ACTION_MOVE events for camera control first
                if (ev.actionButtonId == MouseAction::ACTION_MOVE)
                {
                    // TouchInputHolder needs Multitouch move events to track finger movement
                    // Extract the actual pointerId from the event - this is key for multitouch!
                    int actualPointerId = static_cast<int>(ev.pointerId);  // unsigned char to int
                    
                    // Initialize mapping if not done yet
                    if (!_pointerMapInitialized) {
                        for (int i = 0; i < 256; i++) _pointerIdMap[i] = -1;
                        for (int i = 0; i < 4; i++) _pointerIdActive[i] = false;
                        _pointerMapInitialized = true;
                    }
                    
                    // Find or assign a slot for this pointer (0-3)
                    int mappedId = _pointerIdMap[actualPointerId];
                    if (mappedId < 0 || mappedId >= 4) {
                        // Find first free slot
                        for (int i = 0; i < 4; i++) {
                            if (!_pointerIdActive[i]) {
                                mappedId = i;
                                _pointerIdMap[actualPointerId] = (char)i;
                                _pointerIdActive[i] = true;
                                break;
                            }
                        }
                    }
                    if (mappedId < 0 || mappedId >= 4) mappedId = 0;  // Fallback
                    
                    // MOBILE CAMERA FIX: Calculate deltas for camera look
                    // Touch drag needs dx/dy values to rotate camera
                    short dx = 0, dy = 0;
                    if (_hasLastTouchPosition) {
                        dx = ev.x - _lastTouchX;
                        dy = ev.y - _lastTouchY;
                    }
                    _lastTouchX = ev.x;
                    _lastTouchY = ev.y;
                    _hasLastTouchPosition = true;
                    
                    // Feed move with deltas to Mouse for camera control
                    // dx/dy are critical for MouseTurnInput::getTurnDelta() which uses Mouse::getDX()/getDY()
                    Mouse::feed(MouseAction::ACTION_MOVE, 0, ev.x, ev.y, dx, dy);
                    
                    // Also feed to Multitouch so TouchInputHolder can track active pointers
                    // Use mappedId (0-3) instead of actual pointerId for compatibility
                    Multitouch::feed(1, 1, ev.x, ev.y, mappedId);  // buttonData=1 (held)
                }
                else if (ev.buttonData == 1)  // PRESS
                {
                    // Get the actual pointerId and map it to 0-3 range
                    int actualPointerId = static_cast<int>(ev.pointerId);  // unsigned char to int
                    
                    // Initialize mapping if not done yet
                    if (!_pointerMapInitialized) {
                        for (int i = 0; i < 256; i++) _pointerIdMap[i] = -1;
                        for (int i = 0; i < 4; i++) _pointerIdActive[i] = false;
                        _pointerMapInitialized = true;
                    }
                    
                    // Find existing slot for this pointer
                    int mappedId = _pointerIdMap[actualPointerId];
                    if (mappedId < 0 || mappedId >= 4) {
                        // Find first free slot
                        for (int i = 0; i < 4; i++) {
                            if (!_pointerIdActive[i]) {
                                mappedId = i;
                                _pointerIdMap[actualPointerId] = (char)i;
                                _pointerIdActive[i] = true;
                                break;
                            }
                        }
                    }
                    if (mappedId < 0 || mappedId >= 4) mappedId = 0;
                    
                    // Feed the press immediately to BOTH Mouse and Multitouch
                    Mouse::feed(ev.actionButtonId, ev.buttonData, ev.x, ev.y);
                    Multitouch::feed(1, 1, ev.x, ev.y, mappedId);  // Use mapped slot 0-3
                    
                    // Reset touch position tracking so delta starts from press position
                    _lastTouchX = ev.x;
                    _lastTouchY = ev.y;
                    _hasLastTouchPosition = true;
                }
                else if (ev.buttonData == 0)  // RELEASE
                {
                    // Get the actual pointerId and map it to 0-3 range
                    int actualPointerId = static_cast<int>(ev.pointerId);  // unsigned char to int
                    
                    // Initialize mapping if not done yet (should already be initialized, but be safe)
                    if (!_pointerMapInitialized) {
                        for (int i = 0; i < 256; i++) _pointerIdMap[i] = -1;
                        for (int i = 0; i < 4; i++) _pointerIdActive[i] = false;
                        _pointerMapInitialized = true;
                    }
                    
                    // Find the mapped slot for this pointer
                    int mappedId = _pointerIdMap[actualPointerId];
                    if (mappedId >= 0 && mappedId < 4) {
                        _pointerIdActive[mappedId] = false;  // Mark slot as free
                        _pointerIdMap[actualPointerId] = -1;
                    }
                    if (mappedId < 0 || mappedId >= 4) mappedId = 0;
                    
                    // UWP MOBILE FIX: Feed release immediately to BOTH systems
                    Mouse::feed(ev.actionButtonId, ev.buttonData, ev.x, ev.y);
                    Multitouch::feed(1, 0, ev.x, ev.y, mappedId);
                    
                    // Reset touch tracking so next touch doesn't calculate delta from old position
                    _hasLastTouchPosition = false;
                }
            }
            else
            {
                // DESKTOP: Feed mouse events directly (not mobile touch)
                if (ev.dx != 0 || ev.dy != 0)
                {
                    Mouse::feed(ev.actionButtonId, ev.buttonData, ev.x, ev.y, ev.dx, ev.dy);
                }
                else
                {
                    Mouse::feed(ev.actionButtonId, ev.buttonData, ev.x, ev.y);
                }
                // Also feed to Multitouch so UI elements that check Multitouch work correctly
                // Use pointerId 0 for desktop mouse (single pointer)
                Multitouch::feed(1, ev.buttonData, ev.x, ev.y, 0);
            }
        }
        else
        {
            // Multitouch events - feed to Multitouch system
            // Note: ev.pointerId here is the UWP pointer ID (0-255), NOT the mapped 0-3 slot
            // The mapping to 0-3 happens in the isMouse==true branch above
            // For raw multitouch events, we need to map them too for consistency
            int actualPointerId = static_cast<int>(ev.pointerId);
            
            // Initialize mapping if not done yet
            if (!_pointerMapInitialized) {
                for (int i = 0; i < 256; i++) _pointerIdMap[i] = -1;
                for (int i = 0; i < 4; i++) _pointerIdActive[i] = false;
                _pointerMapInitialized = true;
            }
            
            // Find or assign a slot for this pointer
            int mappedId = -1;
            for (int i = 0; i < 4; i++) {
                if (_pointerIdMap[actualPointerId] == i) {
                    mappedId = i;
                    break;
                }
            }
            if (mappedId == -1 && ev.buttonData == 1) {  // Only assign on press
                for (int i = 0; i < 4; i++) {
                    if (!_pointerIdActive[i]) {
                        mappedId = i;
                        _pointerIdMap[actualPointerId] = (char)i;
                        _pointerIdActive[i] = true;
                        break;
                    }
                }
            }
            if (mappedId == -1) mappedId = 0;
            
            // Free slot on release
            if (ev.buttonData == 0 && mappedId >= 0 && mappedId < 4) {
                _pointerIdActive[mappedId] = false;
                _pointerIdMap[actualPointerId] = -1;
            }
            
            Multitouch::feed(ev.actionButtonId, ev.buttonData, ev.x, ev.y, mappedId);
        }
    }
}

void AppPlatform_uwp::clearInputBuffer()
{
    std::lock_guard<std::mutex> lock(_inputBufferMutex);
    _inputBuffer.clear();
}

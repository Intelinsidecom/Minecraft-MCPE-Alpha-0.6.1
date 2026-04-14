#include "main_uwp.h"
#include "platform/log.h"
#include <Windows.Gaming.Input.h>
#include <ppltasks.h>
#include <wrl/wrappers/corewrappers.h>
#include <objbase.h>
#include <mutex>

using namespace Windows::Gaming::Input;
using namespace Windows::Foundation;
using namespace concurrency;


UWPApplication::UWPApplication()
    : m_window(nullptr)
    , m_displayInfo(nullptr)
    , m_platform(nullptr)
    , m_app(nullptr)
    , m_initialized(false)
    , m_timer(nullptr)
{
}

UWPApplication::~UWPApplication()
{
    CleanupGame();
}

void UWPApplication::Initialize(CoreApplicationView^ applicationView)
{
    // Set up activation handler
    applicationView->Activated += ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(
        this, &UWPApplication::OnActivated);
    
    // Set up suspension/resume handlers
    CoreApplication::Suspending += ref new EventHandler<SuspendingEventArgs^>(
        [this](Platform::Object^ sender, SuspendingEventArgs^ args) {
            OnSuspending(sender, args);
        });
    
    CoreApplication::Resuming += ref new EventHandler<Object^>(
        [this](Platform::Object^ sender, Object^ args) {
            OnResuming(sender, args);
        });
}

void UWPApplication::SetWindow(CoreWindow^ window)
{
    m_window = window;
    
    // Set up window event handlers
    window->SizeChanged += ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(
        this, &UWPApplication::OnWindowSizeChanged);
    
    window->VisibilityChanged += ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(
        this, &UWPApplication::OnVisibilityChanged);
    
    // Get display information
    m_displayInfo = DisplayInformation::GetForCurrentView();
    m_displayInfo->DpiChanged += ref new TypedEventHandler<DisplayInformation^, Object^>(
        this, &UWPApplication::OnDpiChanged);
    
    // Create platform implementation
    m_platform = new AppPlatform_uwp();
    m_platform->setCoreWindow(window);
    m_platform->setDisplayInfo(m_displayInfo);
    
    // Register platform BEFORE initializing EGL/Shaders
    glSetPlatform(m_platform);
    
    // Initialize EGL
    if (!InitializeEGL())
    {
        return;
    }
    
    // Set up input handlers
    SetupInputHandlers();
}

void UWPApplication::Load(Platform::String^ entryPoint)
{
}

void UWPApplication::Run()
{
    // Initialize the game
    InitializeGame();
    
    if (!m_initialized)
    {
        return;
    }
    
    // Process events until the application exits
    while (g_running && !m_app->wantToQuit())
    {
        // WIN32-STYLE: Process ALL pending events immediately without batching
        // This eliminates input latency caused by ProcessOneAndAllPending batching
        int eventCount = 0;
        const int MAX_EVENTS_PER_FRAME = 100;  // Safety limit
        
        // Process all pending events one at a time (immediate mode)
        auto dispatcher = CoreWindow::GetForCurrentThread()->Dispatcher;
        while (eventCount < MAX_EVENTS_PER_FRAME)
        {
            // ProcessOneIfPresent returns immediately if no events
            // We can't check return value, so rely on internal queue draining
            dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneIfPresent);
            eventCount++;
            
            // Check if we're quitting after each event
            if (!g_running || m_app->wantToQuit())
                break;
        }
        
        // Update and Render on the UI thread
        UpdateGame();
        RenderGame();
    }
}

void UWPApplication::Uninitialize()
{
    CleanupGame();
}

void UWPApplication::OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
{
    CoreWindow::GetForCurrentThread()->Activate();
}

void UWPApplication::OnSuspending(Platform::Object^ sender, SuspendingEventArgs^ args)
{
    auto deferral = args->SuspendingOperation->GetDeferral();
    
    // Save game state if needed
    deferral->Complete();
}

void UWPApplication::OnResuming(Platform::Object^ sender, Platform::Object^ args)
{
}

void UWPApplication::OnWindowSizeChanged(CoreWindow^ sender, WindowSizeChangedEventArgs^ args)
{
    float scale = m_displayInfo ? (m_displayInfo->LogicalDpi / 96.0f) : 1.0f;
    int width = static_cast<int>(args->Size.Width * scale);
    int height = static_cast<int>(args->Size.Height * scale);
    
    if (m_app && m_initialized)
    {
        try {
            m_app->setSize(width, height);
        } catch (...) {
        }
    }
}

void UWPApplication::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
    if (m_app && m_initialized)
    {
        if (!args->Visible)
        {
            // Pause the game when window is hidden
            static_cast<Minecraft*>(m_app)->pauseGame(false);
        }
    }
}

void UWPApplication::OnDpiChanged(DisplayInformation^ sender, Object^ args)
{
    if (m_platform)
    {
        m_platform->setDisplayInfo(sender);
    }
    
    // Potentially trigger a resize to game engine if scale drastically changed
    if (m_app && m_initialized && m_window)
    {
        float scale = sender->LogicalDpi / 96.0f;
        int width = static_cast<int>(m_window->Bounds.Width * scale);
        int height = static_cast<int>(m_window->Bounds.Height * scale);
        m_app->setSize(width, height);
    }
}

void UWPApplication::InitializeGame()
{
    if (m_initialized)
    {
        return;
    }
    
    try
    {
        // Set up application context
        m_context.platform = m_platform;
        m_context.display = m_platform->getEGLDisplay();
        m_context.surface = m_platform->getEGLSurface();
        m_context.context = m_platform->getEGLContext();
        m_context.doRender = true;
        
        // Create the game instance
        m_app = new NinecraftApp();
        g_app = m_app;
        
        // Set external storage paths (UWP writable folders)
        static_cast<NinecraftApp*>(m_app)->externalStoragePath = m_platform->getLocalStoragePath();
        static_cast<NinecraftApp*>(m_app)->externalCacheStoragePath = m_platform->getTempStoragePath();
        
        // Initialize the game
        m_app->init(m_context);
        
        // Mark game as initialized so input can be processed
        m_platform->setGameInitialized(true);
        
        // Register input handlers AFTER game is fully initialized
        // This prevents static initialization order crashes
        m_platform->registerInputHandlers();
        
        // Set initial window size
        int width = m_platform->getScreenWidth();
        int height = m_platform->getScreenHeight();
        m_app->setSize(width, height);
        
        m_initialized = true;
        g_running = true;
    }
    catch (Platform::Exception^ e)
    {
        m_initialized = false;
    }
    catch (std::exception& e)
    {
        m_initialized = false;
    }
}

void UWPApplication::UpdateGame()
{
    if (!m_initialized || !m_app)
    {
        return;
    }
    
    try
    {
        // Update the game and process input within the lock
        {
            std::lock_guard<std::mutex> lock(m_platform->getGameMutex());
            
            // Process gamepad input
            ProcessGamepadInput();
            
            m_app->update();
        }
    }
    catch (Platform::Exception^ e)
    {
    }
    catch (std::exception& e)
    {
    }
}

void UWPApplication::RenderGame()
{
    if (!m_initialized || !m_app || !m_context.doRender)
    {
        return;
    }
    
    try
    {
        // Make EGL context current
        if (eglMakeCurrent(m_context.display, m_context.surface, m_context.surface, m_context.context))
        {
            // Render the game
            {
                std::lock_guard<std::mutex> lock(m_platform->getGameMutex());
                m_app->draw();
            }
            
            // Swap buffers
            eglSwapBuffers(m_context.display, m_context.surface);
        }
    }
    catch (Platform::Exception^ e)
    {
    }
    catch (std::exception& e)
    {
    }
}

void UWPApplication::CleanupGame()
{
    if (!m_initialized)
    {
        return;
    }
    
    g_running = false;
    
    if (m_timer)
    {
        m_timer->Cancel();
        m_timer = nullptr;
    }
    
    if (m_app)
    {
        try
        {
            m_app->quit();
            Sleep(50); // Give the game time to clean up
            delete m_app;
            m_app = nullptr;
            g_app = nullptr;
        }
        catch (...)
        {
        }
    }
    
    CleanupEGL();
    
    if (m_platform)
    {
        delete m_platform;
        m_platform = nullptr;
    }
    
    m_initialized = false;
}

void UWPApplication::SetupInputHandlers()
{
    // Input handlers are already set up in AppPlatform_uwp
    // This method can be used for additional input setup if needed
}

void UWPApplication::ProcessGamepadInput()
{
    try
    {
        // Check for connected gamepads
        auto gamepads = Gamepad::Gamepads;
        
        if (gamepads->Size > 0)
        {
            auto gamepad = gamepads->GetAt(0); // Use first gamepad
            auto reading = gamepad->GetCurrentReading();
            
            // Map gamepad input to controller system
            float leftX = reading.LeftThumbstickX;
            float leftY = reading.LeftThumbstickY;
            float rightX = reading.RightThumbstickX;
            float rightY = reading.RightThumbstickY;
            
            // Apply deadzone
            const float deadzone = 0.1f;
            if (abs(leftX) < deadzone) leftX = 0.0f;
            if (abs(leftY) < deadzone) leftY = 0.0f;
            if (abs(rightX) < deadzone) rightX = 0.0f;
            if (abs(rightY) < deadzone) rightY = 0.0f;
            
            // Feed left stick (movement)
            if (abs(leftX) > 0.01f || abs(leftY) > 0.01f)
            {
                Controller::feed(0, Controller::STATE_MOVE, leftX, leftY);
            }
            
            // Feed right stick (camera)
            if (abs(rightX) > 0.01f || abs(rightY) > 0.01f)
            {
                Controller::feed(1, Controller::STATE_MOVE, rightX, rightY);
            }
            
            // Handle buttons
            bool buttonA = (reading.Buttons & GamepadButtons::A) == GamepadButtons::A;
            bool buttonB = (reading.Buttons & GamepadButtons::B) == GamepadButtons::B;
            bool buttonX = (reading.Buttons & GamepadButtons::X) == GamepadButtons::X;
            bool buttonY = (reading.Buttons & GamepadButtons::Y) == GamepadButtons::Y;
            bool buttonLB = (reading.Buttons & GamepadButtons::LeftShoulder) == GamepadButtons::LeftShoulder;
            bool buttonRB = (reading.Buttons & GamepadButtons::RightShoulder) == GamepadButtons::RightShoulder;
            bool buttonLT = reading.LeftTrigger > 0.5f;
            bool buttonRT = reading.RightTrigger > 0.5f;
            bool buttonLS = (reading.Buttons & GamepadButtons::LeftThumbstick) == GamepadButtons::LeftThumbstick;
            bool buttonRS = (reading.Buttons & GamepadButtons::RightThumbstick) == GamepadButtons::RightThumbstick;
            bool dpadUp = (reading.Buttons & GamepadButtons::DPadUp) == GamepadButtons::DPadUp;
            bool dpadDown = (reading.Buttons & GamepadButtons::DPadDown) == GamepadButtons::DPadDown;
            bool dpadLeft = (reading.Buttons & GamepadButtons::DPadLeft) == GamepadButtons::DPadLeft;
            bool dpadRight = (reading.Buttons & GamepadButtons::DPadRight) == GamepadButtons::DPadRight;
            bool buttonMenu = (reading.Buttons & GamepadButtons::Menu) == GamepadButtons::Menu;
            bool buttonView = (reading.Buttons & GamepadButtons::View) == GamepadButtons::View;
            
            // Map buttons to keyboard keys for compatibility
            if (buttonA) Keyboard::feed(' ', 1); else Keyboard::feed(' ', 0); // Jump
            if (buttonB) Keyboard::feed('E', 1); else Keyboard::feed('E', 0); // Inventory
            if (buttonX) Keyboard::feed('Q', 1); else Keyboard::feed('Q', 0); // Drop item
            if (buttonY) Keyboard::feed('F', 1); else Keyboard::feed('F', 0); // Swap item
            if (buttonLB) Keyboard::feed('1', 1); else Keyboard::feed('1', 0); // Hotbar 1
            if (buttonRB) Keyboard::feed('2', 1); else Keyboard::feed('2', 0); // Hotbar 2
            if (buttonLT) Keyboard::feed(256, 1); else Keyboard::feed(256, 0); // Left mouse
            if (buttonRT) Keyboard::feed(257, 1); else Keyboard::feed(257, 0); // Right mouse
            if (buttonMenu) Keyboard::feed(27, 1); else Keyboard::feed(27, 0); // Escape
            if (buttonView) Keyboard::feed('Tab', 1); else Keyboard::feed('Tab', 0); // Tab
            if (dpadUp) Keyboard::feed('W', 1); else Keyboard::feed('W', 0);
            if (dpadDown) Keyboard::feed('S', 1); else Keyboard::feed('S', 0);
            if (dpadLeft) Keyboard::feed('A', 1); else Keyboard::feed('A', 0);
            if (dpadRight) Keyboard::feed('D', 1); else Keyboard::feed('D', 0);
        }
    }
    catch (...)
    {
        // Gamepad input failed, continue without it
    }
}

bool UWPApplication::InitializeEGL()
{
    if (!m_platform)
    {
        return false;
    }
    
    // EGL is already initialized in AppPlatform_uwp
    // Just verify it's working
    EGLDisplay display = m_platform->getEGLDisplay();
    EGLSurface surface = m_platform->getEGLSurface();
    EGLContext context = m_platform->getEGLContext();
    
    if (display == EGL_NO_DISPLAY || surface == EGL_NO_SURFACE || context == EGL_NO_CONTEXT)
    {
        return false;
    }
    
    // Initialize OpenGL ES functions
    glInit();
    
    return true;
}

void UWPApplication::CleanupEGL()
{
    // EGL cleanup is handled in AppPlatform_uwp destructor
}

IFrameworkView^ UWPApplicationFactory::CreateView()
{
    return ref new UWPApplication();
}

// UWP application entry point
[Platform::MTAThread]
int __stdcall wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow)
{
    // Initialize COM for UWP
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE)
    {
        return -1;
    }
    
    try
    {
        // Create and run the UWP application
        auto factory = ref new UWPApplicationFactory();
        CoreApplication::Run(factory);
        
        // Cleanup COM
        CoUninitialize();
        return 0;
    }
    catch (Platform::Exception^ e)
    {
        CoUninitialize();
        return -1;
    }
    catch (std::exception& e)
    {
        CoUninitialize();
        return -1;
    }
    catch (...)
    {
        CoUninitialize();
        return -1;
    }
}

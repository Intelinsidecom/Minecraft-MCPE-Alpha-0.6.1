#ifndef MAIN_UWP_H__
#define MAIN_UWP_H__

#include "App.h"
#include "client/renderer/gles.h"
#include "platform/input/Mouse.h"
#include "platform/input/Keyboard.h"
#include "platform/input/Multitouch.h"
#include "platform/input/Controller.h"
#include "util/Mth.h"
#include "AppPlatform_uwp.h"
#include "NinecraftApp.h"
#include <EGL/egl.h>
#include <windows.foundation.h>
#include <windows.applicationmodel.core.h>
#include <windows.ui.core.h>
#include <windows.graphics.display.h>
#include <windows.system.threading.h>
#include <windows.applicationmodel.h>
#include <vccorlib.h>
#include <wrl/client.h>
#include <wrl/event.h>
#include <memory>

using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::Graphics::Display;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Windows::ApplicationModel;
using namespace Platform;
using namespace Microsoft::WRL;

extern App* g_app;
static volatile bool g_running = true;

ref class UWPApplication sealed : public IFrameworkView
{
public:
    UWPApplication();
    virtual ~UWPApplication();
    virtual void Initialize(CoreApplicationView^ applicationView);
    virtual void SetWindow(CoreWindow^ window);
    virtual void Load(Platform::String^ entryPoint);
    virtual void Run();
    virtual void Uninitialize();

private:
    CoreWindow^ m_window;
    DisplayInformation^ m_displayInfo;
    AppPlatform_uwp* m_platform;
    App* m_app;
    AppContext m_context;
    bool m_initialized;
    ThreadPoolTimer^ m_timer;
    void OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args);
    void OnSuspending(Platform::Object^ sender, SuspendingEventArgs^ args);
    void OnResuming(Platform::Object^ sender, Platform::Object^ args);
    void OnWindowSizeChanged(CoreWindow^ sender, WindowSizeChangedEventArgs^ args);
    void OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args);
    void OnDpiChanged(DisplayInformation^ sender, Platform::Object^ args);
    void InitializeGame();
    void UpdateGame();
    void RenderGame();
    void CleanupGame();
    void SetupInputHandlers();
    void ProcessGamepadInput();
    bool InitializeEGL();
    void CleanupEGL();
};

ref class UWPApplicationFactory sealed : public IFrameworkViewSource
{
public:
    virtual IFrameworkView^ CreateView();
};

#endif /*MAIN_UWP_H__*/

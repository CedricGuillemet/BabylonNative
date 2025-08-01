#include <Babylon/AppRuntime.h>
#include <Babylon/Graphics/Device.h>
#include <Babylon/ScriptLoader.h>
#include <Babylon/Plugins/NativeCapture.h>
#include <Babylon/Plugins/NativeEngine.h>
#include <Babylon/Plugins/NativeOptimizations.h>
//#include <Babylon/Plugins/NativeXr.h>
//#include <Babylon/Plugins/NativeCamera.h>
#include <Babylon/Plugins/NativeInput.h>
#include <Babylon/Plugins/TestUtils.h>
#include <Babylon/Polyfills/Console.h>
#include <Babylon/Polyfills/Window.h>
#include <Babylon/Polyfills/XMLHttpRequest.h>
#include <Babylon/Polyfills/Canvas.h>
#include <Babylon/ShaderCache.h>
#include <Babylon/DebugTrace.h>
#include <iostream>

#include <pybind11/pybind11.h>

namespace py = pybind11;

std::optional<Babylon::AppRuntime> runtime{};
std::optional<Babylon::Graphics::Device> device{};
std::optional<Babylon::Graphics::DeviceUpdate> update{};
Babylon::Plugins::NativeInput* nativeInput{};
std::optional<Babylon::Polyfills::Canvas> nativeCanvas{};

// A simple function to add two numbers
int add(int i, int j) {
    return i + j;
}

void helloWorld() {
    std::cout << "Hello from C++!" << std::endl;
}
void InitPlatform();

const char* GetLogLevelString(Babylon::Polyfills::Console::LogLevel logLevel)
{
    switch (logLevel)
    {
    case Babylon::Polyfills::Console::LogLevel::Log:
        return "Log";
    case Babylon::Polyfills::Console::LogLevel::Warn:
        return "Warn";
    case Babylon::Polyfills::Console::LogLevel::Error:
        return "Error";
    default:
        return "";
    }
}

void Uninitialize()
{
    if (device)
    {
        update->Finish();
        device->FinishRenderingCurrentFrame();
    }

    nativeCanvas.reset();
    nativeInput = {};
    runtime.reset();
    update.reset();
    device.reset();
}

void Initialize(/*HWND hWnd*/)
{
    Uninitialize();

    InitPlatform();
    /*
    RECT rect;
    if (!GetClientRect(hWnd, &rect))
    {
        return;
    }
    */
    Babylon::DebugTrace::EnableDebugTrace(true);
    Babylon::DebugTrace::SetTraceOutput([](const char* trace) {
        OutputDebugStringA(trace);
        OutputDebugStringA("\n");
        });

    auto width = 640;// static_cast<size_t>(rect.right - rect.left);
    auto height = 480;// static_cast<size_t>(rect.bottom - rect.top);

    Babylon::Graphics::Configuration graphicsConfig{};
    graphicsConfig.Window = 0;// hWnd;
    graphicsConfig.Width = width;
    graphicsConfig.Height = height;
    graphicsConfig.MSAASamples = 4;

    device.emplace(graphicsConfig);
    update.emplace(device->GetUpdate("update"));

    Babylon::ShaderCache::Enabled(true);

    device->StartRenderingCurrentFrame();
    update->Start();

    Babylon::AppRuntime::Options options{};

    options.EnableDebugger = true;

    options.UnhandledExceptionHandler = [](const Napi::Error& error) {
        std::ostringstream ss{};
        ss << "[Uncaught Error] " << Napi::GetErrorString(error) << std::endl;
        //OutputDebugStringA(ss.str().data());

        std::cerr << ss.str();
        std::cerr.flush();
        /*
        Babylon::Plugins::TestUtils::errorCode = -1;
        PostMessage(hWnd, WM_CLOSE, 0, 0);
        
        auto string = Napi::GetErrorString(error);
        //printf("Fatal: %s\n", string.c_str());
        std::cout << "Fatal : " << Napi::GetErrorString(error) << std::endl;
        */
        };

    runtime.emplace(options);

    runtime->Dispatch([](Napi::Env env) {
        device->AddToJavaScript(env);

        Babylon::Polyfills::Console::Initialize(env, [](const char* message, Babylon::Polyfills::Console::LogLevel logLevel) {
            std::ostringstream ss{};
            ss << "[" << GetLogLevelString(logLevel) << "] " << message << std::endl;
            //OutputDebugStringA(ss.str().data());

            std::cout << ss.str();
            std::cout.flush();
            
            });

        Babylon::Polyfills::Window::Initialize(env);

        Babylon::Polyfills::XMLHttpRequest::Initialize(env);

        nativeCanvas.emplace(Babylon::Polyfills::Canvas::Initialize(env));

        Babylon::Plugins::NativeEngine::Initialize(env);

        Babylon::Plugins::NativeOptimizations::Initialize(env);

        Babylon::Plugins::NativeCapture::Initialize(env);

        /*Babylon::Plugins::NativeCamera::Initialize(env);

        Babylon::Plugins::NativeXr::Initialize(env);
        */
        nativeInput = &Babylon::Plugins::NativeInput::CreateForJavaScript(env);

        });

    Babylon::ScriptLoader loader{ *runtime };
    //loader.LoadScript("app:///Scripts/ammo.js");
    // Commenting out recast.js for now because v8jsi is incompatible with asm.js.
    // loader.LoadScript("app:///Scripts/recast.js");
    loader.LoadScript("file://E:\\dev\\babylon\\BNPython\\build\\Apps\\Pybylon\\Debug\\Scripts\\babylon.max.js");
    loader.LoadScript("file://E:\\dev\\babylon\\BNPython\\build\\Apps\\Pybylon\\Debug\\Scripts\\babylonjs.loaders.js");
    loader.LoadScript("file://E:\\dev\\babylon\\BNPython\\build\\Apps\\Pybylon\\Debug\\Scripts\\babylonjs.materials.js");
    loader.LoadScript("file://E:\\dev\\babylon\\BNPython\\build\\Apps\\Pybylon\\Debug\\Scripts\\babylon.gui.js");
    loader.LoadScript("file://E:\\dev\\babylon\\BNPython\\build\\Apps\\Pybylon\\Debug\\Scripts\\main.js");
    //loader.LoadScript("app:///Scripts/meshwriter.min.js");
    /*
    std::vector<std::string> scripts = GetCommandLineArguments();
    if (scripts.empty())
    {
        loader.LoadScript("app:///Scripts/experience.js");
    }
    else
    {
        for (const auto& script : scripts)
        {
            loader.LoadScript(GetUrlFromPath(script));
        }

        loader.LoadScript("app:///Scripts/playground_runner.js");
    }
    */
}


void renderFrame() {
    if (device)
    {
        device->StartRenderingCurrentFrame();
        update->Start();
        update->Finish();
        device->FinishRenderingCurrentFrame();
    }
}


// Binding code
PYBIND11_MODULE(Pybylon, m) {
    m.def("add", &add, "A function that adds two numbers");
    m.def("helloWorld", &helloWorld, "Hi there!");
    
    m.def("initialize", &Initialize, "Initialize Babylon.js");
    m.def("renderFrame", &renderFrame, "Render a Babylon frame");
    
}
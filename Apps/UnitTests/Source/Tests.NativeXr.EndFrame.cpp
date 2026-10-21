#ifdef HAS_FAKE_XR

#include <gtest/gtest.h>

#include <Babylon/AppRuntime.h>
#include <Babylon/Graphics/Device.h>
#include <Babylon/Plugins/NativeXr.h>

#include <FakeXr.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <future>

extern Babylon::Graphics::Configuration g_deviceConfig;

// Regression test for the Android field crash (BabylonReactNative, Debug, render thread):
//
//   NativeXrImpl.cpp:410  NativeXr::Impl::EndFrame()  assert(m_sessionState->Frame != nullptr)
//
// This runs the real, unmodified Plugins/NativeXr sources (see UnitTestsNativeXr in CMakeLists.txt)
// against the fake xr backend in FakeXr/, driven by the real Graphics::Device frame loop and a
// JavaScript requestAnimationFrame loop, exactly like an AR app.
//
// Every frame, NativeXr::Impl::ScheduleFrame chains:
//   A  FrameStartScheduler (render thread)    BeginFrame(): Frame = Session->GetNextFrame(); acquires a frame completion scope.
//   B  runtime scheduler (JS thread)          runs the requestAnimationFrame callbacks, then drops its frame completion scope.
//   C  inline                                 error check.
//   D  AfterRenderScheduler (render thread)   EndFrame(): Frame->Render(); Frame.reset().
//
// The render thread's FinishRenderingCurrentFrame waits for the frame completion scopes to drain and then
// runs the pending AfterRender continuations. But arcana destroys B's callable (releasing the scope) *before*
// it runs C and enqueues D. If the render thread wakes up in that window, D misses the AfterRender tick of its
// own frame and runs one frame late: it consumes (and resets) the *next* frame, which makes that frame's own D
// hit EndFrame() with a null Frame. The window is a few instructions wide, so this takes many frames to hit,
// which is why the field crash only showed up after minutes of AR rendering. Rendering without vsync
// makes the frame rate high enough to hit it in seconds.
//
// A BabylonNative that keeps frame N's EndFrame paired with frame N does not crash and this test passes after
// the bounded run (BN_XR_SECONDS, 60 by default).
TEST(NativeXrFrameLifecycle, EndFrameAlwaysHasActiveFrame)
{
    Babylon::Graphics::Device device{g_deviceConfig};
    Babylon::AppRuntime runtime{};

    std::atomic<bool> sessionEnded{false};
    std::atomic<bool> sessionFailed{false};

    std::promise<void> initialized;
    runtime.Dispatch([&](Napi::Env env) {
        device.AddToJavaScript(env);
        Babylon::Plugins::NativeXr::Initialize(env);

        env.Global().Set("reportSessionEnded", Napi::Function::New(env, [&](const Napi::CallbackInfo&) { sessionEnded = true; }, "reportSessionEnded"));
        env.Global().Set("reportSessionFailed", Napi::Function::New(env, [&](const Napi::CallbackInfo&) { sessionFailed = true; }, "reportSessionFailed"));

        Napi::Eval(env, R"(
            // Babylon.js provides NativeXRFrame (nativeXRFrame.ts); the plugin only needs the wrapper class.
            _native.NativeXRFrame = function NativeXRFrame(nativeImpl) { this._nativeImpl = nativeImpl; };

            globalThis.xrTest = { session: undefined, stop: false };
            navigator.xr.requestSession("immersive-ar", {}).then(session => {
                xrTest.session = session;
                const onFrame = () => {
                    if (!xrTest.stop) {
                        session.requestAnimationFrame(onFrame);
                    }
                };
                session.requestAnimationFrame(onFrame);
            }, () => reportSessionFailed());
        )", "native-xr-session.js");
        initialized.set_value();
    });
    initialized.get_future().wait();

    auto& stats = xr::fake::GetStats();
    const auto begunBefore = stats.FramesBegun.load();
    const auto renderedBefore = stats.FramesRendered.load();

    const char* seconds = std::getenv("BN_XR_SECONDS");
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds{static_cast<int64_t>((seconds ? std::atof(seconds) : 60.0) * 1000)};

    // The session starts asynchronously on the render loop, then every frame requests the next one.
    while (std::chrono::steady_clock::now() < deadline && !sessionFailed)
    {
        device.StartRenderingCurrentFrame();
        device.FinishRenderingCurrentFrame();
    }

    ASSERT_FALSE(sessionFailed);
    EXPECT_GT(stats.FramesBegun.load(), begunBefore + 100) << "The XR session never got going";

    runtime.Dispatch([](Napi::Env env) {
        Napi::Eval(env, R"(
            xrTest.stop = true;
            xrTest.session.end().then(() => reportSessionEnded());
        )", "native-xr-end.js");
    });
    const auto endDeadline = std::chrono::steady_clock::now() + std::chrono::seconds{10};
    while (!sessionEnded && std::chrono::steady_clock::now() < endDeadline)
    {
        device.StartRenderingCurrentFrame();
        device.FinishRenderingCurrentFrame();
    }

    ASSERT_TRUE(sessionEnded) << "The XR session did not end";
    EXPECT_EQ(stats.FramesBegun.load() - begunBefore, stats.FramesRendered.load() - renderedBefore) << "Every XR frame that begins must be rendered";
}

#endif

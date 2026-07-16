import UIKit

@UIApplicationMain
class AppDelegate: UIResponder, UIApplicationDelegate {

    var window: UIWindow?

    /// Owned by the app: created in `application(_:didFinishLaunchingWithOptions:)`,
    /// torn down in `applicationWillTerminate`. The `ViewController` borrows
    /// this handle to construct its `BNView`.
    var runtime: BNRuntime?

    func application(_ application: UIApplication, didFinishLaunchingWithOptions launchOptions: [UIApplication.LaunchOptionsKey: Any]?) -> Bool {
        let runtimeOptions = BNRuntimeOptions()
        runtimeOptions.enableDebugger = true
        runtimeOptions.enableDebugTrace = true
        guard let runtime = BNRuntime(options: runtimeOptions) else {
            fatalError("Failed to construct BNRuntime")
        }

        // Queue the Babylon.js bootstrap scripts (shared with the other
        // Playground hosts via Apps/Playground/Shared/PlaygroundScripts.cpp),
        // then the playground experience script. They will run after the
        // first BNView attach completes engine initialization on the JS
        // thread, in submission order.
        PlaygroundBootstrap.loadScripts(runtime)

        // Scripts passed as launch arguments (e.g. the native validation suite
        // launched via `simctl launch ... app:///Scripts/validation_native.js`)
        // drive the test runner; with no script arguments the app loads the
        // interactive experience. Mirrors the macOS host's argv handling.
        let scripts = ViewController.testScriptArguments()
        if scripts.isEmpty {
            runtime.loadScript("app:///Scripts/experience.js")
        } else {
            // Run the whole suite (don't stop on the first failing pixel-diff)
            // so CI reports every test's pass/fail. No-op on other hosts.
            //
            // NOTE: no swapchain warm-up / render-count floor is set. Those were
            // once needed because the iOS Simulator's paravirtual Metal GPU could
            // hand back a black frame on the first framebuffer read-back, but the
            // real cause was bgfx's screenshot read-back not waiting for the GPU
            // on iOS; that is now fixed (the read-back blit is kicked and waited
            // on for all platforms). Warm-up/min-render-count frames also call
            // Scene.render(), which advances the deterministic animation clock
            // (Scene.useConstantAnimationDeltaTime = true), so they shifted every
            // animated test to a different pose than its reference image and
            // caused spurious pixel-diff failures. With them removed, iOS
            // compares the same frame as every other host.
            runtime.eval("globalThis.__validationContinueOnFailure = true;",
                sourceURL: "ios-validation-setup")
            for script in scripts {
                runtime.loadScript(script)
            }
            runtime.loadScript("app:///Scripts/playground_runner.js")
        }

        self.runtime = runtime
        return true
    }

    func applicationWillResignActive(_ application: UIApplication) {
        runtime?.suspend()
    }

    func applicationDidBecomeActive(_ application: UIApplication) {
        runtime?.resume()
    }

    func applicationWillTerminate(_ application: UIApplication) {
        runtime = nil
    }
}


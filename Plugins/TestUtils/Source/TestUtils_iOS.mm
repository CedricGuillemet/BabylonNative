#include "TestUtils.h"

#include <cstdlib>
#include <filesystem>

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <QuartzCore/CAMetalLayer.h>

namespace Babylon::Plugins::Internal
{
    namespace
    {
        // Documents directory inside the app sandbox. Writable at runtime and
        // retrievable from the host via
        // `xcrun simctl get_app_container <udid> <bundle-id> data`.
        NSString* ValidationDocumentsDirectory()
        {
            NSArray<NSString*>* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
            return paths.firstObject;
        }
    }

    void TestUtils::Exit(const Napi::CallbackInfo& info)
    {
        const int32_t exitCode = info[0].As<Napi::Number>().Int32Value();

        InvokeExitCallback(exitCode);

        // The simulator does not surface a launched app's process exit code to
        // the host `simctl launch`, so persist it to a sentinel file inside the
        // app's Documents container. A host-side harness reads it after the app
        // terminates (container path resolved via
        // `xcrun simctl get_app_container <udid> <bundle-id> data`).
        NSString* file = [ValidationDocumentsDirectory() stringByAppendingPathComponent:@"validation_exit_code.txt"];
        NSString* contents = [NSString stringWithFormat:@"%d\n", exitCode];
        [contents writeToFile:file atomically:YES encoding:NSUTF8StringEncoding error:nil];

        // Terminate with the test's exit code. Deferred to the main queue so
        // any in-flight console/log flush completes first.
        dispatch_async(dispatch_get_main_queue(), ^{
            std::exit(exitCode);
        });
    }

    void TestUtils::UpdateSize(const Napi::CallbackInfo& info)
    {
        const int32_t width = info[0].As<Napi::Number>().Int32Value();
        const int32_t height = info[1].As<Napi::Number>().Int32Value();

        // Resize the backing MTKView to the requested logical size so the
        // validation framebuffer readback matches the reference-image
        // dimensions (the reference images are authored at this exact size).
        // The layer's delegate is the MTKView (a UIView); forcing its enclosing
        // view controller to re-lay-out drives BNView.resize with the new
        // bounds. This assumes the host leaves the view frame-driven (no
        // full-screen Auto Layout constraints) in validation mode.
        dispatch_async(dispatch_get_main_queue(), ^{
            UIView* view = (UIView*)((__bridge CAMetalLayer*)m_window).delegate;
            if (view == nil)
            {
                return;
            }

            CGRect frame = view.frame;
            frame.size = CGSizeMake(width, height);
            view.frame = frame;

            // Changing a subview's frame does not by itself run the view
            // controller's -viewDidLayoutSubviews, so force a synchronous
            // layout pass on the enclosing view to deliver the new size to
            // BNView.
            [view.superview setNeedsLayout];
            [view.superview layoutIfNeeded];
        });
    }

    void TestUtils::SetTitle(const Napi::CallbackInfo& /*info*/)
    {
        // No title bar on iOS.
    }

    Napi::Value TestUtils::GetOutputDirectory(const Napi::CallbackInfo& info)
    {
        auto path = std::filesystem::path{[ValidationDocumentsDirectory() UTF8String]}.generic_string();
        return Napi::Value::From(info.Env(), path);
    }
}

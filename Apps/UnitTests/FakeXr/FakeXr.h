#pragma once

#include <atomic>
#include <cstdint>

// Test-only backend for the platform "xr" library (Dependencies/xr/Include/XR.h).
//
// The shipping NativeXr plugin is compiled against ARCore (Android), ARKit (iOS) or OpenXR. This
// library implements the same xr::System / Session / Frame API on top of nothing at all, so the
// real, unmodified Plugins/NativeXr sources can be linked into UnitTests on desktop and driven by
// the real Graphics::Device frame loop. It produces tracked frames without views, input sources or
// scene understanding, which is everything NativeXr's frame scheduling needs.
namespace xr::fake
{
    struct Stats
    {
        // Session::GetNextFrame calls that produced a frame (NativeXr::Impl::BeginFrame).
        std::atomic<uint64_t> FramesBegun{0};

        // Frame::Render calls (NativeXr::Impl::EndFrame).
        std::atomic<uint64_t> FramesRendered{0};
    };

    Stats& GetStats();
}

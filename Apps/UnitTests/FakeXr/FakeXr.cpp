#include "FakeXr.h"

#include <XR.h>

#include <stdexcept>

namespace xr
{
    namespace fake
    {
        Stats& GetStats()
        {
            static Stats stats;
            return stats;
        }
    }

    struct System::Impl
    {
        bool Initialized{false};
    };

    struct System::Session::Impl
    {
        std::vector<Frame::View> Views{};
        std::vector<Frame::InputSource> InputSources{};
        std::vector<FeaturePoint> FeaturePointCloud{};
        std::optional<Space> EyeTrackerSpace{};
        bool EndRequested{false};
    };

    struct System::Session::Frame::Impl
    {
    };

    System::System(const char*)
        : m_impl{std::make_unique<Impl>()}
    {
    }

    System::~System() = default;

    bool System::IsInitialized() const
    {
        return m_impl->Initialized;
    }

    bool System::TryInitialize()
    {
        m_impl->Initialized = true;
        return true;
    }

    arcana::task<bool, std::exception_ptr> System::IsSessionSupportedAsync(SessionType)
    {
        return arcana::task_from_result<std::exception_ptr>(true);
    }

    uintptr_t System::GetNativeXrContext()
    {
        return 0;
    }

    std::string System::GetNativeXrContextType()
    {
        return {};
    }

    arcana::task<std::shared_ptr<System::Session>, std::exception_ptr> System::Session::CreateAsync(System& system, void* graphicsDevice, void* commandQueue, std::function<void*()> windowProvider)
    {
        return arcana::task_from_result<std::exception_ptr>(std::make_shared<Session>(system, graphicsDevice, commandQueue, std::move(windowProvider)));
    }

    System::Session::Session(System&, void*, void*, std::function<void*()>)
        : m_impl{std::make_unique<Impl>()}
    {
    }

    System::Session::~Session() = default;

    std::unique_ptr<System::Session::Frame> System::Session::GetNextFrame(bool& shouldEndSession, bool& shouldRestartSession, std::function<arcana::task<void, std::exception_ptr>(void*)>)
    {
        // Like a real runtime, report the end of the session on the first frame after RequestEndSession.
        shouldEndSession = m_impl->EndRequested;
        shouldRestartSession = false;

        fake::GetStats().FramesBegun++;
        return std::make_unique<Frame>(*m_impl);
    }

    void System::Session::RequestEndSession()
    {
        m_impl->EndRequested = true;
    }

    void System::Session::SetDepthsNearFar(float, float)
    {
    }

    void System::Session::SetPlaneDetectionEnabled(bool) const
    {
    }

    bool System::Session::TrySetFeaturePointCloudEnabled(bool) const
    {
        return false;
    }

    bool System::Session::TrySetPreferredPlaneDetectorOptions(const GeometryDetectorOptions&)
    {
        return false;
    }

    bool System::Session::TrySetMeshDetectorEnabled(const bool)
    {
        return false;
    }

    bool System::Session::TrySetPreferredMeshDetectorOptions(const GeometryDetectorOptions&)
    {
        return false;
    }

    std::vector<ImageTrackingScore>* System::Session::GetImageTrackingScores() const
    {
        return nullptr;
    }

    void System::Session::CreateAugmentedImageDatabase(const std::vector<ImageTrackingRequest>&) const
    {
    }

    System::Session::Frame::Frame(System::Session::Impl& sessionImpl)
        : Views{sessionImpl.Views}
        , InputSources{sessionImpl.InputSources}
        , FeaturePointCloud{sessionImpl.FeaturePointCloud}
        , EyeTrackerSpace{sessionImpl.EyeTrackerSpace}
        , IsTracking{true}
        , m_impl{std::make_unique<Impl>()}
    {
    }

    System::Session::Frame::~Frame() = default;

    void System::Session::Frame::GetHitTestResults(std::vector<HitResult>&, Ray, HitTestTrackableType) const
    {
    }

    Anchor System::Session::Frame::CreateAnchor(Pose, NativeAnchorPtr) const
    {
        throw std::runtime_error{"The fake xr backend does not support anchors."};
    }

    Anchor System::Session::Frame::DeclareAnchor(NativeAnchorPtr) const
    {
        throw std::runtime_error{"The fake xr backend does not support anchors."};
    }

    void System::Session::Frame::UpdateAnchor(Anchor&) const
    {
    }

    void System::Session::Frame::DeleteAnchor(Anchor&) const
    {
    }

    System::Session::Frame::SceneObject& System::Session::Frame::GetSceneObjectByID(SceneObject::Identifier) const
    {
        throw std::runtime_error{"The fake xr backend does not track scene objects."};
    }

    System::Session::Frame::Plane& System::Session::Frame::GetPlaneByID(Plane::Identifier) const
    {
        throw std::runtime_error{"The fake xr backend does not track planes."};
    }

    System::Session::Frame::Mesh& System::Session::Frame::GetMeshByID(Mesh::Identifier) const
    {
        throw std::runtime_error{"The fake xr backend does not track meshes."};
    }

    System::Session::Frame::ImageTrackingResult& System::Session::Frame::GetImageTrackingResultByID(ImageTrackingResult::Identifier) const
    {
        throw std::runtime_error{"The fake xr backend does not track images."};
    }

    void System::Session::Frame::Render()
    {
        fake::GetStats().FramesRendered++;
    }
}

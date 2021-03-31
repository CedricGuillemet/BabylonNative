#pragma once

#include <jni.h>
#include <string>
#include <vector>
#include <cstddef>
#include <android/asset_manager.h>
#include <android/native_window.h>

// --------------------
// Forward Declarations
// --------------------

namespace java::lang
{
    class ByteArray;
    class Object;
    class String;
    class StringArray;
}

namespace java::io
{
    class ByteArrayOutputStream;
    class InputStream;
}

namespace java::net
{
    class HttpURLConnection;
    class URL;
    class URLConnection;
}

namespace android
{
    class ManifestPermission;
}

namespace android::app
{
    class Activity;
}

namespace android::content
{
    class Context;
}

namespace android::content::res
{
    class AssetManager;
}

namespace android::net
{
    class Uri;
}

namespace android::os
{
    class Handler;
    class HandlerThread;
    class Looper;
}

namespace android::hardware::camera2
{
    class CameraManager;
    class CaptureRequest;
    class CameraDevice;
    class CameraCaptureSession;
}

// ------------
// Declarations
// ------------

namespace java::lang
{
    class ByteArray
    {
    public:
        ByteArray(int size);
        ByteArray(jbyteArray byteArray);

        operator jbyteArray() const;

        operator std::vector<std::byte>() const;

    protected:
        JNIEnv* m_env;
        jbyteArray m_byteArray;
    };

    class Class
    {
    public:
        Class(const char* className);
        Class(const jclass classObj);

        operator jclass() const;

        bool IsAssignableFrom(Class otherClass);

    protected:
        JNIEnv* m_env;
        const jclass m_class;
    };

    class Object
    {
    public:
        operator jobject() const;
        Class GetClass();

    protected:
        Object(const char* className);
        Object(jobject object);

        JNIEnv* m_env;
        Class m_class;
        jobject m_object;
    };

    class String
    {
    public:
        String(jstring string);
        String(const char* string);

        operator jstring() const;

        operator std::string() const;

    protected:
        JNIEnv* m_env;
        jstring m_string;
    };

    class StringArray
    {
    public:
        StringArray(jobject array);

        size_t GetArrayLength() const;
        String GetObjectArrayElement(size_t index) const;

    protected:
        JNIEnv* m_env;
        jobjectArray m_stringArray;
    };

    class Throwable : public Object, public std::exception
    {
    public:
        Throwable(jthrowable throwable);
        ~Throwable();

        String GetMessage() const;

        const char* what() const noexcept override;

    private:
        jobject m_throwableRef;
        std::string m_message;
    };
}

namespace java::io
{
    class ByteArrayOutputStream : public lang::Object
    {
    public:
        ByteArrayOutputStream();
        ByteArrayOutputStream(int size);
        ByteArrayOutputStream(jobject object);

        void Write(lang::ByteArray b, int off, int len);

        lang::ByteArray ToByteArray() const;

        lang::String ToString(const char* charsetName) const;
    };

    class InputStream : public lang::Object
    {
    public:
        InputStream(jobject object);

        int Read(lang::ByteArray byteArray) const;
    };
}

namespace java::net
{
    class HttpURLConnection : public lang::Object
    {
    public:
        static lang::Class Class();

        HttpURLConnection(jobject object);

        int GetResponseCode() const;
    };

    class URL : public lang::Object
    {
    public:
        URL(jobject object);
        URL(lang::String url);

        URLConnection OpenConnection();

        lang::String ToString();
    };

    class URLConnection : public lang::Object
    {
    public:
        URLConnection(jobject object);

        void Connect();

        URL GetURL() const;

        int GetContentLength() const;

        io::InputStream GetInputStream() const;

        explicit operator HttpURLConnection() const;
    };
}

namespace android
{
    class ManifestPermission
    {
    public:
        static jstring CAMERA();

    private:
        static jstring getPermissionName(const char* permissionName);
    };
}

namespace android::app
{
    class Activity : public java::lang::Object
    {
    public:
        Activity(jobject object);

        void requestPermissions(jstring systemPermissionName, int permissionRequestID);
    };
}

namespace android::content
{
    class Context : public java::lang::Object
    {
    public:
        Context(jobject object);

        Context getApplicationContext();

        res::AssetManager getAssets() const;

        template<typename ServiceT>
        ServiceT getSystemService()
        {
            return {getSystemService(ServiceT::ServiceName)};
        };

        jobject getSystemService(const char* serviceName);

        bool checkSelfPermission(jstring systemPermissionName);
    };
}

namespace android::content::res
{
    class AssetManager : public java::lang::Object
    {
    public:
        AssetManager(jobject object);

        operator AAssetManager*() const;
    };
}

namespace android::graphics
{
    class SurfaceTexture : public java::lang::Object
    {
    public:
        SurfaceTexture();

        void initWithTexture(int texture);
        void updateTexture() const;
    };
}

namespace android::view
{
    class Display : public java::lang::Object
    {
    public:
        Display(jobject object);

        int getRotation();
    };

    class WindowManager : public java::lang::Object
    {
    public:
        static constexpr const char* ServiceName{"window"};
        WindowManager(jobject object);

        Display getDefaultDisplay();
    };

    class Surface : public java::lang::Object
    {
    public:
        Surface();

        void initWithSurfaceTexture(android::graphics::SurfaceTexture surfaceTexture);
        ANativeWindow* getNativeWindow();
    };
}

namespace android::net
{
    class Uri : public java::lang::Object
    {
    public:
        Uri(jobject object);

        java::lang::String getScheme() const;

        java::lang::String getPath() const;

        static Uri Parse(java::lang::String uriString);
    };
}

namespace android::os
{
    class Looper : public java::lang::Object
    {
    public:
        Looper(jobject object);
    };

    class Handler : public java::lang::Object
    {
    public:
        Handler() : Object{"android/os/Handler"} {}
        Handler(Looper looper);
    };

    class HandlerThread : public java::lang::Object
    {
    public:
        HandlerThread() : Object{"android/os/HandlerThread"} {}
        HandlerThread(java::lang::String name);
        void Start();
        void Join();
        Looper getLooper();
    };
}

namespace java::BabylonNative
{
    class BabylonNativeCameraStateCallback : public java::lang::Object
    {
    public:
        BabylonNativeCameraStateCallback() : Object{"BabylonNative/BabylonNativeCameraStateCallback"} {}
        BabylonNativeCameraStateCallback(jobject object) : Object(object) {}
        template<typename T>
        BabylonNativeCameraStateCallback(T* host) : Object{"BabylonNative/BabylonNativeCameraStateCallback"}
        {
            m_object = m_env->NewObject(callbackClass, newMethod, reinterpret_cast<jlong>(host));
        }

        static inline jclass callbackClass{};
        static inline jmethodID newMethod{};
    };
}


namespace android::hardware::camera2
{
    namespace Params
    {
        class SessionConfiguration : public java::lang::Object
        {
        public:
            SessionConfiguration() : Object{"android/hardware/camera2/Params/SessionConfiguration"} {}
        };
    }

    class CaptureRequest : public java::lang::Object
    {
    public:
        CaptureRequest() : Object{"android/hardware/camera2/CaptureRequest"} {}

        class Builder : public java::lang::Object
        {
        public:
            Builder(jobject object) : Object(object) {}
            //Builder() : Object{"android/hardware/camera2/CaptureRequest/Builder"} {}
            void addTarget(android::view::Surface outputTarget);
        };

    };

    class CameraDevice : public java::lang::Object
    {
    public:
        CameraDevice() : Object{"android/hardware/camera2/CameraDevice"} {}
        CaptureRequest::Builder createCaptureRequest(int templateType);
        void createCaptureSession(Params::SessionConfiguration config);
        void close();
#if 0
        
#endif
    };

    class CameraCharacteristics  : public java::lang::Object
    {
    public:
        CameraCharacteristics(jobject object) : Object(object) {}
        template<typename T> T get(const char* fieldName);

        static constexpr auto LENS_FACING_FRONT = 0;
        static constexpr auto LENS_FACING_BACK = 1;
    };

    class CameraCaptureSession : public java::lang::Object
    {
    public:
        CameraCaptureSession() : Object{"android/hardware/camera2/CameraCaptureSession"} {}
        void close();
    };

    class CameraManager : public java::lang::Object
    {
    public:
        CameraManager(jobject object) : Object(object) {}
        void openCamera(java::lang::String cameraId, java::BabylonNative::BabylonNativeCameraStateCallback callback, android::os::Handler handler);
        java::lang::StringArray getCameraIdList();
        CameraCharacteristics getCameraCharacteristics(java::lang::String cameraId);

        static constexpr const char* ServiceName{"camera"};
    };
    
}


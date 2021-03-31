#include <AndroidExtensions/JavaWrappers.h>
#include <AndroidExtensions/Globals.h>
#include <android/surface_texture.h>
#include <android/surface_texture_jni.h>
#include <android/asset_manager_jni.h>
#include <android/native_window_jni.h>

using namespace android::global;

namespace
{
    void ThrowIfFaulted(JNIEnv* env)
    {
        if (env->ExceptionCheck())
        {
            auto jthrowable{env->ExceptionOccurred()};
            env->ExceptionClear();
            throw java::lang::Throwable{jthrowable};
        }
    }
}

namespace java::lang
{
    ByteArray::ByteArray(int size)
        : m_env{GetEnvForCurrentThread()}
        , m_byteArray{m_env->NewByteArray(size)}
    {
    }

    ByteArray::ByteArray(jbyteArray byteArray)
        : m_env{GetEnvForCurrentThread()}
        , m_byteArray{byteArray}
    {
    }

    ByteArray::operator jbyteArray() const
    {
        return m_byteArray;
    }

    ByteArray::operator std::vector<std::byte>() const
    {
        std::vector<std::byte> result{static_cast<size_t>(m_env->GetArrayLength(m_byteArray))};
        std::memcpy(result.data(), m_env->GetByteArrayElements(m_byteArray, nullptr), result.size());
        return result;
    }

    Class::Class(const char* className)
        : m_env{GetEnvForCurrentThread()}
        , m_class{m_env->FindClass(className)}
    {
    }

    Class::Class(const jclass classObj)
        : m_env{GetEnvForCurrentThread()}
        , m_class{classObj}
    {
    }

    Class::operator jclass() const
    {
        return m_class;
    };

    bool Class::IsAssignableFrom(Class otherClass)
    {
        return m_env->IsAssignableFrom(m_class, otherClass.m_class);
    };

    Object::operator jobject() const
    {
        return m_object;
    }

    Object::Object(const char* className)
        : m_env{GetEnvForCurrentThread()}
        , m_class{m_env->FindClass(className)}
        , m_object{nullptr}
    {
    }

    Object::Object(jobject object)
            : m_env{GetEnvForCurrentThread()}
            , m_class{m_env->GetObjectClass(object)}
            , m_object{object}
    {
    }

    Class Object::GetClass()
    {
        return m_class;
    }

    String::String(jstring string)
        : m_env{GetEnvForCurrentThread()}
        , m_string{string}
    {
    }

    String::String(const char* string)
        : m_env{GetEnvForCurrentThread()}
        , m_string{m_env->NewStringUTF(string)}
    {
    }

    String::operator jstring() const
    {
        return m_string;
    }

    String::operator std::string() const
    {
        const char* buffer{m_env->GetStringUTFChars(m_string, nullptr)};
        std::string str{buffer};
        m_env->ReleaseStringUTFChars(m_string, buffer);
        return str;
    }

    StringArray::StringArray(jobject array)
        : m_env{GetEnvForCurrentThread()}
        , m_stringArray(static_cast<jobjectArray>(array))
    {
    }

    size_t StringArray::GetArrayLength() const
    {
        return m_env->GetArrayLength(m_stringArray);
    }

    String StringArray::GetObjectArrayElement(size_t index) const
    {
        return {(jstring)m_env->GetObjectArrayElement(m_stringArray, index)};
    }

    Throwable::Throwable(jthrowable throwable)
        : Object{throwable}
        , m_throwableRef{m_env->NewGlobalRef(throwable)}
        , m_message{GetMessage()}
    {
    }

    Throwable::~Throwable()
    {
        m_env->DeleteGlobalRef(m_throwableRef);
    }

    String Throwable::GetMessage() const
    {
        return {(jstring)m_env->CallObjectMethod(m_object, m_env->GetMethodID(m_class, "getMessage", "()Ljava/lang/String;"))};
    }

    const char* Throwable::what() const noexcept
    {
        return m_message.c_str();
    }
}

namespace java::io
{
    ByteArrayOutputStream::ByteArrayOutputStream()
        : Object{"java/io/ByteArrayOutputStream"}
    {
        m_object = m_env->NewObject(m_class, m_env->GetMethodID(m_class, "<init>", "()V"));
    }

    ByteArrayOutputStream::ByteArrayOutputStream(int size)
        : Object{"java/io/ByteArrayOutputStream"}
    {
        m_object = m_env->NewObject(m_class, m_env->GetMethodID(m_class, "<init>", "(I)V"), size);
    }

    ByteArrayOutputStream::ByteArrayOutputStream(jobject object)
        : Object{object}
    {
    }

    void ByteArrayOutputStream::Write(lang::ByteArray b, int off, int len)
    {
        m_env->CallVoidMethod(m_object, m_env->GetMethodID(m_class, "write", "([BII)V"), (jbyteArray)b, off, len);
    }

    lang::ByteArray ByteArrayOutputStream::ToByteArray() const
    {
        return {(jbyteArray)m_env->CallObjectMethod(m_object, m_env->GetMethodID(m_class, "toByteArray", "()[B"))};
    }

    lang::String ByteArrayOutputStream::ToString(const char* charsetName) const
    {
        jmethodID method{m_env->GetMethodID(m_class, "toString", "(Ljava/lang/String;)Ljava/lang/String;")};
        return {(jstring)m_env->CallObjectMethod(m_object, method, m_env->NewStringUTF(charsetName))};
    }

    InputStream::InputStream(jobject object)
        : Object{object}
    {
    }

    int InputStream::Read(lang::ByteArray byteArray) const
    {
        return m_env->CallIntMethod(m_object, m_env->GetMethodID(m_class, "read", "([B)I"), (jbyteArray)byteArray);
    }
}

namespace java::net
{
    HttpURLConnection::HttpURLConnection(jobject object)
        : Object{object}
    {
    }

    lang::Class HttpURLConnection::Class()
    {
        return {"java/net/HttpURLConnection"};
    }

    int HttpURLConnection::GetResponseCode() const
    {
        auto responseCode = m_env->CallIntMethod(m_object, m_env->GetMethodID(m_class, "getResponseCode", "()I"));
        ThrowIfFaulted(m_env);
        return responseCode;
    }

    URL::URL(lang::String url)
        : Object{"java/net/URL"}
    {
        m_object = m_env->NewObject(m_class, m_env->GetMethodID(m_class, "<init>", "(Ljava/lang/String;)V"), (jstring)url);
    }

    URL::URL(jobject object)
        : Object{object}
    {
    }

    URLConnection URL::OpenConnection()
    {
        auto urlConnection{m_env->CallObjectMethod(m_object, m_env->GetMethodID(m_class, "openConnection", "()Ljava/net/URLConnection;"))};
        ThrowIfFaulted(m_env);
        return {urlConnection};
    }

    lang::String URL::ToString()
    {
        return {(jstring)m_env->CallObjectMethod(m_object, m_env->GetMethodID(m_class, "toString", "()Ljava/lang/String;"))};
    }

    URLConnection::URLConnection(jobject object)
        : Object{object}
    {
    }

    void URLConnection::Connect()
    {
        m_env->CallVoidMethod(m_object, m_env->GetMethodID(m_class, "connect", "()V"));
        ThrowIfFaulted(m_env);
    }

    URL URLConnection::GetURL() const
    {
        return {m_env->CallObjectMethod(m_object, m_env->GetMethodID(m_class, "getURL", "()Ljava/net/URL;"))};
    }

    int URLConnection::GetContentLength() const
    {
        return m_env->CallIntMethod(m_object, m_env->GetMethodID(m_class, "getContentLength", "()I"));
    }

    io::InputStream URLConnection::GetInputStream() const
    {
        auto inputStream{m_env->CallObjectMethod(m_object, m_env->GetMethodID(m_class, "getInputStream", "()Ljava/io/InputStream;"))};
        ThrowIfFaulted(m_env);
        return {inputStream};
    }

    URLConnection::operator HttpURLConnection() const
    {
        return {m_object};
    }
}

namespace android
{
    jstring ManifestPermission::CAMERA()
    {
        return getPermissionName("CAMERA");
    }

    jstring ManifestPermission::getPermissionName(const char* permissionName)
    {
        JNIEnv* env{GetEnvForCurrentThread()};
        jclass cls{env->FindClass("android/Manifest$permission")};
        jfieldID permId{env->GetStaticFieldID(cls, permissionName, "Ljava/lang/String;")};
        return (jstring)env->GetStaticObjectField(cls, permId);
    }
}

namespace android::app
{
    Activity::Activity(jobject object)
        : Object{object}
    {
    }

    void Activity::requestPermissions(jstring systemPermissionName, int permissionRequestID)
    {
        jobjectArray permissionArray{m_env->NewObjectArray(
            1,
            m_env->FindClass("java/lang/String"),
            systemPermissionName)};
        m_env->CallVoidMethod(m_object, m_env->GetMethodID(m_class, "requestPermissions", "([Ljava/lang/String;I)V"), permissionArray, permissionRequestID);
        m_env->DeleteLocalRef(permissionArray);
    }
}

namespace android::content
{
    Context::Context(jobject object)
        : Object{object}
    {
    }

    Context Context::getApplicationContext()
    {
        return {m_env->CallObjectMethod(m_object, m_env->GetMethodID(m_class, "getApplicationContext", "()Landroid/content/Context;"))};
    }

    res::AssetManager Context::getAssets() const
    {
        return {m_env->CallObjectMethod(m_object, m_env->GetMethodID(m_class, "getAssets", "()Landroid/content/res/AssetManager;"))};
    }

    jobject Context::getSystemService(const char* serviceName)
    {
        return m_env->CallObjectMethod(m_object, m_env->GetMethodID(m_class, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;"), m_env->NewStringUTF(serviceName));
    }

    bool Context::checkSelfPermission(jstring systemPermissionName)
    {
        // Get the package manager, and get the value that represents a successful permission grant.
        jclass packageManager{m_env->FindClass("android/content/pm/PackageManager")};
        jfieldID permissionGrantedId{m_env->GetStaticFieldID(packageManager, "PERMISSION_GRANTED", "I")};
        jint permissionGrantedValue{m_env->GetStaticIntField(packageManager, permissionGrantedId)};

        // Perform the actual permission check by checking against the android context object.
        jint permissionCheckResult{m_env->CallIntMethod(m_object, m_env->GetMethodID(m_class, "checkSelfPermission", "(Ljava/lang/String;)I"), systemPermissionName)};
        return permissionGrantedValue == permissionCheckResult;
    }
}

namespace android::content::res
{
    AssetManager::AssetManager(jobject object)
        : Object(object)
    {
    }

    AssetManager::operator AAssetManager*() const
    {
        return AAssetManager_fromJava(m_env, m_object);
    }
}

namespace android::view
{
    Display::Display(jobject object)
            : Object(object)
    {
    }

    int Display::getRotation()
    {
        return m_env->CallIntMethod(m_object, m_env->GetMethodID(m_class, "getRotation", "()I"));
    }

    WindowManager::WindowManager(jobject object)
        : Object(object)
    {
    }

    Display WindowManager::getDefaultDisplay()
    {
        return {m_env->CallObjectMethod(m_object, m_env->GetMethodID(m_class, "getDefaultDisplay", "()Landroid/view/Display;"))};
    }

    Surface::Surface()
        : Object("android/view/Surface")
    {
    }

    void Surface::initWithSurfaceTexture(android::graphics::SurfaceTexture surfaceTexture)
    {
        jobject surfaceTextureObject = surfaceTexture;
        m_object = m_env->NewObject(m_class, m_env->GetMethodID(m_class, "<init>", "(Landroid/graphics/SurfaceTexture;)V"), surfaceTextureObject);
    }

    ANativeWindow* Surface::getNativeWindow()
    {
        return ANativeWindow_fromSurface(m_env, m_object);
    }
}

namespace android::os
{
    HandlerThread::HandlerThread(java::lang::String name)
        : Object{"android/os/HandlerThread"}
    {
        m_object = m_env->NewObject(m_class, m_env->GetMethodID(m_class, "<init>", "(Ljava/lang/String;)V"), (jstring)name);
    }

    void HandlerThread::Start()
    {
        m_env->CallVoidMethod(m_class, m_env->GetMethodID(m_class, "start", "()V"));
    }

    void HandlerThread::Join()
    {
        m_env->CallVoidMethod(m_class, m_env->GetMethodID(m_class, "join", "()V"));
    }

    Looper HandlerThread::getLooper()
    {
        return m_env->CallObjectMethod(m_class, m_env->GetMethodID(m_class, "getLooper", "()Landroid/os/Looper;"));
    }

    Looper::Looper(jobject object)
        : Object{object}
    {
    }

    Handler::Handler(Looper looper)
        : Object{"android/os/Handler"}
    {
        m_object = m_env->NewObject(m_class, m_env->GetMethodID(m_class, "<init>", "(Landroid/os/Looper;)V"), looper);
    }
}

namespace android::net
{
    Uri::Uri(jobject object)
        : Object{object}
    {
    }

    java::lang::String Uri::getScheme() const
    {
        return {(jstring)m_env->CallObjectMethod(m_object, m_env->GetMethodID(m_class, "getScheme", "()Ljava/lang/String;"))};
    }

    java::lang::String Uri::getPath() const
    {
        return {(jstring)m_env->CallObjectMethod(m_object, m_env->GetMethodID(m_class, "getPath", "()Ljava/lang/String;"))};
    }

    Uri Uri::Parse(java::lang::String uriString)
    {
        JNIEnv* env{GetEnvForCurrentThread()};
        jclass cls{env->FindClass("android/net/Uri")};
        return {env->CallStaticObjectMethod(cls, env->GetStaticMethodID(cls, "parse", "(Ljava/lang/String;)Landroid/net/Uri;"), (jstring)uriString)};
    }
}

namespace android::graphics
{
    SurfaceTexture::SurfaceTexture()
        : Object("android/graphics/SurfaceTexture")
    {
    }

    void SurfaceTexture::initWithTexture(int texture)
    {
        m_object = m_env->NewObject(m_class, m_env->GetMethodID(m_class, "<init>", "(I)V"), texture);
    }

    void SurfaceTexture::updateTexture() const
    {
        if (m_object) {
            m_env->CallVoidMethod(m_object, m_env->GetMethodID(m_class, "updateTexImage", "()V"));
        }
    }
}

namespace android::hardware::camera2
{
    CaptureRequest::Builder CameraDevice::createCaptureRequest(int templateType)
    {
        return {m_env->CallObjectMethod(m_object, m_env->GetMethodID(m_class, "createCaptureRequest", "(I)Landroid/hardware/camera2/CaptureRequest$Builder;"), templateType)};
    }

    void CameraDevice::createCaptureSession(Params::SessionConfiguration config)
    {
        m_env->CallVoidMethod(m_object, m_env->GetMethodID(m_class, "createCaptureSession", "(Landroid/hardware/camera2/params/SessionConfiguration;)V"), config);
    }

    void CameraDevice::close()
    {
        m_env->CallVoidMethod(m_object, m_env->GetMethodID(m_class, "close", "()V"));
    }

    template <>
    int CameraCharacteristics::get(const char* fieldName)
    {
        jfieldID field = m_env->GetStaticFieldID(m_class, fieldName, "Landroid/hardware/camera2/CameraCharacteristics$Key;");
        if (field)
        {
            auto key{m_env->GetStaticObjectField(m_class, field)};
            auto getMethod{m_env->GetMethodID(m_class, "get", "(Landroid/hardware/camera2/CameraCharacteristics$Key;)Ljava/lang/Object;")};
            auto value{m_env->CallObjectMethod(m_object, getMethod, key)};
            auto classInteger{m_env->FindClass("java/lang/Integer")};
            auto intValue{m_env->CallIntMethod(value, m_env->GetMethodID(classInteger, "intValue", "()I"))};
            return intValue;
        }
        return 0;
    }

    void CameraCaptureSession::close()
    {
        m_env->CallVoidMethod(m_object, m_env->GetMethodID(m_class, "close", "()V"));
    }

    void CameraManager::openCamera(java::lang::String cameraId, java::BabylonNative::BabylonNativeCameraStateCallback callback, android::os::Handler handler)
    {
        jmethodID method{m_env->GetMethodID(m_class, "openCamera", "(Ljava/lang/String;Landroid/hardware/camera2/CameraDevice$StateCallback;Landroid/os/Handler;)V")};
        m_env->CallVoidMethod(m_object, method, jstring{cameraId}, (jobject)callback, handler);
    }

    java::lang::StringArray CameraManager::getCameraIdList()
    {
        return {m_env->CallObjectMethod(m_object, m_env->GetMethodID(m_class, "getCameraIdList", "()[Ljava/lang/String;"))};
    }

    CameraCharacteristics CameraManager::getCameraCharacteristics(java::lang::String cameraId)
    {
        jmethodID method{m_env->GetMethodID(m_class, "getCameraCharacteristics", "(Ljava/lang/String;)Landroid/hardware/camera2/CameraCharacteristics;")};
        return m_env->CallObjectMethod(m_object, method, jstring{cameraId});
    }

}

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* /*reserved*/)
{
    JNIEnv* env = NULL;
    jint result = -1;
    
	if (vm->GetEnv((void**) &env, JNI_VERSION_1_6) != JNI_OK) {
        return result;
    }

    java::BabylonNative::BabylonNativeCameraStateCallback::callbackClass = env->FindClass("BabylonNative/CameraStateCallback");
    java::BabylonNative::BabylonNativeCameraStateCallback::newMethod = env->GetMethodID(java::BabylonNative::BabylonNativeCameraStateCallback::callbackClass, "<init>", "(J)V");

    return JNI_VERSION_1_6;
}
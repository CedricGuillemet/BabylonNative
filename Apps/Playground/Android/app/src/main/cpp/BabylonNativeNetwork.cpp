#include <jni.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <memory>
#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include <Babylon/AppRuntime.h>
#include <iostream>
#include "BabylonNativeLooper.h"

jclass networkClass{};
jmethodID loadURLStringMethod{};
jmethodID loadURLBufferMethod{};
JavaVM* g_jvm{};

extern AAssetManager* g_assetMgrNative;
extern ALooper* g_mainThreadLooper;

extern "C" jint JNI_OnLoad(JavaVM* jvm, void *reserved)
{
    JNIEnv* env;
    if (jvm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK)
    {
        return JNI_ERR;
    }
    g_jvm = jvm;
    networkClass = env->FindClass("BabylonNative/Network");
    networkClass = static_cast<jclass>(env->NewGlobalRef(networkClass));
    loadURLStringMethod = env->GetStaticMethodID(networkClass, "loadURLString", "(Ljava/lang/String;)Ljava/lang/String;");
    loadURLBufferMethod = env->GetStaticMethodID(networkClass, "loadURLBuffer", "(Ljava/lang/String;)[B");

    return JNI_VERSION_1_6;
}

namespace
{
    JNIEnv* AcquireEnvFromJVM(JavaVM* jvm)
    {
        JNIEnv * env;
        int getEnvStat = jvm->GetEnv((void **)&env, JNI_VERSION_1_6);
        if (getEnvStat == JNI_EDETACHED)
        {
            log("GetEnv: not attached");
            if (g_jvm->AttachCurrentThread(&env, NULL) != 0)
            {
                log("Failed to attach Env");
                return nullptr;
            }
        } else if (getEnvStat == JNI_EVERSION)
        {
            log("GetEnv: version not supported");
            return nullptr;
        }
        return env;
    }

    template<typename T> bool AndroidLoadAsset(const std::string& url, T& data)
    {
        AAsset* asset = AAssetManager_open(g_assetMgrNative, url.c_str(),
            AASSET_MODE_UNKNOWN);
        if (asset)
        {
            size_t size = AAsset_getLength64(asset);
            data.resize(size);
            AAsset_read(asset, data.data(), size);
            AAsset_close(asset);
            return true;
        }
        return false;
    }
}

namespace Babylon
{
    std::string AndroidGetAbsoluteUrl(const std::string& url, const std::string& rootUrl)
    {
        AAsset* asset = AAssetManager_open(g_assetMgrNative, url.c_str(),
            AASSET_MODE_UNKNOWN);
        if (asset)
        {
            AAsset_close(asset);
            return url;
        }
        if (!rootUrl.empty())
        {
            return rootUrl + "/" + url;
        }
        return url;
    }

    void AndroidLoadURLFunction(const std::string& url, std::function<void(const std::string& content)> contentHandler)
    {
        std::string content;
        if (AndroidLoadAsset(url, content))
        {
            contentHandler(content);
        }
        else
        {
            DispatchLooperFunction<DispatchPayloadDirect>(g_mainThreadLooper, [url, contentHandler = std::move(contentHandler)](){
                JNIEnv * env = AcquireEnvFromJVM(g_jvm);
                assert(env);
                auto urlString = env->NewStringUTF(url.c_str());
                jstring javaString = (jstring)env->CallStaticObjectMethod(networkClass, loadURLStringMethod, urlString);
                const char *nativeString = env->GetStringUTFChars(javaString, 0);
                std::string content(nativeString);
                contentHandler(content);
                env->ReleaseStringUTFChars(javaString, nativeString);
            });
        }
    }

    void AndroidLoadURLFunction(const std::string& url, std::function<void(const std::vector<uint8_t>& content)> contentHandler)
    {
        std::vector<uint8_t> content;
        if (AndroidLoadAsset(url, content))
        {
            contentHandler(content);
        }
        else
        {
            DispatchLooperFunction<DispatchPayloadDirect>(g_mainThreadLooper, [url, contentHandler = std::move(contentHandler)](){
                JNIEnv * env = AcquireEnvFromJVM(g_jvm);
                assert(env);
                auto urlString = env->NewStringUTF(url.c_str());
                jbyteArray javaByteArray = (jbyteArray)env->CallStaticObjectMethod(networkClass, loadURLBufferMethod, urlString);
                int contentLength = env->GetArrayLength(javaByteArray);
                jboolean isCopy;
                jbyte* bytes = env->GetByteArrayElements(javaByteArray, &isCopy);
                std::vector<uint8_t> content((uint8_t*)bytes, (uint8_t*)bytes + contentLength);
                contentHandler(content);
                env->ReleaseByteArrayElements(javaByteArray, bytes, 0);
            });
        }
    }
}
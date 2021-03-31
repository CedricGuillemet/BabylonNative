package BabylonNative;

import android.hardware.camera2.CameraDevice;

public class CameraStateCallback extends android.hardware.camera2.CameraDevice.StateCallback {

    // JNI interface
    public static native void NativeCameraClosed(CameraDevice camera, long interfacePtr);
    public static native void NativeCameraDisconnected(CameraDevice camera, long interfacePtr);
    public static native void NativeCameraError(CameraDevice camera, int error, long interfacePtr);
    public static native void NativeCameraOpened(CameraDevice camera, long interfacePtr);

    protected long _interfacePtr;

    public CameraStateCallback(long interfacePtr) {
        _interfacePtr = interfacePtr;
    }

    public void onClosed(CameraDevice camera) {
        NativeCameraClosed(camera, _interfacePtr);
    }
    public void onDisconnected(CameraDevice camera) {
        NativeCameraDisconnected(camera, _interfacePtr);
    }

    public void onError(CameraDevice camera, int error) {
        NativeCameraError(camera, error, _interfacePtr);
    }

    public void onOpened(CameraDevice camera) {
        NativeCameraOpened(camera, _interfacePtr);
    }
}
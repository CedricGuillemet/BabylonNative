package com.android.babylonnative.playground;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;

import com.babylonjs.embedding.BabylonNative;
import com.library.babylonnative.BabylonView;

public class PlaygroundActivity extends Activity {
    /**
     * Bridges to {@code Apps/Playground/Shared/PlaygroundScripts.cpp},
     * which holds the bootstrap script list shared with the other
     * Playground hosts. Implemented in PlaygroundJNI.cpp.
     */
    private static native void loadBootstrapScripts(long runtimeHandle);

    private long mRuntimeHandle = 0;
    private BabylonView mView;

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        // Register Context/Activity with AndroidExtensions::Globals (used
        // by NativeCamera, NativeXr). Process-wide, not per-view. The JNI
        // layer guards against double-initialization.
        BabylonNative.setContext(getApplication());
        BabylonNative.setCurrentActivity(this);

        // Activity owns the Runtime lifetime; the View only borrows it.
        BabylonNative.RuntimeOptions runtimeOptions = new BabylonNative.RuntimeOptions();
        runtimeOptions.enableDebugger = true;
        runtimeOptions.enableDebugTrace = true;
        mRuntimeHandle = BabylonNative.runtimeCreate(runtimeOptions);

        // Queue the bootstrap scripts + the launch script(s). They run after
        // the first View attach completes engine init on the JS thread, in
        // submission order.
        //
        // Scripts passed via the launch Intent's "scripts" string extra
        // (comma-separated app:/// urls, e.g. the native validation suite
        // launched via `adb shell am start ... --es scripts
        // app:///Scripts/validation_native.js`) drive the test runner; with
        // no script extra the app loads the interactive experience. Mirrors
        // the iOS/macOS hosts' argv handling.
        loadBootstrapScripts(mRuntimeHandle);

        // Optional validation-suite knobs injected as JS globals before the
        // test runner executes (validation_native.js reads them at load time).
        // Mirrors the _playgroundOptions/__validation* globals the desktop and
        // iOS hosts inject. Currently: continueOnFailure keeps the suite
        // running past a failing test so every failure is enumerated in one
        // pass (instead of aborting at the first one):
        //   adb shell am start ... --ez continueOnFailure true
        if (getIntent().getBooleanExtra("continueOnFailure", false)) {
            BabylonNative.runtimeEval(mRuntimeHandle,
                    "globalThis.__validationContinueOnFailure = true;",
                    "app:///validation_options.js");
        }

        String[] scripts = null;
        String scriptsExtra = getIntent().getStringExtra("scripts");
        if (scriptsExtra != null && !scriptsExtra.trim().isEmpty()) {
            scripts = scriptsExtra.split(",");
        }

        if (scripts != null && scripts.length > 0) {
            for (String script : scripts) {
                String url = script.trim();
                if (!url.isEmpty()) {
                    BabylonNative.runtimeLoadScript(mRuntimeHandle, url);
                }
            }
        } else {
            BabylonNative.runtimeLoadScript(mRuntimeHandle, "app:///Scripts/experience.js");
        }

        mView = new BabylonView(getApplication(), mRuntimeHandle);

        // Optional fixed render-surface size (e.g. the validation suite needs
        // an exact framebuffer size that matches its reference images):
        //   adb shell am start ... --ei renderWidth 600 --ei renderHeight 400
        int renderWidth = getIntent().getIntExtra("renderWidth", 0);
        int renderHeight = getIntent().getIntExtra("renderHeight", 0);
        if (renderWidth > 0 && renderHeight > 0) {
            mView.setFixedSurfaceSize(renderWidth, renderHeight);
        }

        setContentView(mView);
    }

    @Override
    protected void onPause() {
        // Hide the view to stop its draw loop; onWindowFocusChanged
        // restores visibility on return.
        mView.setVisibility(View.GONE);

        // Process-wide: every Runtime auto-suspends (each subscribed in
        // runtimeCreate); cross-cutting subsystems (NativeCamera,
        // NativeXr) also hook this via AndroidExtensions::Globals.
        BabylonNative.pause();
        super.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        BabylonNative.resume();
    }

    @Override
    protected void onDestroy() {
        // Surface lifecycle (view detach) has already fired; just
        // release the Runtime.
        if (mRuntimeHandle != 0) {
            BabylonNative.runtimeDestroy(mRuntimeHandle);
            mRuntimeHandle = 0;
        }
        super.onDestroy();
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] results) {
        BabylonNative.requestPermissionsResult(requestCode, permissions, results);
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus && mView.getVisibility() == View.GONE) {
            mView.setVisibility(View.VISIBLE);
        }
    }
}

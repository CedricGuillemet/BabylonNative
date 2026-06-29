package com.library.babylonnative;

import android.content.Context;
import android.graphics.Canvas;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.FrameLayout;

import com.babylonjs.embedding.BabylonNative;

/**
 * Playground View built on {@link BabylonNative}. Borrows a Runtime handle
 * from the host (which owns the Runtime's lifetime); this class owns only
 * the View handle, mirroring the Surface lifecycle: attach in
 * {@code surfaceCreated}, resize in {@code surfaceChanged}, detach in
 * {@code surfaceDestroyed}.
 *
 * <p>All sizes and coordinates passed to native are physical pixels —
 * the Device queries DPR internally.
 *
 * <p>The host Activity owns process-wide {@code setContext} /
 * {@code setCurrentActivity} / {@code pause} / {@code resume} /
 * {@code requestPermissionsResult} (see {@code PlaygroundActivity}). The
 * Runtime auto-subscribes to pause/resume on creation, so there is no
 * per-view pause/resume.
 */
public class BabylonView extends FrameLayout implements SurfaceHolder.Callback2, View.OnTouchListener {
    private static final FrameLayout.LayoutParams childViewLayoutParams =
            new FrameLayout.LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT);

    private final SurfaceView primarySurfaceView;
    private final SurfaceView secondarySurfaceView;
    private final SurfaceView xrSurfaceView;

    /** Runtime handle borrowed from the host. Not owned by this view. */
    private final long mRuntimeHandle;

    /** Native View handle (0 if not attached). Owned by this view. */
    private long mViewHandle = 0;

    public BabylonView(Context context, long runtimeHandle) {
        super(context);
        mRuntimeHandle = runtimeHandle;

        // Background Android UI image: the translucent secondary surface
        // composites over this, demonstrating GL alpha -> SurfaceView opacity.
        android.widget.ImageView bg = new android.widget.ImageView(context);
        bg.setBackgroundColor(0xFF1E7A1E);
        bg.setImageResource(android.R.drawable.ic_menu_compass);
        bg.setScaleType(android.widget.ImageView.ScaleType.CENTER_INSIDE);
        this.addView(bg, new FrameLayout.LayoutParams(
                LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT));

        // Two render surfaces side by side: primary drives Babylon Native; the
        // secondary is registered as a mirror swapchain (SPIKE).
        android.widget.LinearLayout split = new android.widget.LinearLayout(context);
        split.setOrientation(android.widget.LinearLayout.VERTICAL);
        split.setLayoutParams(childViewLayoutParams);

        this.primarySurfaceView = new SurfaceView(context);
        this.primarySurfaceView.getHolder().addCallback(this);
        split.addView(this.primarySurfaceView, new android.widget.LinearLayout.LayoutParams(
                LayoutParams.MATCH_PARENT, 0, 1f));

        this.secondarySurfaceView = new SurfaceView(context);
        // Transparent overlay: SurfaceView shows compositor alpha from the GL
        // alpha channel mirrored into it.
        this.secondarySurfaceView.setZOrderOnTop(true);
        this.secondarySurfaceView.getHolder().setFormat(android.graphics.PixelFormat.TRANSLUCENT);
        this.secondarySurfaceView.getHolder().addCallback(new SurfaceHolder.Callback() {
            @Override public void surfaceCreated(SurfaceHolder holder) {}
            @Override public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
                BabylonNative.runtimeAddSecondarySurface(mRuntimeHandle, holder.getSurface());
            }
            @Override public void surfaceDestroyed(SurfaceHolder holder) {
                BabylonNative.runtimeRemoveSecondarySurface(mRuntimeHandle, holder.getSurface());
            }
        });
        split.addView(this.secondarySurfaceView, new android.widget.LinearLayout.LayoutParams(
                LayoutParams.MATCH_PARENT, 0, 1f));

        this.addView(split);

        setOnTouchListener(this);

        this.xrSurfaceView = new SurfaceView(context);
        this.xrSurfaceView.setLayoutParams(childViewLayoutParams);
        this.xrSurfaceView.getHolder().addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder holder) {
                // surfaceChanged is also called when the surface is created, so just do all the handling there
            }

            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
                BabylonNative.runtimeSetXrSurface(mRuntimeHandle, holder.getSurface());
            }

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {
                BabylonNative.runtimeSetXrSurface(mRuntimeHandle, null);
            }
        });
        this.xrSurfaceView.setVisibility(View.INVISIBLE);
        this.addView(this.xrSurfaceView);

        setWillNotDraw(false);
    }

    /**
     * This method is part of the SurfaceHolder.Callback interface, and is
     * not normally called or subclassed by clients of BabylonView.
     */
    public void surfaceCreated(SurfaceHolder holder) {
        mViewHandle = BabylonNative.viewAttach(mRuntimeHandle, holder.getSurface());
    }

    /**
     * This method is part of the SurfaceHolder.Callback interface, and is
     * not normally called or subclassed by clients of BabylonView.
     */
    public void surfaceDestroyed(SurfaceHolder holder) {
        if (mViewHandle != 0) {
            BabylonNative.viewDetach(mViewHandle);
            mViewHandle = 0;
        }
    }

    /**
     * This method is part of the SurfaceHolder.Callback interface, and is
     * not normally called or subclassed by clients of BabylonView.
     */
    public void surfaceChanged(SurfaceHolder holder, int format, int w, int h) {
        if (mViewHandle != 0) {
            BabylonNative.viewResize(mViewHandle, w, h);
        }
    }

    @Override
    public boolean onTouch(View v, MotionEvent event) {
        if (mViewHandle == 0) {
            return false;
        }

        int pointerId = event.getPointerId(event.getActionIndex());
        float x = event.getX(event.getActionIndex());
        float y = event.getY(event.getActionIndex());

        switch (event.getActionMasked()) {
            case MotionEvent.ACTION_DOWN:
            case MotionEvent.ACTION_POINTER_DOWN:
                BabylonNative.viewPointerDown(mViewHandle, pointerId, x, y);
                break;
            case MotionEvent.ACTION_MOVE:
                BabylonNative.viewPointerMove(mViewHandle, pointerId, x, y);
                break;
            case MotionEvent.ACTION_UP:
            case MotionEvent.ACTION_POINTER_UP:
                BabylonNative.viewPointerUp(mViewHandle, pointerId, x, y);
                break;
        }
        return true;
    }

    /**
     * This method is part of the SurfaceHolder.Callback2 interface, and is
     * not normally called or subclassed by clients of BabylonView.
     */
    @Deprecated
    @Override
    public void surfaceRedrawNeeded(SurfaceHolder holder) {
        // Redraw happens in the bgfx thread. No need to handle it here.
    }

    @Override
    protected void onDraw(Canvas canvas) {
        if (BabylonNative.runtimeIsXrActive(mRuntimeHandle)) {
            this.xrSurfaceView.setVisibility(View.VISIBLE);
        } else {
            this.xrSurfaceView.setVisibility(View.INVISIBLE);
        }

        if (mViewHandle != 0) {
            BabylonNative.viewRenderFrame(mViewHandle);
            BabylonNative.runtimeMirrorFrame(mRuntimeHandle);
        }
        invalidate();
    }
}

package com.android.babylonnative.playground;

import android.Manifest;
import android.app.Activity;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.view.View;
import android.widget.Button;
import android.widget.RelativeLayout;

import BabylonNative.BabylonView;

public class PlaygroundActivity extends Activity implements BabylonView.ViewDelegate {
    BabylonView mView;
    private boolean isFullSize = false;
    private static final int HALF_SIZE_HEIGHT = 400;
    // Activity life
    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        // Create a RelativeLayout to hold the views
        RelativeLayout layout = new RelativeLayout(this);
        layout.setLayoutParams(new RelativeLayout.LayoutParams(
                RelativeLayout.LayoutParams.MATCH_PARENT,
                RelativeLayout.LayoutParams.MATCH_PARENT
        ));

        mView = new BabylonView(getApplication(), this);
        //setContentView(mView);
        RelativeLayout.LayoutParams viewParams = new RelativeLayout.LayoutParams(
                RelativeLayout.LayoutParams.MATCH_PARENT,
                HALF_SIZE_HEIGHT
        );
        mView.setLayoutParams(viewParams);
        mView.setBackgroundColor(0xFFFF0000); // Red color
        layout.addView(mView);

        // Create the button programmatically
        Button toggleButton = new Button(this);
        toggleButton.setText("Toggle Size");
        RelativeLayout.LayoutParams buttonParams = new RelativeLayout.LayoutParams(
                RelativeLayout.LayoutParams.WRAP_CONTENT,
                RelativeLayout.LayoutParams.WRAP_CONTENT
        );
        buttonParams.addRule(RelativeLayout.BELOW, mView.getId());
        buttonParams.addRule(RelativeLayout.CENTER_HORIZONTAL);
        buttonParams.topMargin = 20;
        toggleButton.setLayoutParams(buttonParams);
        layout.addView(toggleButton);


        //other button
        Button toggleButton2 = new Button(this);
        toggleButton2.setText("Toggle PP");
        RelativeLayout.LayoutParams buttonParams2 = new RelativeLayout.LayoutParams(
                RelativeLayout.LayoutParams.WRAP_CONTENT,
                RelativeLayout.LayoutParams.WRAP_CONTENT
        );
        buttonParams2.addRule(RelativeLayout.BELOW, mView.getId());
        buttonParams2.addRule(RelativeLayout.ALIGN_LEFT);
        buttonParams2.topMargin = 20;
        toggleButton2.setLayoutParams(buttonParams2);
        layout.addView(toggleButton2);

        // Set the layout as the content view
        setContentView(layout);

        toggleButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                toggleViewSize();
            }
        });

        toggleButton2.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mView.eval("togglePP();", "");
            }
        });
    }

    private void toggleViewSize() {
        RelativeLayout.LayoutParams params = (RelativeLayout.LayoutParams) mView.getLayoutParams();

        if (isFullSize) {
            // Set to half size
            params.height = HALF_SIZE_HEIGHT;
        } else {
            // Set to full size
            params.height = RelativeLayout.LayoutParams.MATCH_PARENT;
        }

        mView.setLayoutParams(params);
        isFullSize = !isFullSize;
    }
    @Override
    protected void onPause() {
        mView.onPause();
        super.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        mView.onResume();
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] results) {
        mView.onRequestPermissionsResult(requestCode, permissions, results);
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus && mView.getVisibility() == View.GONE) {
            mView.setVisibility(View.VISIBLE);
        }
    }

    @Override
    public void onViewReady() {
        mView.loadScript("app:///Scripts/experience.js");
    }
}

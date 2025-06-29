package com.enthusiasticcoder.tfltracker;

import android.content.Context;
import android.content.res.AssetManager;

import com.google.android.play.core.assetpacks.AssetPackManager;
import com.google.android.play.core.assetpacks.AssetPackManagerFactory;
import com.google.android.play.core.assetpacks.AssetLocation;
import com.google.android.play.core.assetpacks.AssetPackLocation;
import com.google.android.play.core.assetpacks.AssetPackStates;

import java.io.IOException;
import java.io.InputStream;

public class AssetPackHelper {
    private AssetPackManager assetPackManager;

    public AssetPackHelper(Context context) {
        assetPackManager = AssetPackManagerFactory.getInstance(context);
    }

    public AssetPackLocation getPackLocation(String packName) {
        return assetPackManager.getPackLocation(packName);
    }

    public AssetLocation getAssetLocation(String packName, String fileName) {
        return assetPackManager.getAssetLocation(packName, fileName);
    }
}

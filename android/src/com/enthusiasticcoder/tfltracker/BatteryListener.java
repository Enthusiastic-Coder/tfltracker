package com.enthusiasticcoder.tfltracker;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.IntentFilter;
import android.content.Intent;
import android.os.BatteryManager;
import android.content.Context;

public class BatteryListener
{
    private static Activity s_activity;

    public static void setActivity(Activity activity)
    {
        s_activity = activity;
    }

    public static native void OnBatteryInfoAvailable(int percent, boolean bCharging);

    public static void StartBatteryListener() {

        IntentFilter filter = new IntentFilter(Intent.ACTION_BATTERY_CHANGED);

        BroadcastReceiver receiver = new BroadcastReceiver()
        {
            @Override
            public void onReceive(Context context, Intent intent)
            {
                boolean bCharging = (intent.getIntExtra(BatteryManager.EXTRA_PLUGGED, -1) == 0) ? false : true;
                int Level = intent.getIntExtra(BatteryManager.EXTRA_LEVEL, -1);
                int Scale = intent.getIntExtra(BatteryManager.EXTRA_SCALE, -1);

                if(Level >= 0 && Scale > 0)
                    OnBatteryInfoAvailable((Level * 100) / Scale, bCharging);
            }
        };

        s_activity.registerReceiver(receiver, filter);
    }
}

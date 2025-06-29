package com.enthusiasticcoder.tfltracker;

import android.os.Bundle;
import android.view.WindowManager;
import android.widget.Toast;
import android.content.Intent;
import android.net.Uri;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.content.Context;
import android.util.Log;
import android.app.Activity;

import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.view.Surface;
import android.content.Context;

import androidx.annotation.Nullable;
import androidx.annotation.NonNull;

import com.android.billingclient.api.AcknowledgePurchaseParams;
import com.android.billingclient.api.AcknowledgePurchaseResponseListener;
import com.android.billingclient.api.BillingClient;
import com.android.billingclient.api.BillingClientStateListener;
import com.android.billingclient.api.BillingFlowParams;
import com.android.billingclient.api.BillingResult;
import com.android.billingclient.api.Purchase;
import com.android.billingclient.api.PurchasesUpdatedListener;
import com.android.billingclient.api.PurchasesResponseListener;

import com.android.billingclient.api.SkuDetails;
import com.android.billingclient.api.SkuDetailsParams;
import com.android.billingclient.api.SkuDetailsResponseListener;

import static com.android.billingclient.api.BillingClient.SkuType.INAPP;
import static com.android.billingclient.api.BillingClient.SkuType.SUBS;
import static com.android.billingclient.api.BillingClient.SkuType;
import java.util.ArrayList;
import java.util.List;

import java.io.IOException;

public class MyActivity extends org.qtproject.qt.android.bindings.QtActivity
        implements PurchasesUpdatedListener, SensorEventListener{

    private static final String LOG_TAG = "tfltracker_iabv3";
    private static final String LICENSE_KEY = "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAlrRcx7ZowaE93GuMLDMXSeLmzKgr1VDm/SSke6unvYfLilym8t/gho5JvaUZ/m+7TrFAsp95VjLdYIwLHuYH5NZTCYd67m7fcZwJhAoBDIAptaJHaVI3KddunFDoq28EYvm8TlR3rhmryeeAh/HuT3p+mc6qNS+2dKo0/VQ61NL4yOZlitUBRP1Ce9SauqqwB28LseWS2MZ04BnfNDMgXm6N+xixrxaTgAR2b+N6XH0Q6XQBwcdG5SYtxIb4xuiwyhUSl1kdMPmyYLbz4fGA8JOTewrDD9YiKycd70/lR+yYkXLNYSHdgZ5o+qsOPeJnW552Phmy0AHSChe3VEIBAwIDAQAB";

    private BillingClient billingClient;

    private float[] gravityValues = new float[3];
    private float[] geoMagneticValues = new float[3];
    private float[] rotationMatrix = new float[16];
    private float[] orientation = new float[3];

    private SensorManager mSensorManager;
    private Sensor magFieldSensor;
    private Sensor accelSensor;
    private Sensor rotVectorSensor;

    private boolean mWantRotationVector;

    public MyActivity() {
        BatteryListener.setActivity(this);
    }

    @Override
    public void onCreate(android.os.Bundle savedInstanceState){

          super.onCreate(savedInstanceState);
          this.getWindow().addFlags(android.view.WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
          mSensorManager = (SensorManager)getApplication().getSystemService(Context.SENSOR_SERVICE);
          magFieldSensor = mSensorManager.getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD);
          accelSensor = mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
          rotVectorSensor = mSensorManager.getDefaultSensor(Sensor.TYPE_ROTATION_VECTOR);
    }

    @Override
    public void onDestroy() {
        if(billingClient!=null) {
             billingClient.endConnection();
        }

        super.onDestroy();
    }

    public void startIntent(int port, int freq, int samplerate) {

        Intent intent = new Intent(Intent.ACTION_VIEW).setData(
            Uri.parse("iqsrc://-a 0.0.0.0 -p "+ port + " -s "+ samplerate +" -f " + freq + " -g 0"));

        startActivityForResult(intent, 1234);
    }

    private void showToast(final String message, final int length) {

        runOnUiThread(new Runnable(){
                   public void run(){
                         Toast.makeText(getApplicationContext(), message, length).show();
                         }
                     });
   }

    private void createBillingProcessor() {
        // Establish connection to billing client
        //check purchase status from google play store cache
        //to check if item already Purchased previously or refunded
        billingClient = BillingClient.newBuilder(this)
                                .enablePendingPurchases()
                                .setListener(this)
                                .build();

        billingClient.startConnection(new BillingClientStateListener() {
            @Override
            public void onBillingSetupFinished(BillingResult billingResult) {
                if(billingResult.getResponseCode()==BillingClient.BillingResponseCode.OK){
                    onNativeBillingInitialized();
                }
            }

            @Override
            public void onBillingServiceDisconnected() {
            }
        });
    }

    public void InitializeBilling() {

        runOnUiThread(new Runnable(){
                   public void run(){
                       createBillingProcessor();
                }
            }
        );
    }

    private void RegisterProductHelper(String id, boolean inApp) {

        final String ID = id;
        final boolean bINAPP = inApp;
        List<String> skuList = new ArrayList<> ();
        skuList.add(id);
        SkuDetailsParams.Builder params = SkuDetailsParams.newBuilder();
        params.setSkusList(skuList).setType(inApp?INAPP:SUBS);
        billingClient.querySkuDetailsAsync(params.build(),
            new SkuDetailsResponseListener() {
                @Override
                public void onSkuDetailsResponse(BillingResult billingResult, List<SkuDetails> skuDetailsList) {

                    if (billingResult.getResponseCode() == BillingClient.BillingResponseCode.OK) {

                        SkuDetails skuDetails = null;
                        for (SkuDetails details : skuDetailsList) {

                            if( details.getSku().equals(ID)) {
                                skuDetails = details;
                                break;
                            }
                        }

                        if( skuDetails == null) {
                            onNativeProductUnknown(ID);
                            return;
                        }

                        final String productId = skuDetails.getSku();
                        final String title = skuDetails.getTitle();
                        final String price = skuDetails.getPrice();
                        final String description = skuDetails.getDescription();

                        billingClient.queryPurchasesAsync(bINAPP?INAPP:SUBS,
                            new PurchasesResponseListener() {

                               @Override
                               public void onQueryPurchasesResponse(@NonNull BillingResult billingResult, @NonNull List<Purchase> myPurchases) {

                                    for (Purchase purchase: myPurchases) {

                                        if (purchase.getSkus().contains(ID)) {

                                            onNativeProductKnown(productId,
                                                                 purchase.getPurchaseState() == Purchase.PurchaseState.PURCHASED,
                                                                 title,
                                                                 description,
                                                                 price );
                                            return;
                                        }
                                    }

                                    onNativeProductKnown(productId,
                                                     false,
                                                     title,
                                                     description,
                                                     price );


                               }
                            }
                        );
                     }
                }
            });
    }

    private void RegisterSubOrInApp(final String id, final boolean inApp) {
        runOnUiThread(new Runnable(){
           public void run(){
                        if (billingClient.isReady()) {
                            RegisterProductHelper(id, inApp);
                        } else {

                                billingClient = BillingClient.newBuilder(MyActivity.this).enablePendingPurchases().setListener(MyActivity.this).build();
                                billingClient.startConnection(new BillingClientStateListener() {
                                @Override
                                public void onBillingSetupFinished(BillingResult billingResult) {
                                    if (billingResult.getResponseCode() == BillingClient.BillingResponseCode.OK) {
                                        RegisterProductHelper(id, inApp);
                                    } else {
                                        showToast( "Error "+billingResult.getDebugMessage(),Toast.LENGTH_SHORT);
                                    }
                                }
                                @Override
                                public void onBillingServiceDisconnected() {
                                }
                            });
                        }
                    }
            });
    }

    public void RegisterSubscription(String id) {
        RegisterSubOrInApp(id, false);
    }

    public void RegisterProduct(String id) {
        RegisterSubOrInApp(id, true);
    }

    private void purchase(String id, boolean inApp) {
        final String ID = id;
        final boolean INAPP = inApp;
        //check if service is already connected
        if (billingClient.isReady()) {
            initiatePurchase(id, inApp);
        }
        //else reconnect service
        else{
            billingClient = BillingClient.newBuilder(this)
                                    .enablePendingPurchases()
                                    .setListener(this)
                                    .build();

            billingClient.startConnection(new BillingClientStateListener() {
                @Override
                public void onBillingSetupFinished(BillingResult billingResult) {
                    if (billingResult.getResponseCode() == BillingClient.BillingResponseCode.OK) {
                        initiatePurchase(ID, INAPP);
                    } else {
                        showToast( "Error "+billingResult.getDebugMessage(),Toast.LENGTH_SHORT);
                    }
                }
                @Override
                public void onBillingServiceDisconnected() {
                }
            });
        }
    }

    private void initiatePurchase(String PRODUCT_ID, boolean inApp) {
        List<String> skuList = new ArrayList<>();
        skuList.add(PRODUCT_ID);
        SkuDetailsParams.Builder params = SkuDetailsParams.newBuilder();
        params.setSkusList(skuList).setType(inApp?INAPP:SUBS);
        billingClient.querySkuDetailsAsync(params.build(),
                new SkuDetailsResponseListener() {
                    @Override
                    public void onSkuDetailsResponse(BillingResult billingResult, List<SkuDetails> skuDetailsList) {

                        if (billingResult.getResponseCode() == BillingClient.BillingResponseCode.OK) {

                            if (skuDetailsList != null && skuDetailsList.size() > 0) {

                                BillingFlowParams flowParams = BillingFlowParams.newBuilder()
                                        .setSkuDetails(skuDetailsList.get(0))
                                        .build();

                                billingClient.launchBillingFlow(MyActivity.this, flowParams);
                            }
                            else{
                                //try to add item/product id "purchase" inside managed product in google play console
                                showToast("Purchase Item not Found",Toast.LENGTH_SHORT);
                            }

                        } else {
                            showToast(" Error "+billingResult.getDebugMessage(), Toast.LENGTH_SHORT);
                        }
                    }
                });
    }


    public void MakePurchase(String id) {

        final String ID = id;
        runOnUiThread(new Runnable(){
           public void run(){
                    purchase(ID, true);
                  }
              });
    }

    public void MakeSubscription(String id) {

        final String ID = id;
        runOnUiThread(new Runnable(){
           public void run(){
                    purchase(ID, false);
                  }
              });
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {

        if (requestCode == 1234) {

            if (resultCode == RESULT_OK) {
               // Connection with device has been opened and the rtl-tcp server is running. You are now responsible for connecting.
               showToast("Driver started OK", Toast.LENGTH_LONG );
            } else {
                if( data == null ) {
                    showToast("Wrong driver selected, see menu - dump1090 for help.", Toast.LENGTH_LONG );
                    } else {
                   // something went wrong, and the driver failed to start
                   String errmsg = data.getStringExtra("detailed_exception_message");
                   //showErrorMessage(errmsg); // Show the user why something went wrong
                   showToast("Result :" + errmsg, Toast.LENGTH_LONG );
                }
            }

        } else {
            super.onActivityResult(requestCode, resultCode, data);
        }
    }

    @Override
   public void onPurchasesUpdated(BillingResult billingResult, @Nullable List<Purchase> purchases) {
       //if item newly purchased
       if (billingResult.getResponseCode() == BillingClient.BillingResponseCode.OK && purchases != null) {
           handlePurchases(purchases);
       }
       //if item already purchased then check and reflect changes
       else if (billingResult.getResponseCode() == BillingClient.BillingResponseCode.ITEM_ALREADY_OWNED) {
               billingClient.queryPurchasesAsync(INAPP,new PurchasesResponseListener() {

                   @Override
                   public void onQueryPurchasesResponse(@NonNull BillingResult billingResult, @NonNull List<Purchase> myPurchases) {
                       handlePurchases(myPurchases);
                   }
                });
       }
       //if purchase cancelled
       else if (billingResult.getResponseCode() == BillingClient.BillingResponseCode.USER_CANCELED) {
           showToast("Purchase Canceled",Toast.LENGTH_SHORT);
       }
       // Handle any other error msgs
       else {
           showToast("Error "+billingResult.getDebugMessage(),Toast.LENGTH_SHORT);
       }
   }

    void handlePurchases(List<Purchase> purchases) {

       for(Purchase purchase:purchases) {

           //if item is purchased
           if (purchase.getPurchaseState() == Purchase.PurchaseState.PURCHASED)
           {
               if (!verifyValidSignature(purchase.getOriginalJson(), purchase.getSignature())) {
                   // Invalid purchase
                   // show error to user
                   showToast( "Error : Invalid Purchase", Toast.LENGTH_SHORT);
                   return;
               }
               // else purchase is valid
               //if item is purchased and not acknowledged
               if (!purchase.isAcknowledged()) {

                   AcknowledgePurchaseParams acknowledgePurchaseParams
                                       = AcknowledgePurchaseParams.newBuilder()
                                       .setPurchaseToken(purchase.getPurchaseToken())
                                       .build();

                    final String productId = purchase.getSkus().get(0);

                   billingClient.acknowledgePurchase(acknowledgePurchaseParams, new AcknowledgePurchaseResponseListener() {
                       @Override
                       public void onAcknowledgePurchaseResponse(BillingResult billingResult) {
                           if(billingResult.getResponseCode()== BillingClient.BillingResponseCode.OK){

                               showToast( "Item Purchased/Acknowledged", Toast.LENGTH_SHORT);
                               onNativeProductPurchased(productId);
                           }
                       }
                   });
               }
               //else item is purchased and also acknowledged
               else {
                    showToast("Item Purchased", Toast.LENGTH_SHORT);
                    onNativeProductPurchased(purchase.getSkus().get(0));
               }
           }
           //if purchase is pending
           else if(purchase.getPurchaseState() == Purchase.PurchaseState.PENDING)
           {
               showToast("Purchase is Pending. Please complete Transaction", Toast.LENGTH_SHORT);
           }
           //if purchase is unknown
           else if(purchase.getPurchaseState() == Purchase.PurchaseState.UNSPECIFIED_STATE)
           {
               showToast("Purchase Status Not Purchased", Toast.LENGTH_SHORT);
           }
       }
   }

    public static native void onNativeProductKnown(String productId, boolean purchased, String title, String desc, String cost );
    public static native void onNativeProductUnknown(String productId);
    public static native void onNativeProductPurchased(String productId);
    public static native void onNativeBillingInitialized();
    public static native void onNativeRotationVector(float x, float y, float z);
    public static native void onNativeVRStarted(boolean started);

    /**
     * Verifies that the purchase was signed correctly for this developer's public key.
     * <p>Note: It's strongly recommended to perform such check on your backend since hackers can
     * replace this method with "constant true" if they decompile/rebuild your app.
     * </p>
     */
    private boolean verifyValidSignature(String signedData, String signature) {

        // To get key go to Developer Console > Select your app > Development Tools > Services & APIs.
        String base64Key = LICENSE_KEY;
        return Security.verifyPurchase(base64Key, signedData, signature);
    }

     @Override
     protected void onResume() {
             // TODO Auto-generated method stub
             super.onResume();

             if(mWantRotationVector)
                enableVR(true);
     }

     @Override
     protected void onPause() {
             // TODO Auto-generated method stub
             super.onPause();

             enableVR(false);
     }

     private void enableSensor(Sensor sensor, boolean enabled) {

         if (enabled)
             mSensorManager.registerListener(this, sensor, SensorManager.SENSOR_DELAY_FASTEST, null);
         else
             mSensorManager.unregisterListener(this,sensor);
     }

    public boolean IsVRActive() {
        return mWantRotationVector;
    }

    private void enableVR(boolean enable) {

        if( rotVectorSensor != null) {
            enableSensor(rotVectorSensor, enable);
        } else {
            enableSensor(magFieldSensor, enable);
            enableSensor(accelSensor, enable);
        }
    }

    public void TriggerRotationVector() {
        mWantRotationVector = !mWantRotationVector;
        runOnUiThread(new Runnable(){
           public void run(){

               if( magFieldSensor == null && rotVectorSensor == null) {

                   showToast("No sensors found so VR is not supported!", Toast.LENGTH_LONG);
                   mWantRotationVector = false;
                   return;
               }

               enableVR(mWantRotationVector);

               if(mWantRotationVector && rotVectorSensor == null)
                    showToast("No gyroscope found so VR will run in degraded mode!", Toast.LENGTH_LONG);

               onNativeVRStarted(mWantRotationVector);
            }
        });
    }


    @Override
     public void onAccuracyChanged(Sensor sensor, int accuracy) {
             // TODO Auto-generated method stub

             if( accuracy <=1 ) {
                     showToast("Please shake the device in a figure eight pattern to improve sensor accuracy!", Toast.LENGTH_LONG);
             }
     }

     public class AxisPair {

         public AxisPair(int x, int y) {
             this.axisX = x;
             this.axisY = y;
             }

         public int getX() {
             return this.axisX;
         }

         public int getY() {
             return this.axisY;
         }

         int axisX = 0;
         int axisY = 0;
     }

    AxisPair getCurrentAxisOrientation() {
        int axisX = 0;
        int axisY = 0;

        switch(getWindowManager().getDefaultDisplay().getRotation())
        {
        case Surface.ROTATION_0:
                break;
        case Surface.ROTATION_90:
                axisX = SensorManager.AXIS_Y;
                axisY = SensorManager.AXIS_MINUS_X;
                break;
        case Surface.ROTATION_180:
                axisX = SensorManager.AXIS_MINUS_X;
                axisY = SensorManager.AXIS_MINUS_Y;
                break;
        case Surface.ROTATION_270:
                axisX = SensorManager.AXIS_MINUS_Y;
                axisY = SensorManager.AXIS_X;
                break;
        default:
                break;
        }

        return new AxisPair(axisX, axisY);
        }

     @Override
     public void onSensorChanged(SensorEvent event) {

         int sensorEventType = event.sensor.getType();
         boolean ready = false;

         if( sensorEventType == Sensor.TYPE_ACCELEROMETER) {

             for(int i=0; i<3; i++){
                 gravityValues[i] = event.values[i];
             }

            if( geoMagneticValues[0] != 0)
                ready = true;
         }
         else if( sensorEventType == Sensor.TYPE_MAGNETIC_FIELD) {

             for(int i=0; i<3; i++){
                 geoMagneticValues[i] = event.values[i];
             }

             if( gravityValues[2] != 0)
                ready = true;
         }
         else if(sensorEventType == Sensor.TYPE_ROTATION_VECTOR) {

             SensorManager.getRotationMatrixFromVector(rotationMatrix, event.values);

             AxisPair axisPair = getCurrentAxisOrientation();

             int axisX = axisPair.getX();
             int axisY = axisPair.getY();

             if( axisX != 0 && axisY != 0)
                SensorManager.remapCoordinateSystem(rotationMatrix, axisX, axisY, rotationMatrix);

             SensorManager.remapCoordinateSystem(rotationMatrix, SensorManager.AXIS_X, SensorManager.AXIS_Z,  rotationMatrix);
             SensorManager.getOrientation(rotationMatrix, orientation);

             onNativeRotationVector((float) Math.toDegrees(orientation[0]),
                                    (float) Math.toDegrees(orientation[1]),
                                    (float) Math.toDegrees(orientation[2]));

             return;
        }
        else
            return;

        if( !ready)
            return;

        if( SensorManager.getRotationMatrix(rotationMatrix, null, gravityValues, geoMagneticValues)) {

            AxisPair axisPair = getCurrentAxisOrientation();

            int axisX = axisPair.getX();
            int axisY = axisPair.getY();

            if( axisX != 0 && axisY != 0)
                SensorManager.remapCoordinateSystem(rotationMatrix, axisX, axisY,  rotationMatrix);

            SensorManager.remapCoordinateSystem(rotationMatrix, SensorManager.AXIS_X, SensorManager.AXIS_Z,  rotationMatrix);
            SensorManager.getOrientation(rotationMatrix, orientation);

            onNativeRotationVector((float) Math.toDegrees(orientation[0]),
                                   (float) Math.toDegrees(orientation[1]),
                                   (float) Math.toDegrees(orientation[2]));
        }
    }
}

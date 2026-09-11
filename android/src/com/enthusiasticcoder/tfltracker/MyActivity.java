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
import com.android.billingclient.api.PendingPurchasesParams;
import com.android.billingclient.api.ProductDetails;
import com.android.billingclient.api.ProductDetailsResponseListener;
import com.android.billingclient.api.Purchase;
import com.android.billingclient.api.PurchasesResponseListener;
import com.android.billingclient.api.PurchasesUpdatedListener;
import com.android.billingclient.api.QueryProductDetailsParams;
import com.android.billingclient.api.QueryProductDetailsResult;
import com.android.billingclient.api.QueryPurchasesParams;

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

        PendingPurchasesParams pendingPurchasesParams =
                PendingPurchasesParams.newBuilder()
                        .enableOneTimeProducts()
                        .build();

        billingClient = BillingClient.newBuilder(this)
                .setListener(this)
                .enablePendingPurchases(pendingPurchasesParams)
                .build();

        billingClient.startConnection(new BillingClientStateListener() {

            @Override
            public void onBillingSetupFinished(BillingResult billingResult) {

                if (billingResult.getResponseCode()
                        == BillingClient.BillingResponseCode.OK) {

                    onNativeBillingInitialized();

                } else {

                    showToast(
                            "Billing setup error: "
                                    + billingResult.getDebugMessage(),
                            Toast.LENGTH_SHORT);
                }
            }

            @Override
            public void onBillingServiceDisconnected() {

                // We reconnect when another billing request is made.
            }
        });
    }


    public void InitializeBilling() {

        runOnUiThread(new Runnable() {

            @Override
            public void run() {
                createBillingProcessor();
            }
        });
    }


    private String getProductType(boolean inApp) {

        return inApp
                ? BillingClient.ProductType.INAPP
                : BillingClient.ProductType.SUBS;
    }


    private QueryProductDetailsParams buildProductQuery(
            String productId,
            boolean inApp) {

        QueryProductDetailsParams.Product product =
                QueryProductDetailsParams.Product.newBuilder()
                        .setProductId(productId)
                        .setProductType(getProductType(inApp))
                        .build();

        List<QueryProductDetailsParams.Product> products =
                new ArrayList<>();

        products.add(product);

        return QueryProductDetailsParams.newBuilder()
                .setProductList(products)
                .build();
    }


    private QueryPurchasesParams buildPurchasesQuery(boolean inApp) {

        return QueryPurchasesParams.newBuilder()
                .setProductType(getProductType(inApp))
                .build();
    }


    private ProductDetails findProductDetails(
            String wantedProductId,
            QueryProductDetailsResult result) {

        if (result == null)
            return null;

        List<ProductDetails> products =
                result.getProductDetailsList();

        if (products == null)
            return null;

        for (ProductDetails productDetails : products) {

            if (wantedProductId.equals(productDetails.getProductId()))
                return productDetails;
        }

        return null;
    }


    private boolean purchaseContainsProduct(
            Purchase purchase,
            String productId) {

        if (purchase == null)
            return false;

        List<String> products = purchase.getProducts();

        return products != null && products.contains(productId);
    }


    private String getProductPrice(
            ProductDetails productDetails,
            boolean inApp) {

        if (productDetails == null)
            return "";

        if (inApp) {

            /*
             * Existing/simple one-time products generally expose this
             * non-list form.
             */
            ProductDetails.OneTimePurchaseOfferDetails offer =
                    productDetails.getOneTimePurchaseOfferDetails();

            if (offer != null)
                return offer.getFormattedPrice();

            /*
             * Billing 9 can also expose multiple purchase offers.
             */
            List<ProductDetails.OneTimePurchaseOfferDetails> offers =
                    productDetails.getOneTimePurchaseOfferDetailsList();

            if (offers != null && !offers.isEmpty())
                return offers.get(0).getFormattedPrice();

            return "";
        }


        List<ProductDetails.SubscriptionOfferDetails> offers =
                productDetails.getSubscriptionOfferDetails();

        if (offers == null || offers.isEmpty())
            return "";

        ProductDetails.SubscriptionOfferDetails offer =
                offers.get(0);

        List<ProductDetails.PricingPhase> phases =
                offer.getPricingPhases().getPricingPhaseList();

        if (phases == null || phases.isEmpty())
            return "";

        /*
         * Use the final pricing phase.
         *
         * This is normally the ongoing subscription price rather
         * than a free trial / introductory phase.
         */
        return phases.get(phases.size() - 1).getFormattedPrice();
    }


    private void RegisterProductHelper(
            final String id,
            final boolean inApp) {

        QueryProductDetailsParams params =
                buildProductQuery(id, inApp);

        billingClient.queryProductDetailsAsync(
                params,
                new ProductDetailsResponseListener() {

                    @Override
                    public void onProductDetailsResponse(
                            @NonNull BillingResult billingResult,
                            @NonNull QueryProductDetailsResult result) {

                        if (billingResult.getResponseCode()
                                != BillingClient.BillingResponseCode.OK) {

                            showToast(
                                    "Product query error: "
                                            + billingResult.getDebugMessage(),
                                    Toast.LENGTH_SHORT);

                            onNativeProductUnknown(id);
                            return;
                        }


                        ProductDetails productDetails =
                                findProductDetails(id, result);

                        if (productDetails == null) {

                            onNativeProductUnknown(id);
                            return;
                        }


                        final String productId =
                                productDetails.getProductId();

                        final String title =
                                productDetails.getTitle();

                        final String description =
                                productDetails.getDescription();

                        final String price =
                                getProductPrice(productDetails, inApp);


                        QueryPurchasesParams purchaseParams =
                                buildPurchasesQuery(inApp);


                        billingClient.queryPurchasesAsync(
                                purchaseParams,
                                new PurchasesResponseListener() {

                                    @Override
                                    public void onQueryPurchasesResponse(
                                            @NonNull BillingResult billingResult,
                                            @NonNull List<Purchase> purchases) {

                                        if (billingResult.getResponseCode()
                                                != BillingClient.BillingResponseCode.OK) {

                                            onNativeProductKnown(
                                                    productId,
                                                    false,
                                                    title,
                                                    description,
                                                    price);

                                            return;
                                        }


                                        for (Purchase purchase : purchases) {

                                            if (purchaseContainsProduct(
                                                    purchase,
                                                    productId)) {

                                                boolean purchased =
                                                        purchase.getPurchaseState()
                                                                == Purchase.PurchaseState.PURCHASED;

                                                onNativeProductKnown(
                                                        productId,
                                                        purchased,
                                                        title,
                                                        description,
                                                        price);

                                                return;
                                            }
                                        }


                                        onNativeProductKnown(
                                                productId,
                                                false,
                                                title,
                                                description,
                                                price);
                                    }
                                });
                    }
                });
    }


    private void RegisterSubOrInApp(
            final String id,
            final boolean inApp) {

        runOnUiThread(new Runnable() {

            @Override
            public void run() {

                if (billingClient == null) {

                    showToast(
                            "Billing has not been initialized",
                            Toast.LENGTH_SHORT);

                    return;
                }


                if (billingClient.isReady()) {

                    RegisterProductHelper(id, inApp);

                } else {

                    billingClient.startConnection(
                            new BillingClientStateListener() {

                                @Override
                                public void onBillingSetupFinished(
                                        BillingResult billingResult) {

                                    if (billingResult.getResponseCode()
                                            == BillingClient.BillingResponseCode.OK) {

                                        RegisterProductHelper(id, inApp);

                                    } else {

                                        showToast(
                                                "Billing connection error: "
                                                        + billingResult.getDebugMessage(),
                                                Toast.LENGTH_SHORT);
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


    private void purchase(
            final String id,
            final boolean inApp) {

        if (billingClient == null) {

            showToast(
                    "Billing has not been initialized",
                    Toast.LENGTH_SHORT);

            return;
        }


        if (billingClient.isReady()) {

            initiatePurchase(id, inApp);

        } else {

            billingClient.startConnection(
                    new BillingClientStateListener() {

                        @Override
                        public void onBillingSetupFinished(
                                BillingResult billingResult) {

                            if (billingResult.getResponseCode()
                                    == BillingClient.BillingResponseCode.OK) {

                                initiatePurchase(id, inApp);

                            } else {

                                showToast(
                                        "Billing connection error: "
                                                + billingResult.getDebugMessage(),
                                        Toast.LENGTH_SHORT);
                            }
                        }


                        @Override
                        public void onBillingServiceDisconnected() {
                        }
                    });
        }
    }


    private String getOfferToken(
            ProductDetails productDetails,
            boolean inApp) {

        if (inApp) {

            /*
             * First try the traditional/default one-time purchase offer.
             */
            ProductDetails.OneTimePurchaseOfferDetails offer =
                    productDetails.getOneTimePurchaseOfferDetails();

            if (offer != null)
                return offer.getOfferToken();


            /*
             * Billing 9 also supports multiple one-time purchase offers.
             * For now ADSB Flight Tracker selects the first eligible one.
             */
            List<ProductDetails.OneTimePurchaseOfferDetails> offers =
                    productDetails.getOneTimePurchaseOfferDetailsList();

            if (offers != null && !offers.isEmpty())
                return offers.get(0).getOfferToken();


            return null;
        }


        List<ProductDetails.SubscriptionOfferDetails> offers =
                productDetails.getSubscriptionOfferDetails();

        if (offers == null || offers.isEmpty())
            return null;


        /*
         * For your existing single subscription setup this selects
         * the first eligible base plan / offer returned by Google Play.
         */
        return offers.get(0).getOfferToken();
    }


    private void initiatePurchase(
            final String productId,
            final boolean inApp) {

        QueryProductDetailsParams params =
                buildProductQuery(productId, inApp);


        billingClient.queryProductDetailsAsync(
                params,
                new ProductDetailsResponseListener() {

                    @Override
                    public void onProductDetailsResponse(
                            @NonNull BillingResult billingResult,
                            @NonNull QueryProductDetailsResult result) {

                        if (billingResult.getResponseCode()
                                != BillingClient.BillingResponseCode.OK) {

                            showToast(
                                    "Product query error: "
                                            + billingResult.getDebugMessage(),
                                    Toast.LENGTH_SHORT);

                            return;
                        }


                        ProductDetails productDetails =
                                findProductDetails(productId, result);


                        if (productDetails == null) {

                            showToast(
                                    "Purchase item not found",
                                    Toast.LENGTH_SHORT);

                            return;
                        }


                        BillingFlowParams.ProductDetailsParams.Builder
                                productParamsBuilder =
                                BillingFlowParams.ProductDetailsParams
                                        .newBuilder()
                                        .setProductDetails(productDetails);


                        String offerToken =
                                getOfferToken(productDetails, inApp);


                        if (offerToken != null
                                && !offerToken.isEmpty()) {

                            productParamsBuilder.setOfferToken(offerToken);
                        }


                        List<BillingFlowParams.ProductDetailsParams>
                                productParamsList =
                                new ArrayList<>();


                        productParamsList.add(
                                productParamsBuilder.build());


                        BillingFlowParams flowParams =
                                BillingFlowParams.newBuilder()
                                        .setProductDetailsParamsList(
                                                productParamsList)
                                        .build();


                        BillingResult launchResult =
                                billingClient.launchBillingFlow(
                                        MyActivity.this,
                                        flowParams);


                        if (launchResult.getResponseCode()
                                != BillingClient.BillingResponseCode.OK) {

                            showToast(
                                    "Unable to start purchase: "
                                            + launchResult.getDebugMessage(),
                                    Toast.LENGTH_SHORT);
                        }
                    }
                });
    }


    public void MakePurchase(final String id) {

        runOnUiThread(new Runnable() {

            @Override
            public void run() {

                purchase(id, true);
            }
        });
    }


    public void MakeSubscription(final String id) {

        runOnUiThread(new Runnable() {

            @Override
            public void run() {

                purchase(id, false);
            }
        });
    }


    @Override
    public void onPurchasesUpdated(
            BillingResult billingResult,
            @Nullable List<Purchase> purchases) {

        int responseCode =
                billingResult.getResponseCode();


        if (responseCode
                == BillingClient.BillingResponseCode.OK
                && purchases != null) {

            handlePurchases(purchases);
            return;
        }


        if (responseCode
                == BillingClient.BillingResponseCode.ITEM_ALREADY_OWNED) {

            /*
             * ITEM_ALREADY_OWNED applies to one-time products.
             * Query currently owned INAPP purchases.
             */

            QueryPurchasesParams params =
                    QueryPurchasesParams.newBuilder()
                            .setProductType(
                                    BillingClient.ProductType.INAPP)
                            .build();


            billingClient.queryPurchasesAsync(
                    params,
                    new PurchasesResponseListener() {

                        @Override
                        public void onQueryPurchasesResponse(
                                @NonNull BillingResult billingResult,
                                @NonNull List<Purchase> purchases) {

                            if (billingResult.getResponseCode()
                                    == BillingClient.BillingResponseCode.OK) {

                                handlePurchases(purchases);
                            }
                        }
                    });

            return;
        }


        if (responseCode
                == BillingClient.BillingResponseCode.USER_CANCELED) {

            showToast(
                    "Purchase Canceled",
                    Toast.LENGTH_SHORT);

            return;
        }


        showToast(
                "Billing error: "
                        + billingResult.getDebugMessage(),
                Toast.LENGTH_SHORT);
    }


    void handlePurchases(List<Purchase> purchases) {

    if (purchases == null)
        return;


    for (Purchase purchase : purchases) {

        if (purchase.getPurchaseState()
                == Purchase.PurchaseState.PURCHASED) {

            if (!verifyValidSignature(
                    purchase.getOriginalJson(),
                    purchase.getSignature())) {

                showToast(
                        "Error : Invalid Purchase",
                        Toast.LENGTH_SHORT);

                continue;
            }


            List<String> products =
                    purchase.getProducts();


            if (products == null || products.isEmpty())
                continue;


            final String productId =
                    products.get(0);


            if (!purchase.isAcknowledged()) {

                AcknowledgePurchaseParams acknowledgeParams =
                        AcknowledgePurchaseParams.newBuilder()
                                .setPurchaseToken(
                                        purchase.getPurchaseToken())
                                .build();


                billingClient.acknowledgePurchase(
                        acknowledgeParams,
                        new AcknowledgePurchaseResponseListener() {

                            @Override
                            public void onAcknowledgePurchaseResponse(
                                    BillingResult billingResult) {

                                if (billingResult.getResponseCode()
                                        == BillingClient.BillingResponseCode.OK) {

                                    showToast(
                                            "Item Purchased/Acknowledged",
                                            Toast.LENGTH_SHORT);

                                    onNativeProductPurchased(
                                            productId);

                                } else {

                                    showToast(
                                            "Acknowledgement error: "
                                                    + billingResult.getDebugMessage(),
                                            Toast.LENGTH_SHORT);
                                }
                            }
                        });

            } else {

                showToast(
                        "Item Purchased",
                        Toast.LENGTH_SHORT);

                onNativeProductPurchased(
                        productId);
            }
        }


        else if (purchase.getPurchaseState()
                == Purchase.PurchaseState.PENDING) {

            showToast(
                    "Purchase is Pending. Please complete Transaction",
                    Toast.LENGTH_SHORT);
        }


        else if (purchase.getPurchaseState()
                == Purchase.PurchaseState.UNSPECIFIED_STATE) {

            showToast(
                    "Purchase Status Not Purchased",
                    Toast.LENGTH_SHORT);
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

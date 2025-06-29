#include "InAppStore.h"
#include <QTimer>
#include <QSettings>
#include <QDebug>

#ifdef Q_OS_WIN
const QString InAppStore::AndroidID_Windows = "-windows--desktop-";
#endif

const QString InAppStore::AppStoreID_3DView = "view3d";
const QString InAppStore::AppStoreID_ProximityAlert = "proximityalert";
const QString InAppStore::AppStoreID_RealTimeGPS = "realtimegps";
const QString InAppStore::AppStoreID_EstimatedPosition = "estimatedposition";
const QString InAppStore::AppStoreID_Monthly_Subcriber = "app_support_enhances";
const QString InAppStore::AppStoreID_LifeTime_Purchase = "lifetime_purchase";


namespace {
    const QString me_samsung4   = "3d31435423529a6d";
    const QString me_samsung8   = "283a2d5d1df2574b";
    const QString android_tv    = "48e4f37546874436";
    const QString anas           = "45363adaf99c5d95";
    const QString anas_note_8   = "75bcd6f7ec6abfd4";
    const QString rash           = "8a501dd0bd7997b8";

    QString settingForProduct(QString id)
    {
        return "InAppStore/" + id + "/evalPeriodLeft";
    }
}

#ifdef Q_OS_ANDROID
#include <QJniObject>
#include <QJniEnvironment>
#include <QCoreApplication>

InAppStore* global_inAppStore = nullptr;

QString fromJString(JNIEnv* env, jstring jstr)
{
    const char* str = env->GetStringUTFChars(jstr, 0);
    QString qstr(str);
    env->ReleaseStringUTFChars(jstr, str);
    return qstr;
}

void JNI_InitializeBilling()
{
    QJniObject activity = QNativeInterface::QAndroidApplication::context();
    activity.callMethod<void>("InitializeBilling" );
}

void JNI_Purchase(QString id)
{
    QJniObject activity = QNativeInterface::QAndroidApplication::context();
    QJniObject javaString = QJniObject::fromString(id);
    activity.callMethod<void>("MakePurchase", "(Ljava/lang/String;)V", javaString.object<jstring>());
}

void JNI_Subscribe(QString id)
{
    QJniObject activity = QNativeInterface::QAndroidApplication::context();
    QJniObject javaString = QJniObject::fromString(id);
    activity.callMethod<void>("MakeSubscription", "(Ljava/lang/String;)V", javaString.object<jstring>());
}

void JNI_RegisterProduct(QString id)
{
    QJniObject activity = QNativeInterface::QAndroidApplication::context();
    QJniObject javaString = QJniObject::fromString(id);
    activity.callMethod<void>("RegisterProduct", "(Ljava/lang/String;)V", javaString.object<jstring>());
}

void JNI_RegisterSubscription(QString id)
{
    QJniObject activity = QNativeInterface::QAndroidApplication::context();
    QJniObject javaString = QJniObject::fromString(id);
    activity.callMethod<void>("RegisterSubscription", "(Ljava/lang/String;)V", javaString.object<jstring>());
}

void onNativeProductKnown(JNIEnv *env, jobject thiz, jstring id, jboolean purchased, jstring title, jstring desc, jstring cost )
{
    Q_UNUSED(env)
    Q_UNUSED(thiz)

    QString productId(fromJString(env, id));
    bool isPurchased = purchased;
    QString sTitle(fromJString(env, title));
    QString sDesc(fromJString(env, desc));
    QString sCost(fromJString(env, cost));

    qDebug() << "onNativeProductKnown : "
             << productId << ":"
             << isPurchased << ":"
             << sTitle << ":"
             << sDesc << ":"
             << sCost;

    QMetaObject::invokeMethod(global_inAppStore, "onJavaProductKnown",
                              Qt::QueuedConnection,
                              Q_ARG(QString, productId),
                              Q_ARG(bool, purchased),
                              Q_ARG(QString, sTitle),
                              Q_ARG(QString, sDesc),
                              Q_ARG(QString, sCost));
}

void onNativeProductUnknown(JNIEnv *env, jobject thiz, jstring id)
{
    Q_UNUSED(env)
    Q_UNUSED(thiz)

    QString productId(fromJString(env, id));
    qDebug() << "onNativeProductUnknown : " << productId;
    QMetaObject::invokeMethod(global_inAppStore, "onJavaProductUnknown",
                              Qt::QueuedConnection,
                              Q_ARG(QString, productId));

}

void onNativeBillingInitialized(JNIEnv *env, jobject thiz)
{
    Q_UNUSED(env)
    Q_UNUSED(thiz)

    qDebug() << "onNativeBillingInitialized";

    QMetaObject::invokeMethod(global_inAppStore,
                              "onJavaBillingInitialised",
                              Qt::QueuedConnection);
}

void onNativeProductPurchased(JNIEnv *env, jobject thiz, jstring id)
{
    Q_UNUSED(env)
    Q_UNUSED(thiz)

    QString productId(fromJString(env, id));

    qDebug() << "onNativeProductPurchased : " << productId;

    QMetaObject::invokeMethod(global_inAppStore,
                              "onJavaProductPurchased",
                              Qt::QueuedConnection,
                              Q_ARG(QString, productId));

}

void HookAndroidJNI(InAppStore* appstore )
{
    global_inAppStore = appstore;

    if(global_inAppStore == nullptr)
        return;

    JNINativeMethod methods[] = {
        {
            "onNativeProductKnown",
            "(Ljava/lang/String;ZLjava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
            reinterpret_cast<void*>(onNativeProductKnown)
        },
        {
            "onNativeProductUnknown",
            "(Ljava/lang/String;)V",
            reinterpret_cast<void*>(onNativeProductUnknown)
        },
        {
            "onNativeBillingInitialized",
            "()V",
            reinterpret_cast<void*>(onNativeBillingInitialized)
        },
        {
            "onNativeProductPurchased",
            "(Ljava/lang/String;)V",
            reinterpret_cast<void*>(onNativeProductPurchased)
        }
    };

    QJniObject activity = QNativeInterface::QAndroidApplication::context();

    QJniEnvironment env;

    jclass objectClass = env->GetObjectClass(activity.object<jobject>());
    env->RegisterNatives(objectClass, methods, sizeof(methods) / sizeof(methods[0]));
    env->DeleteLocalRef(objectClass);
}
#endif

InAppStore::InAppStore(QString id, QObject *parent) :
    QObject(parent)
    ,_androidID(id)
#ifdef Q_OS_ANDROID2
  #if QT_VERSION >= QT_VERSION_CHECK(5,7,0)

    ,_store(new QInAppStore(this))
  #endif
#endif
{
}

InAppStore::~InAppStore()
{
#ifdef Q_OS_ANDROID
    HookAndroidJNI(nullptr);
#endif
}

void InAppStore::initialise()
{
#ifdef Q_OS_ANDROID
    HookAndroidJNI(this);
    JNI_InitializeBilling();
#else
    registerProducts();
#endif

    //setupPlaystoreConnections();
    setUpExemptions();
    //registerProducts();
}

void InAppStore::setUpExemptions()
{
    _androidIDs.push_back(anas);//Anas Phone
    _androidIDs.push_back(anas_note_8);

    _androidIDs.push_back("28c1e071cca73eaa");// Latest Raymond
    _androidIDs.push_back("34bcd56322883477");// Raymond 31/03/2019
    _androidIDs.push_back("7afc5768e275737e");// Raymond 23/05/2019
    _androidIDs.push_back("1d6682fe34413c70");//Dad
}

void InAppStore::setEvalTimeInSecs(QString id, int secs)
{
    Product& p = _products[id];
    p.setEvalPeriod(secs);
}

void InAppStore::update(const QString &id, float dt)
{ 
    Product& p = _products[id];
    bool bExpiredBefore = p.expired();
    p.update(dt);
    bool bExpiredAfter = p.expired();

#ifdef Q_OS_WIN32
    qDebug() << "InApp: " << id << " - " << p.secondsLeft();
#endif

    QSettings s;
    s.setValue(settingForProduct(id), p.secondsLeft());

    if(!isPurchased(id) && (bExpiredBefore||bExpiredAfter))
    {
        emit productPurchased(id,false);
        emit promptPurchasePage();

#ifdef __REMOVED__
        if(bExpiredBefore
#ifdef Q_OS_WIN32
            && false
#endif
                )
        {
            purchase(id);
        }
        else
        {
            QString str;

            if( p.isInApp())
                str = QString("To unlock this feature\n[%1]\na payment of %2 is required on the Playstore?\n").
                    arg(p.description()).arg(p.cost());
            else
                str = QString("To unlock this feature\n[%1]\na subscription fee is required on the Playstore?\n").
                    arg(p.description());

            emit promptProductPurchase(str, id);
        }            
#endif
    }
}

bool InAppStore::isFreeUser() const
{
    if( _androidID.isEmpty())
        return false;

    auto it = std::find(_androidIDs.begin(), _androidIDs.end(), _androidID);

    if( it == _androidIDs.end())
        return false;

    return *it == _androidID;
}

bool InAppStore::isPurchased(const QString &id) const
{
    auto it = _products.find(id);
    if( it == _products.end())
        return true;

    return it->second.isPurchasedOrUninitialised();
}

bool InAppStore::isInApp(const QString &id) const
{
    auto it = _products.find(id);
    if( it == _products.end())
        return true;

    return it->second.isInApp();
}

QString InAppStore::title(const QString &id) const
{
    auto it = _products.find(id);
    if( it == _products.end())
        return "";

    return it->second.title();
}

void InAppStore::purchase(const QString &id)
{
    if(isPurchased(id))
        return;

#ifdef Q_OS_WIN
    if(true)
    {
        QTimer::singleShot(1000, [id, this]//Purchase successful after 1 second.
        {
            _products[id].setStatus(Product::Purchased);
            emit productPurchased(id, true);
        });
    }
    else
    {
#ifdef Q_OS_ANDROID2
        QInAppProduct* product = _store->registeredProduct(id);

        if( product != 0)
        {
            _lastPurchaseID = id;

            QTimer::singleShot(4000, [this]
            {
                _lastPurchaseID = "";
            });

            product->purchase();
        }
#endif
    }
#endif


#ifdef Q_OS_ANDROID
    if(_products[id].isInApp())
        JNI_Purchase(id);
    else
        JNI_Subscribe(id);
#endif
}

int InAppStore::secondsLeft(const QString &id) const
{
    auto it = _products.find(id);
    if( it == _products.end())
        return 0;

    return it->second.secondsLeft();
}

int InAppStore::minutesLeft(const QString &id) const
{
    return std::ceil(secondsLeft(id)/60.0f);
}

QString InAppStore::cost(const QString &id) const
{
    auto it = _products.find(id);
    if( it == _products.end())
        return "0";

    return it->second.cost();
}

int InAppStore::numberOfPurchases() const
{
    int count = 0;
    for( auto& product : _products)
    {
        if( product.second.isPurchased())
            count++;
    }

    return count;
}

bool InAppStore::isMe() const
{
    return _androidID == me_samsung4
            || _androidID == me_samsung8
        #ifdef Q_OS_WIN
            || _androidID == AndroidID_Windows
        #endif
            || _androidID == android_tv
            || _androidID == rash
            ;
}

bool InAppStore::isAnas() const
{
    return _androidID == anas || _androidID == anas_note_8;
}

void InAppStore::registerProducts()
{
#ifdef Q_OS_ANDROID2
    _productCount = 0;
#endif
    _products.clear();

    const int secsInMin = 60;

    QSettings s;

    const int MaxTime = 5*60;
    int period = s.value(settingForProduct(AppStoreID_3DView), MaxTime).toInt();
    period = std::max(10*60, period);

    _products["Test"].setEvalPeriod(100);
    _products["android.test.purchased"].setEvalPeriod(100);

    _products[AppStoreID_3DView].setEvalPeriod(period);
    _products[AppStoreID_3DView].setDescription("3DView");
    _products[AppStoreID_3DView].setTitle("3DView");

    period = s.value(settingForProduct(AppStoreID_ProximityAlert), MaxTime).toInt();
    period = std::max(60*30, period);
    _products[AppStoreID_ProximityAlert].setEvalPeriod(period);
    _products[AppStoreID_ProximityAlert].setDescription("ProximityAlert");
    _products[AppStoreID_ProximityAlert].setTitle("ProximityAlert");

    period = s.value(settingForProduct(AppStoreID_RealTimeGPS), MaxTime).toInt();
    period = std::max(60*30, period);
    _products[AppStoreID_RealTimeGPS].setEvalPeriod(period);
    _products[AppStoreID_RealTimeGPS].setDescription("RealTimeGPS");
    _products[AppStoreID_RealTimeGPS].setTitle("RealTimeGPS");

    period = s.value(settingForProduct(AppStoreID_EstimatedPosition), MaxTime).toInt();
    period = std::max(60*30, period);
    _products[AppStoreID_EstimatedPosition].setEvalPeriod(period);
    _products[AppStoreID_EstimatedPosition].setDescription("EstimatedVehiclePosition");
    _products[AppStoreID_EstimatedPosition].setTitle("EstimatedVehiclePosition");

    period = s.value(settingForProduct(AppStoreID_Monthly_Subcriber), MaxTime).toInt();
    _products[AppStoreID_Monthly_Subcriber].setEvalPeriod(period);
    _products[AppStoreID_Monthly_Subcriber].setDescription("Ongoing support");
    _products[AppStoreID_Monthly_Subcriber].setTitle("Ongoing support");
    _products[AppStoreID_Monthly_Subcriber].setType(Product::subScription);

    period = s.value(settingForProduct(AppStoreID_LifeTime_Purchase), MaxTime).toInt();
    _products[AppStoreID_LifeTime_Purchase].setEvalPeriod(period);
    _products[AppStoreID_LifeTime_Purchase].setDescription("Ongoing support");
    _products[AppStoreID_LifeTime_Purchase].setTitle("Ongoing support");

#ifdef Q_OS_WIN32
    for(auto&item : _products)
    {
        item.second.setStatus(Product::NotPurchased);
        emit productKnown(item.first);
    }
    emit allProductsRegistered();
#endif

#ifdef Q_OS_ANDROID2
    for(auto& product:_products)
    {
        _store->registerProduct(QInAppProduct::Unlockable, product.first);
    }
#endif

#ifdef Q_OS_ANDROID
    for(auto& product:_products)
    {
        if( product.second.isInApp())
            JNI_RegisterProduct(product.first);
        else
            JNI_RegisterSubscription(product.first);
    }
#endif
}

#ifdef Q_OS_ANDROID2
void InAppStore::setupPlaystoreConnections()
{
#if QT_VERSION >= QT_VERSION_CHECK(5,7,0)
    connect( _store, SIGNAL(productRegistered(QInAppProduct*)), this, SLOT(inAppProductRegistered(QInAppProduct*)));

    connect(_store,SIGNAL(productUnknown(QInAppProduct::ProductType,QString)), this, SLOT(inAppProductUnknown(QInAppProduct::ProductType,QString)));

    connect( _store, SIGNAL(transactionReady(QInAppTransaction*)), this, SLOT(inAppTransactionReady(QInAppTransaction*)));
#endif
}

void InAppStore::incrementProductCount()
{
    _productCount ++;
#if QT_VERSION >= QT_VERSION_CHECK(5,7,0)
    if( _productCount == _products.size())
        _store->restorePurchases();
#endif
}
#endif

void InAppStore::onJavaBillingInitialised()
{
    qDebug() << "onJavaBillingInitialised";
    registerProducts();
}

void InAppStore::onJavaProductKnown(QString id, bool purchased, QString title, QString desc, QString cost)
{
    Product& p = _products[id];

    p.setStatus( purchased ? Product::Purchased:Product::NotPurchased);
    p.setTitle( title);
    p.setCost( cost );
    p.setDescription(desc);

    qDebug() << "Product Known: "
             << id << ":"
             << purchased << ":"
             << title << ":"
             << desc << ":"
             << cost;

    emit productKnown(id);
    _productsVerifiedWithJava++;
    if( _productsVerifiedWithJava == _products.size())
        emit allProductsRegistered();
}

void InAppStore::onJavaProductUnknown(QString id)
{
    _products[id].setStatus( Product::Uninitialised);
    qDebug() << "Product Unknown : " << id;
    _productsVerifiedWithJava++;
    if( _productsVerifiedWithJava == _products.size())
        emit allProductsRegistered();
}

void InAppStore::onJavaProductPurchased(QString id)
{
    qDebug() << "Product Purchased : " << id;
    _products[id].setStatus( Product::Purchased);
     emit productPurchased(id, true);
}

#ifdef Q_OS_ANDROID2
#if QT_VERSION >= QT_VERSION_CHECK(5,7,0)
void InAppStore::inAppProductUnknown(QInAppProduct::ProductType,QString identifier)
{
    qDebug() << "InAppStore - Unknown : " << identifier;
    _products[identifier]._status  = Product::Uninitialised;
    incrementProductCount();
}

void InAppStore::inAppProductRegistered(QInAppProduct *a)
{
    qDebug() << "InAppStore - Registered :" << a->identifier();
    Product& p = _products[a->identifier()];

    p._status  = Product::NotPurchased;
    p._title = a->title();
    p._cost = a->price();
    p._description = a->description();
    incrementProductCount();
}

void InAppStore::inAppTransactionReady(QInAppTransaction *a)
{
    qDebug() << "InAppStore - transactionReady :" << a->orderId() << "-" << a->status();

    if( _lastPurchaseID == a->product()->identifier()
            || a->status() == QInAppTransaction::PurchaseApproved
            || a->status() == QInAppTransaction::PurchaseRestored )
    {
        _products[a->product()->identifier()]._status = Product::Purchased;

        if( _lastPurchaseID.length() > 0)
        {
            emit productPurchased(a->product()->identifier(), true);
            _lastPurchaseID = "";
        }
    }

    a->finalize();
}
#endif
#endif

/////////////////////////////////////////////////////


void InAppStore::Product::setType(InAppStore::Product::purchaseType type)
{
    _type = type;
}

void InAppStore::Product::setTitle(QString s)
{
    _title = s;
}

void InAppStore::Product::setCost(QString s)
{
    _cost = s;
}

void InAppStore::Product::setEvalPeriod(int secs)
{
    _msTimeLeft = secs* 1000;
}

void InAppStore::Product::setStatus(InAppStore::Product::purchaseStatus s)
{
    _status = s;
}

void InAppStore::Product::setDescription(QString s)
{
    _description = s;
}

void InAppStore::Product::update(float dt)
{
    _msTimeLeft -= dt*1000;
    _msTimeLeft =  std::max(0, _msTimeLeft);
}

bool InAppStore::Product::expired() const
{
    return _msTimeLeft < 1000;
}

InAppStore::Product::purchaseStatus InAppStore::Product::status() const
{
    return _status;
}

QString InAppStore::Product::title() const
{
    return _title;
}

QString InAppStore::Product::cost() const
{
    return _cost;
}

QString InAppStore::Product::description() const
{
    return _description;
}

bool InAppStore::Product::isPurchasedOrUninitialised() const
{
    return _status == Purchased || _status == Uninitialised;
}

bool InAppStore::Product::isPurchased() const
{
    return _status == Purchased;
}

bool InAppStore::Product::isInApp() const
{
    return _type == inApp;
}

void InAppStore::Product::expire()
{
    _msTimeLeft = 0;
}

int InAppStore::Product::secondsLeft() const
{
    return _msTimeLeft / 1000;
}

#ifndef INAPPSTORE_H
#define INAPPSTORE_H

#include <QObject>
#include <QTimer>
#include <QString>
#include <vector>
#include <algorithm>
#include <jibbs/gps/GPSLocation.h>

class InAppStore : public QObject
{
    Q_OBJECT
public:
#ifdef Q_OS_WIN
    static const QString AndroidID_Windows;
#endif
    static const QString AppStoreID_3DView;
    static const QString AppStoreID_ProximityAlert;
    static const QString AppStoreID_RealTimeGPS;
    static const QString AppStoreID_EstimatedPosition;
    static const QString AppStoreID_Monthly_Subcriber;
    static const QString AppStoreID_LifeTime_Purchase;

    class Product
    {
    public:
        enum purchaseStatus {
            Uninitialised,
            NotPurchased,
            Purchased
        };

        enum purchaseType {
            inApp,
            subScription
        };

        void setType(purchaseType type);
        void setTitle(QString s);
        void setCost(QString s);
        void setEvalPeriod(int secs);
        void setStatus( purchaseStatus s);
        void setDescription(QString s);

        void update(float dt);
        bool expired() const;
        purchaseStatus status() const;
        QString title() const;
        QString cost() const;
        QString description() const;
        bool isPurchasedOrUninitialised() const;
        bool isPurchased() const;
        bool isInApp() const;
        void expire();
        int secondsLeft() const;

    private:
        purchaseType _type = inApp;
        QString _title;
        QString _cost;
        QString _description;
        purchaseStatus _status  = Uninitialised;
        int _msTimeLeft = 0;
    };

    explicit InAppStore(QString id, QObject *parent = nullptr);
    ~InAppStore();

    void initialise();
    void setUpExemptions();
    void setEvalTimeInSecs(QString id, int secs);
    void update(const QString& id, float dt);
    bool isFreeUser() const;
    bool isPurchased(const QString& id) const;
    bool isInApp(const QString& id) const;
    QString title(const QString& id) const;
    void purchase(const QString &id);
    int secondsLeft(const QString& id) const;
    int minutesLeft(const QString& id) const;
    QString cost(const QString& id) const;
    int numberOfPurchases() const;
    bool isMe() const;
    bool isAnas() const;

protected:
    void registerProducts();

#ifdef Q_OS_ANDROID2
    void setupPlaystoreConnections();
    void incrementProductCount();
#endif

public slots:
    void onJavaBillingInitialised();
    void onJavaProductKnown(QString id, bool purchased, QString title, QString desc, QString cost);
    void onJavaProductUnknown(QString id);
    void onJavaProductPurchased(QString id);

#ifdef Q_OS_ANDROID2
#if QT_VERSION >= QT_VERSION_CHECK(5,7,0)
protected slots:
   void inAppProductRegistered(QInAppProduct* a);
   void inAppProductUnknown(QInAppProduct::ProductType,QString identifier);
   void inAppTransactionReady(QInAppTransaction* a);
#endif
#endif


signals:
   void productKnown(QString id);
    void productPurchased(QString id, bool purchased);
    void promptProductPurchase(QString msg, QString id);
    void allProductsRegistered();
    void promptPurchasePage();

private:
#ifdef Q_OS_ANDROID2
#if QT_VERSION >= QT_VERSION_CHECK(5,7,0)
    QInAppStore* _store;
#endif
#endif
    QString _androidID;
    std::map<QString,Product> _products;
    std::vector<QString> _androidIDs;
    size_t _productsVerifiedWithJava = 0;

#ifdef Q_OS_ANDROID2
    size_t _productCount = 0;
    QString _lastPurchaseID;
#endif
};

#endif // INAPPSTORE_H

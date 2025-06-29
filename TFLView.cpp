#include <jibbs/utilities/stdafx.h>
#include <jibbs/vector/vector4.h>
#include <jibbs/math/qhdgtable.h>
#include <jibbs/maptiles/TileHelpers.h>

#include <QPainter>
#include <QHash>
#include <QQuickWindow>
#include <QQuickPaintedItem>
#include <QSettings>
#include <QQuickWindow>
#include <QDebug>
#include <QJsonArray>
#include <QNetworkReply>
#include <QAuthenticator>

#include <QOpenGLFramebufferObjectFormat>
#include <QOpenGLPaintDevice>
#include <QMutexLocker>
#include <QStringView>

#include <QCryptographicHash>
#include <QByteArray>

#include "MapRenderWorker.h"
#include "TFLModel.h"
#include "TFLView.h"
#include "WorldModel.h"
#include "TurnDirection.h"
#include "OSMData.h"

#include "screenUtils.h"

#ifdef Q_OS_ANDROID

#include <QJniObject>
#include <QJniEnvironment>

TFLView* g_TflView = nullptr;

void AndroidBatteryInfoCallBack(JNIEnv *env, jobject thiz, jint percent, jboolean isCharging)
{
    Q_UNUSED(env)
    Q_UNUSED(thiz)

    qDebug() << "Power Level : " << percent;

    QMetaObject::invokeMethod(g_TflView, "setBatteryInfo",
                              Qt::QueuedConnection,
                              Q_ARG(QString, QString::number(percent)),
                              Q_ARG(bool, isCharging));
}


void startBatteryListenerForAndroid(TFLView* view)
{
    g_TflView = view;

    if( g_TflView == nullptr)
        return;

    JNINativeMethod methods[] = {
        {
            "OnBatteryInfoAvailable",
            "(IZ)V",
            reinterpret_cast<void*>(AndroidBatteryInfoCallBack)
        }
    };

    QJniObject javaClass("com/enthusiasticcoder/tfltracker/BatteryListener");

    QJniEnvironment env;

    jclass objectClass = env->GetObjectClass(javaClass.object<jobject>());
    env->RegisterNatives(objectClass, methods, sizeof(methods) / sizeof(methods[0]));
    env->DeleteLocalRef(objectClass);

    QJniObject::callStaticMethod<void>("com/enthusiasticcoder/tfltracker/BatteryListener", "StartBatteryListener");

    // Checking for errors in the JNI
    if (env->ExceptionCheck()) {
        // Handle exception here.
        env->ExceptionClear();
    }
}
#endif


namespace {

bool operator<(const QColor & a, const QColor & b) {
    return a.redF() < b.redF()
           || a.greenF() < b.greenF()
           || a.blueF() < b.blueF()
           || a.alphaF() < b.alphaF();
}

void makeTransparent(QImage& img, QColor background)
{
    const int w = img.width();
    const int h = img.height();
    for( int x=0; x < w; ++x)
    {
        for(int y =0; y <h; ++y)
        {
            QColor pix = img.pixelColor(x,y);
            int r,g,b;
            pix.getRgb(&r, &g,&b);
            if( r> 200 && g > 200 && b> 200)
                img.setPixelColor(x,y, Qt::transparent);
        }
    }
}

bool busNumberComparerId(const  QStringView& leftId, const  QStringView& rightId)
{
    bool leftIsLetter = leftId[0].isLetter();
    bool rightIsLetter = rightId[0].isLetter();

    if( leftIsLetter != rightIsLetter)
        return rightIsLetter;

    if( leftIsLetter && rightIsLetter)
    {
        if( leftId[0] == rightId[0])
        {
            if( leftId[1].isLetter() && rightId[1].isLetter() )
            {
                if(leftId[1] == rightId[1])
                    return leftId.mid(2).toInt() < rightId.mid(2).toInt();
            }
            return leftId.mid(1).toInt() < rightId.mid(1).toInt();
        }
        else
            return leftId[0] < rightId[0];
    }

    return leftId.toInt() < rightId.toInt();
}

QByteArray generateSnapshotHash(const ViewState& snapShot)
{
    QByteArray data;

    data.append(snapShot.gpsOrigin.toString().c_str());
    data.append(QString::number(snapShot.middleOfScreen.width()).toUtf8());
    data.append(QString::number(snapShot.middleOfScreen.height()).toUtf8());

    return QCryptographicHash::hash(data, QCryptographicHash::Sha256);
}

}

QOpenGLFramebufferObject *TFLView::createFramebufferObject(const QSize &size)
{
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    format.setSamples(4);
    _size = size/_devicePixelRatio;
    return new QOpenGLFramebufferObject(size, format);
}

TFLView::TFLView(float dpr) :
    _devicePixelRatio(dpr),
    _mapWorkerThread(new QThread{this}),
    _mapWorker(new MapRenderWorker{&_osmRenderer, &_lineRenderer}),
    _vehicleRenderer(this)
{
    qRegisterMetaType<GPSLocation>();
    qRegisterMetaType<TFLViewCallback>();

    _3d->setQtTexManager(_texManager);

    connect(&_flatButtonManager, &FlatButtonManager::buttonPressed, this, &TFLView::onFlatButtonPressed);

    connect(this, &TFLView::emitBusyMsg, this, &TFLView::setBusyMsg);

    _tubeStnImg.load( ":/images/tfl/tube.png");
    _busStnImg.load( ":/images/tfl/bus.png");
    _overGrndStnImg.load( ":/images/tfl/overground.png");
    _DLRStnImg.load( ":/images/tfl/dlr.png");
    _RiverBusStnImage.load( ":/images/tfl/river-bus.png");
    _TFLRailStnImg.load( ":/images/tfl/elizabeth-line.png");
    _tramStnImg.load( ":/images/tfl/tram.png");
    _NationalRailStnImg.load( ":/images/tfl/national-rail.png");
    _IFSCableCarImage.load(":/images/tfl/IFScablecar.png");

    addFlatButtons();

    _lastTime = QTime::currentTime();
    setCompassValue(0.0f);

    _dirtyFlagTimer = new QTimer(this);
    _dirtyFlagTimer->setObjectName("RadarView::dirtyFlagTimer");
    _dirtyFlagTimer->setInterval(1000);

    _mapWorker->moveToThread(_mapWorkerThread);

    connect(_mapWorkerThread, &QThread::finished, _mapWorker, &QObject::deleteLater);
    connect(this, &TFLView::requestMapRender, _mapWorker, &MapRenderWorker::renderImage);
    connect(_mapWorker, &MapRenderWorker::imageRendered, this, &TFLView::mapImageRendered);

#ifdef Q_OS_ANDROID
    startBatteryListenerForAndroid(this);
#endif

    // _mapWorkerThread->start();
}

TFLView::~TFLView()
{
    qDebug() << Q_FUNC_INFO;

#ifdef Q_OS_ANDROID
    startBatteryListenerForAndroid(nullptr);
#endif
    emit aboutToBeDestroyed();

    _mapWorkerThread->quit();
    _mapWorkerThread->wait();

    saveSettings();
}

bool TFLView::isPaused() const
{
    return _bPaused;
}

bool TFLView::isReady() const
{
    return _bIsReady;
}

bool TFLView::isView3D() const
{
    return _b3DMode;
}

const QSize& TFLView::size() const
{
    return _size;
}

QRect TFLView::geometry() const
{
    return QRect(QPoint(),_size);
}

QRect TFLView::rect() const
{
    return geometry();
}

void TFLView::parseLineArrival(QByteArray data)
{
    QJsonDocument doc = QJsonDocument::fromJson(data);
    _model->parseLineArrival(doc);
}

void TFLView::parseLineArrival(QByteArray data, QString lineId)
{
    QDomDocument xmlDoc;
    xmlDoc.setContent(data);
    _model->parseLineArrival(xmlDoc, lineId);
}

void TFLView::parseLineArrival(const NationalRailPositionProvider::Train &t)
{
    _model->parseLineArrival(t);
}

void TFLView::removeAllVehicles()
{
    _model->removeAllVehicles();
}

void TFLView::removeVehicles(QString line)
{
    _model->removeVehicles(line);
}

void TFLView::setUseVehicleBehaviour(bool checked)
{
    _model->setUseVehicleBehaviour(checked);
}

void TFLView::setPiccHeathrowDestnNormalFormat(bool checked)
{
    _model->setPiccHeathrowDestnNormalFormat(checked);
    _model->update();
}

void TFLView::setCircleColorOverride(QString id, bool on)
{
    _model->setCircleColorOverride(id, on);
}

void TFLView::removeOldVehicles()
{
    _model->removeOldVehicles();
}

void TFLView::setBusyMsg(QString msg)
{
    _strBusyMsg = msg;
    update();
}

void TFLView::addFlatButtons()
{
    const float fY = 0.0f;
    const float fX = 0.167f;
    const float fdX = 0.167f;
    const FlatButtonManager::VAlignment algn = FlatButtonManager::AlignTop;

   _flatButtonManager.addButton(_flatMainMenu, fX, fY, algn);
   _flatButtonManager.setImage(_flatMainMenu, ":/QtAtcX/images/main-menu.png", Qt::white );

   _flatButtonManager.addButton(_flatShowNormal, 1*fdX+fX, fY, algn, false);
   _flatButtonManager.setImage(_flatShowNormal, ":/QtAtcX/images/shownormal32x32.png", Qt::white);

   _flatButtonManager.addButton(_flatShowMax, 1*fdX + fX, fY, algn, false);
   _flatButtonManager.setImage( _flatShowMax, ":/QtAtcX/images/showmaximum32x32.png", Qt::white);

   _flatButtonManager.addButton(_flatSatId, 2*fdX+fX, fY, algn);
   _flatButtonManager.setImage(_flatSatId, ":/QtAtcX/images/satlogo32x32.png", Qt::white);

   _flatButtonManager.addButton(_flatShow2D, 3*fdX+fX, fY, algn);
   _flatButtonManager.setImage(_flatShow2D, ":/QtAtcX/images/2D-flat.png", Qt::white);

   _flatButtonManager.addButton(_flatShow3D, 3*fdX+fX, fY, algn);
   _flatButtonManager.setImage(_flatShow3D, ":/QtAtcX/images/3D-flat.png", Qt::white);

   _flatButtonManager.addButton(_flatResetCompass, 4*fdX+fX, fY, algn);
   _flatButtonManager.setImage(_flatResetCompass, ":/QtAtcX/images/compassicon32x32.png", Qt::white);

   _flatButtonManager.addButton(_flatVR, 4*fdX+fX, fY, algn);
   _flatButtonManager.setImage(_flatVR, ":/QtAtcX/images/vr.png", Qt::white);

   _flatButtonManager.addButton(_flatZoomOut,0*fdX+ fX, 1.0f, FlatButtonManager::AlignBottom );
   _flatButtonManager.setImage(_flatZoomOut, ":/QtAtcX/images/minus-flat.png", Qt::white);

   _flatButtonManager.addButton(_flatZoomIn, fX+fdX*4, 1.0f, FlatButtonManager::AlignBottom );
   _flatButtonManager.setImage(_flatZoomIn, ":/QtAtcX/images/plus-flat.png", Qt::white);

   _flatButtonManager.addButton(_flatWest,1*fdX+ fX, 1.0f, FlatButtonManager::AlignBottom );
   _flatButtonManager.setImage(_flatWest, ":/QtAtcX/images/west.png", Qt::white);

   _flatButtonManager.addButton(_flatEast,3*fdX+ fX, 1.0f, FlatButtonManager::AlignBottom );
   _flatButtonManager.setImage(_flatEast, ":/QtAtcX/images/east.png", Qt::white);

   _flatButtonManager.addButton(_flatRadarToggleColor, 2*fdX+fX, 1.0f, FlatButtonManager::AlignBottom);
   _flatButtonManager.setImage(_flatRadarToggleColor, ":/QtAtcX/images/radar_color_toggle.png", Qt::white);

   _flatButtonManager.addButton(_flatProxmOn, 2*fdX+fX, 1.0, FlatButtonManager::AlignBottom, false);
   _flatButtonManager.setImage(_flatProxmOn, ":/QtAtcX/images/proximity_mute_on.png", Qt::white);

   _flatButtonManager.addButton(_flatProxmOff, 2*fdX+fX, 1.0, FlatButtonManager::AlignBottom, false);
   _flatButtonManager.setImage(_flatProxmOff, ":/QtAtcX/images/proximity_mute_off.png", Qt::white);

   _flatButtonManager.setObeySize(_flatSubscription, false);
   _flatButtonManager.addButton(_flatSubscription, 0.99, 0.0, FlatButtonManager::AlignTop, false);
   _flatButtonManager.setImage(_flatSubscription, ":/QtAtcX/images/subscription.png", Qt::white);
   _flatButtonManager.setHAlign(_flatSubscription, FlatButtonManager::AlignRight);
}


void TFLView::onFlatButtonPressed(QString id)
{
#ifdef Q_OS_WIN32
    qDebug() << "Flat button pressed :" << id;
#endif
    if( id == _flatSatId)
    {
        QTime now = QTime::currentTime();
        bool realTimeGps = isRealTimeGPS();

        if( _lastTimeGPSClicked.isValid() && _lastTimeGPSClicked.msecsTo(now) < 500)
            realTimeGps = !realTimeGps;

        _lastTimeGPSClicked = now;

        emit onWantRealTimeGPS(realTimeGps);

        if( (_bLookingforGPS = !realTimeGps))
            emit onWantGPSUpdate();
    }
    else if( id  == _flatResetCompass)
    {
        emit onWantCompassMode();
    }
    else if( id == _flatShowMax)
    {
        emit wantShowMaximum();
    }
    else if( id == _flatShowNormal)
    {
        emit wantShowNormal();
    }
    else if( id == _flatShow2D)
    {
        emit wantShow3D(false);
    }
    else if( id == _flatShow3D)
    {
        emit wantShow3D(true);
    }
    else if( id == _flatZoomIn)
    {
        if( isView3D())
            set3DZoom( get3DZoom() + 0.5f);
        else
            onZoom(0.5f);
    }
    else if( id == _flatZoomOut)
    {
        if( isView3D())
            set3DZoom( get3DZoom() - 0.5f);
        else
            onZoom(-0.5f);
    }
    else if( id == _flatMainMenu)
    {
        emit showMenuOptions();
    }
    else if( id == _flatVR)
    {
        emit wantVR();
    }
    else if( id == _flatRadarToggleColor)
    {
        emit wantToggleColor();
    }
    else if( id == _flatProxmOn)
    {
        emit wantMuteProximity(true);
    }
    else if( id == _flatProxmOff)
    {
        emit wantMuteProximity(false);
    }
    else if( id == _flatSubscription)
    {
        emit wantToSubscribe();
    }
    else if( id == _flatWest)
    {
        emit wantToGo(GPSLocation(51.5019,-0.199202));
    }
    else if( id == _flatEast)
    {
        emit wantToGo(GPSLocation(51.5101,-0.0740329));
    }
}

void TFLView::applyCameraPitchBounds()
{
    if( _camera._pitch > 90.0f)
        _camera._pitch = 90.0f;

    if( _camera._pitch < -90.0f)
        _camera._pitch = -90.0f;
}

void TFLView::applyCompassTransform(QPainter &p, bool bScale)
{
    QSize sz = middleOfScreen();

    p.translate(sz.width(), sz.height());
    p.rotate(-_camera._heading);
    if( bScale)
        p.scale(getCurrentScale(), getCurrentScale());
    p.translate(-sz.width(), -sz.height());
}

void TFLView::showSubscribeButton(bool show)
{
    _flatButtonManager.setImgVisibility(_flatSubscription, show);
}

QPoint TFLView::invPtInScreen(const QPoint &pt)
{
    QSize sz = middleOfScreen();

    QTransform t;
    t.translate(sz.width(), sz.height());
    t.rotate(_camera._heading);
    t.scale(getCurrentScale(), getCurrentScale());
    t.translate(-sz.width(), -sz.height());
    return t.map(pt);
}

void TFLView::setGPSHdgCutOffSpd(int spd)
{
    _gpsMinSpdSensitivity = spd;
}

int TFLView::getGPSHdgCutOffSpd() const
{
    return _gpsMinSpdSensitivity;
}

void TFLView::updateTransformCompass()
{
    _compassTransform.reset();
    _compassTransform.rotate(_camera._heading);

    _invCompassTransform.reset();
    _invCompassTransform.rotate(-_camera._heading);
}

void TFLView::setCompassValue(float value, bool bInstant)
{
    _finalCompassReading = value;
    _finalCompassReading = MathSupport<float>::normAng(_finalCompassReading);

    if(bInstant)
    {
        _camera._heading = _finalCompassReading;
        _compassTurnRate = 0.0f;

        updateTransformCompass();
        _rebuildOsmImg = true;
    }

    update();
}

void TFLView::setCamera(const EulerF &e)
{
    _camera = e;
    setTilesDirty();
    update();
}

void TFLView::set3DZoom(float f)
{
    _3dZoomFactor = f;
    if( _3dZoomFactor < 1.0f)
        _3dZoomFactor = 1.0f;

    update();
}

void TFLView::mousePressHandler(QPoint pt)
{
    _mouseLastPoint = pt;
    _bMouseDown = true;
    // _gpsClickLocation = _gpsOrigin;

    update();

    if( _flatButtonManager.handleMouseDown(pt))
        return;

    _mouseAreaManager.handleMouseDown(pt);
}

void TFLView::mouseMoveHandler(QPoint pt)
{
    QPoint diff = _mouseLastPoint - pt;

    bool mouseMovedEnough = _bMouseDown && diff.manhattanLength() > 5;
    _bMouseCapture |= mouseMovedEnough;

    if(!mouseMovedEnough)
        return ;

    update();

    if( _flatButtonManager.handleMouseMove(pt))
        return;

    if( _mouseAreaManager.handleMouseMove(pt))
        return;

    //setCursor(Qt::OpenHandCursor);
    _mouseLastPoint = pt;

    TranslatePosition(diff);
}

void TFLView::triggerArrivalStatusPage(QString id, QString name, QString stopLetter, QString towards)
{
    emit showQMLPage("qmlpages/ArrivalStatusPage.qml",
                     QVariantList()
                     << id
                     << name
                     << stopLetter
                     << towards);
}

void TFLView::triggerArrivalNationalRailStatusPage(QString CRC, QString name)
{
    emit showQMLPage("qmlpages/ArrivalNationalStatusPage.qml",
                     QVariantList()
                     << CRC
                     << name);
}

void TFLView::selectVehicle(QString id)
{
    getWorldModel()->setSelectedVehicle(id);
    update();
}

void TFLView::setBatteryInfo(QString percent, bool charging)
{
    _batteryCharge = percent;
    _batteryStatus = charging;
}


void TFLView::mouseReleaseHandler(QPoint pos)
{
    _mouseAreaManager.handleMouseUp(pos);

    if( !_bMouseDown )
    {
        _flatButtonManager.handleIgnore();
    }
    else
    {
        QVariantList list;

        bool bFlatManagerHandled = _flatButtonManager.handleMouseUp(pos);

        if( _bMouseCapture)
        {
            //unsetCursor();
        }
        else if( !bFlatManagerHandled)
        {
            bool bHandled = false;

            int sensitivity = 20;

            if( isView3D())
                sensitivity = 40;

            QPoint ptSensitivity(sensitivity, sensitivity);

            ptSensitivity = _invCompassTransform.map(ptSensitivity);

            QPoint topLeft = pos - ptSensitivity;
            QPoint bottomRight = pos + ptSensitivity;

            if( !isView3D())
            {
                auto stationIds = _lineRenderer.getStnIdFromPoint(invPtInScreen(pos));
                auto natstationIds = _lineRenderer.getNatStnNameFromPoint(invPtInScreen(pos));
                const auto& items = getWorldModel()->getVehiclesInArea(toGPS(topLeft, true), toGPS(bottomRight, true));

                for(const auto& stationId : std::as_const(stationIds))
                {
                    std::shared_ptr<StopPoint> stopPoint = getWorldModel()->findStationId(stationId);
                    if( stopPoint )
                    {
                        QJsonObject obj;
                        obj["mode"] = stopPoint->mode;
                        obj["type"] = 1;
                        obj["id"] = stopPoint->id;
                        obj["name"] = stopPoint->name;
                        obj["stopLetter"] = stopPoint->stopLetter;
                        obj["towards"] = stopPoint->towards;
                        list << obj;
                    }
                }

                for(const auto& stationId : std::as_const(natstationIds))
                {
                    QString CRC = getWorldModel()->findCRCFromStationName(stationId);
                    std::shared_ptr<StopPoint> stopPoint = getWorldModel()->findNationalRailID(CRC);

                    if( stopPoint )
                    {
                        QJsonObject obj;
                        obj["mode"] = "national-rail";
                        obj["type"] = 2;
                        obj["id"] = CRC;
                        obj["name"] = QString(stopPoint->name).replace("Rail Station", "National Rail");
                        obj["stopLetter"] = stopPoint->stopLetter;

                        list << obj;
                    }
                }

                for( const auto& item : items)
                {
                    const std::shared_ptr<Vehicle>& vehicle = getWorldModel()->getVehicle(item);

                    QJsonObject obj;
                    obj["mode"] = "vehicle";
                    obj["type"] = 0;
                    obj["id"] = item;
                    obj["name"] = vehicle->_lineId + ":" + vehicle->_destinationName;
                    obj["stopLetter"] = item;

                    list << obj;
                }
            }

            if( !bHandled)
                if( (bHandled = !list.empty()))
                {
                    if( list.size() == 1)
                    {
                        QJsonObject obj = list[0].value<QJsonObject>();

                        if( obj["type"].toInt() == 1)
                           triggerArrivalStatusPage(obj["id"].toString(),
                                                    obj["name"].toString(),
                                                    obj["stopLetter"].toString(),
                                                    obj["towards"].toString());

                        else if(obj["type"].toInt() ==2)
                            triggerArrivalNationalRailStatusPage(obj["id"].toString(), obj["name"].toString());

                        else
                            bHandled = false;
                    }
                    else
                        emit showQMLSelectStopPointPage(list);
                }

            if( !bHandled)
            {
                if( isView3D())
                    bHandled = refresh3DBlipSelected(topLeft, bottomRight);
                else
                    bHandled = refresh2DBlipSelected(toGPS(topLeft, true), toGPS(bottomRight, true));
            }

            if( !bHandled)
                _flatButtonManager.setVisibility(!_flatButtonManager.getVisibility());
        }
    }

    _bMouseCapture = false;
    _bMouseDown = false;
    _currentScaleFactor = 1.0;
    // startMapRendering();
    updateCache();
    updateViewBox();
    update();
}

void TFLView::wheelHandler(QPoint pt)
{
    if(isView3D())
    {
        _3dZoomFactor += pt.y()/320.0f;
        set3DZoom(_3dZoomFactor);
    }
    else
    {
        onZoom(pt.y() / 8.0f / 10.0f);
    }
}

void TFLView::onZoomIn()
{
    onZoom(1.5f);
}

void TFLView::onZoomOut()
{
    onZoom(-1.5f);
}

void TFLView::onZoom(float fDiff)
{
    float pix = _PixelsPerMile;

    if (pix < 10.0f)
        pix  += (fDiff > 0 ? 1 : -1)* pix  / 10.0f;
    else
        pix  += fDiff + (fDiff > 0 ? 1 : -1)*pix  / 10.0f;

    onFinalisePixelPerMile(pix);
    updateViewBox(true);
    update();
}

QSize TFLView::middleOfScreen() const
{
#ifdef _DONT_WANT_
    if( isRealTimeGPS())
    {
        QSize sz = size();
        return QSize(sz.width()/2, sz.height()*0.6f);
    }
#endif

    return QSize(size()/2);
}

void TFLView::touchHandler(touchEventData data)
{
    switch (data.type)
    {
    case QEvent::TouchBegin:
        _bInPinch = true;
        _PinchPixelsPerMile = _PixelsPerMile;
        _pinch3DzoomFactor = _3dZoomFactor;
        _pinchCompassReading = _camera._heading;
        _rotUnstuck = false;
        _lockRotationForEver = false;
        _lastHeight = _gpsOrigin._height;
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
    {
        if( data.type == QEvent::TouchEnd)
            _bInPinch = false;

        if (data.touchPointCounts == 1)
        {
            if( data.type==QEvent::TouchBegin)
            {
                mousePressHandler(data.singlePt);
            }
            else if( data.type==QEvent::TouchUpdate)
            {
                mouseMoveHandler(data.singlePt);
            }
            else if( data.type==QEvent::TouchEnd)
            {
                mouseReleaseHandler(data.singlePt);
            }
        }
        else if (data.touchPointCounts == 2)
        {
            _currentScaleFactor = data.currentScaleFactor;
            _bMouseDown = false;

            if( isView3D())
            {
                set3DZoom( _pinch3DzoomFactor * _currentScaleFactor);
            }
            else
            {
                QPointF diffPt = data.diffPt;
                QPointF diffPt0 = data.diffPt0;

                float fDeg0 = RadiansToDegrees( std::atan2(diffPt0.x(), diffPt0.y() ));
                float fDeg1 = RadiansToDegrees(std::atan2(diffPt.x(), diffPt.y()));

                onFinalisePixelPerMile(_PinchPixelsPerMile*_currentScaleFactor);

                if( !_rotUnstuck )
                    if( _currentScaleFactor > 1.1 || _currentScaleFactor< 0.9)
                        _lockRotationForEver = true;

                float fDiffDeg = fDeg1 - fDeg0;
                if( !_lockRotationForEver)
                    if( std::abs(fDiffDeg) > 5)
                        _rotUnstuck = true;

                if( _rotUnstuck)
                    setCompassValue(_pinchCompassReading + fDiffDeg, true);
                else if( !isView3D())
                    setTilesDirty();
            }

            update();
        }
    }
    default:
        break;
    }
}

QString TFLView::getOpenGLInfo() const
{
    return _openGLInfo;
}

void TFLView::recordOpenGLInfo()
{
    QOpenGLContext* context = QOpenGLContext::currentContext();

    QOpenGLFunctions* functions = context->functions();
    QSurfaceFormat ftm = context->format();

    QString str;
    str = QString("Vendor:%1<br>Renderer:%2<br>Version:%3.%4<br>Profile:%5<br>GLES:%6<br>Shading Version:%7<br>Multi-Threaded Mode:%8")
              .arg((const char*)functions->glGetString(GL_VENDOR))
              .arg((const char*)functions->glGetString(GL_RENDERER))
              .arg(ftm.majorVersion()).arg(ftm.minorVersion())
              .arg(ftm.profile() == QSurfaceFormat::CoreProfile ?QStringLiteral("Core"):QStringLiteral("Compatibility"))
              .arg(context->isOpenGLES()?QStringLiteral("Yes"):QStringLiteral("No"))
              .arg((const char*)functions->glGetString(GL_SHADING_LANGUAGE_VERSION))
              .arg(_isMultiThreaded?"Yes":"No");


    str.append("<br>Extensions:<br>");
    auto extensions = context->extensions();
    for(auto& ext : std::as_const(extensions))
        str.append(QString("%1<br>").arg((const char*)ext));

    _openGLInfo = str;
}

void TFLView::setDataDirty()
{
    _bUpdateData = true;
}

void TFLView::tellFullScreen(bool bIsFullScreen)
{
    _bIsFullScreen = bIsFullScreen;
    _flatButtonManager.setImgVisibility(_flatShowMax, !bIsFullScreen );
    _flatButtonManager.setImgVisibility(_flatShowNormal, bIsFullScreen );
}


int TFLView::width() const
{
    return _size.width();
}

int TFLView::height() const
{
    return _size.height();
}

void TFLView::sync()
{
    if( !_initGLDone)
    {
        _initGLDone = true;
        QOpenGLFunctions::initializeOpenGLFunctions();
        m_device.reset(new QOpenGLPaintDevice);
        recordOpenGLInfo();

        tellFullScreen(false);

        connect(_dirtyFlagTimer, &QTimer::timeout, this, &TFLView::setDataDirty);

        _dirtyFlagTimer->start();
    }

    m_device->setSize(_size * _devicePixelRatio);
    m_device->setDevicePixelRatio(_devicePixelRatio);

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    if( !_glueReady)
        return;

    _size = QSizeF(width(),height()).toSize();

    if( _prevSize != _size || _resizeWanted )
    {
        setTilesDirty();
        _prevSize = _size;
        _resizeWanted = false;
        const QSize& sz = _size;
        _mouseAreaManager.handleResize(sz);
        updateViewBox(true);

        _flatButtonManager.setSize(sz, QPoint(0,_topHeightWanted+_perimeterHeightWanted), QPoint(0,-_bottomHeightWanted-_perimeterHeightWanted));
#ifdef Q_OS_WIN32
        qDebug() << Q_FUNC_INFO;
#endif
    }
}


void TFLView::initLinesLogo()
{
    QHash<LineType::mode, QImage> logos;

    logos[LineType::tube] = getTubeStnImage();
    logos[LineType::bus] = getBusStnImage();
    logos[LineType::tram] = getTramStnImage();
    logos[LineType::overground] = getOvergroundStnImage();
    logos[LineType::dlr] = getDLRStnImage();
    logos[LineType::elizabeth] = getElizabethRailStnImage();
    logos[LineType::national_rail] = getNationalRailImage();
    logos[LineType::river_bus] = getRiverBusImage();
    logos[LineType::river_tour] = getRiverBusImage();
    logos[LineType::cable_car] = getIFSCableCarImage();

    _lineRenderer.init(logos);
}

void TFLView::initOSM()
{
    _osmRenderer.init();
    _osmRenderer.loadSettings();

}

void TFLView::updateViewBox(bool bForced)
{
#ifdef Q_OS_WIN
//    if( !_bMouseCapture)
//        qDebug() << Q_FUNC_INFO;
#endif

    setTilesDirty();

    if( _bMouseCapture && !bForced)
        return;

    if(bForced)
        _gpsClickLocation = _gpsOrigin;

    updateCache();
}

qreal TFLView::getCurrentScale() const
{
    return _currentScaleFactor;
}

void TFLView::setShowTopLeftInfo(bool bShow)
{
    _showTopInfo = bShow;
}

bool TFLView::isTotallyReady() const
{
    return (isView3D() && _3d->isReady()) || (!isView3D() && isReady());
}

bool TFLView::ptInScreen(const GPSLocation &loc) const
{
    return ptInScreen(toScreen(loc, true));
}

bool TFLView::ptInScreen(const QPoint& pt) const
{
    return screenUtils::ptInScreen(size(), pt);
}

QPoint TFLView::toScreen(const GPSLocation & location, bool bApplyTransform) const
{
    return screenUtils::toScreen(createToScreenSnapshot(), location, bApplyTransform);
}

ViewState TFLView::createToScreenSnapshot() const
{
    ViewState snapshot;
    snapshot.gpsOrigin = _gpsOrigin;
    snapshot.pixelsPerMile = _PixelsPerMile;
    snapshot.middleOfScreen = middleOfScreen();
    snapshot.invCompassTransform = _invCompassTransform; // Capture the current transformation matrix
    snapshot.mapNight = _mapNight;
    snapshot.size = _size;
    snapshot.world = _model;
    snapshot.compassValue = getCompassValue();
    snapshot.boundaryFromRange = boundaryFromRange(snapshot, 800.3f);
    snapshot.boundaryView = getBoundaryView(false);
    snapshot.devicePixelRatio = _devicePixelRatio;
    return snapshot;
}

QRect TFLView::boundaryFromRange(const ViewState& viewState, float meters) const
{
    QPoint p1 = screenUtils::toScreen(viewState, _gpsOrigin + Vector3F(-meters,0,-meters));
    QPoint p2 = screenUtils::toScreen(viewState, _gpsOrigin + Vector3F(meters,0,meters));
    return QRect(p1, p2);
}

QRect TFLView::boundaryFromRange(float meters) const
{
    QPoint p1 = toScreen(_gpsOrigin + Vector3F(-meters,0,-meters));
    QPoint p2 = toScreen(_gpsOrigin + Vector3F(meters,0,meters));
    return QRect(p1, p2);
}

void TFLView::setMapNight(bool night)
{
    _mapNight = night;
}

bool TFLView::isMapNight() const
{
    return _mapNight;
}

GPSLocation TFLView::toGPS(const QPoint& pt, bool bApplyTransform) const
{
    const QSize& sz = middleOfScreen();
    QPoint center(sz.width(), sz.height());
    QPoint ptDiff = pt - center;
    QPoint ptRot;

    if( bApplyTransform)
        ptRot = _compassTransform.map(ptDiff);
    else
        ptRot = ptDiff;

    Vector3F dist = Vector3F(ptRot.x(), 0, ptRot.y())/_PixelsPerMile * 1609.334f;

    return _gpsOrigin + dist;
}


void TFLView::paintNative(float dt)
{
    loadResourcesIfRequired();

    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_DEPTH_TEST);

    _mouseAreaManager.update();

    _lastUpdateTime = QTime::currentTime();

    if( isTotallyReady() )
    {
        bool bCompassUpdated = updateCompassValue(dt);
        bool bGPSUpdated = updateGPSValue(dt);

        if( bGPSUpdated)
            updateCache();

        if( bGPSUpdated || bCompassUpdated)
        {
            updateViewBox();
            update();
        }
    }
    else
    {
        glClearColor(0,0,0,1);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    updateTransformCompass();

    if( isView3D())
        _3d->paintGL();
    else
        paintGL2D();

}

bool TFLView::updateCompassValue(float fDt)
{
    float dDiffCompass = std::abs(TurnDirection::GetTurnDiff(_camera._heading, _finalCompassReading));
    if( dDiffCompass < 0.001f)
        return false;

    TurnDirection::Dir d = TurnDirection::GetTurnDir(_camera._heading, _finalCompassReading);

    float fFactor = d == TurnDirection::Dir::Left ? -1 :1;

    float wantedCompassTurnRate = std::min(dDiffCompass, 36.0f)* fFactor;
    float fDiffCompassRate = std::abs(_compassTurnRate-wantedCompassTurnRate);

    int f2 = _compassTurnRate < wantedCompassTurnRate ? 1:-1;

    _compassTurnRate += f2 * fDiffCompassRate * fDt;

    float fMin = std::min(dDiffCompass, std::abs(_compassTurnRate))*fDt;

    if( dDiffCompass < 0.15f)
    {
        _camera._heading = _finalCompassReading;
        _compassTurnRate = 0.0f;
    }
    else
        _camera._heading += fFactor * fMin;

    _camera._heading = MathSupport<float>::normAng(_camera._heading);

    return true;
}

bool TFLView::updateGPSValue(float fDt)
{
    float distGPSDiff = _gpsOrigin.distanceTo(_gpsOriginTarget);

    if( distGPSDiff < 1.0f)
        return false;

    float fBearing = _gpsOrigin.bearingTo(_gpsOriginTarget);

    const QuarternionF& qHdg = QHDG(fBearing);

    Vector3F vecUnit = QVRotate(qHdg, Vector3F(0,0,-1));

    const float mMinDist = 3000;
    const float nmZero = 100;

    float velocity = std::min(mMinDist, distGPSDiff);

    if( velocity < mMinDist)
        velocity = std::max(distGPSDiff/2.0f, velocity);

    if( velocity < nmZero || distGPSDiff > 3000)
        _gpsOrigin = _gpsOriginTarget;
    else
        _gpsOrigin = _gpsOrigin + velocity * fDt * vecUnit;

    return true;
}

void TFLView::paintGL2D()
{
    //color3 colSea;
    color3 colGrnd;

//    const auto& profile = getRadarProfile();

    //colSea = color3( 0,0,0.9f);

    if( _mapNight)
        colGrnd = color3(0.1f,0.1f,0.1f);
    else
        colGrnd = color3(0.85f,0.85f,0.85f);

    glClearColor(colGrnd.r, colGrnd.g, colGrnd.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void TFLView::loadResourcesIfRequired()
{
    if( _bResourcesLoaded )
        return;

    initLinesLogo();

    initOSM();

     _bResourcesLoaded = true;
    _texManager->init();

    _tileManager.setFolder(":/data/OSMTiles");

    MouseArea& hdgArea = _mouseAreaManager.add("Heading");

    hdgArea.setSizeHandler([](const QSize& sze)
    {
        const int dW = sze.width() < sze.height() ? sze.width() / 7 : sze.height()/7;
        return QRect(0, sze.height()-dW, sze.width(), dW);
    });

    hdgArea.setEnabledHandler([this]()
    {
        return isView3D();
    });

    hdgArea.setMouseMoveListener( [this](const QPoint& diffPt)
    {
        float value = getCompassValue();
        setCompassValue(value += diffPt.x()/4.0f/_3dZoomFactor, true);
    });

    hdgArea.setPaintHandler(this, &TFLView::hdgAreaPaintHandler);

    MouseArea& altArea = _mouseAreaManager.add("Height");

    altArea.setEnabledHandler([this]()
    {
        return isView3D();
    });

    altArea.setSizeHandler([](const QSize& sze)
    {
        const int dW = sze.width() < sze.height() ? sze.width() / 8 : sze.height()/8;
        return QRect(sze.width()- dW, dW, dW, sze.height()-2*dW);
    });

    altArea.setMouseMoveListener([this](const QPoint& diffPt)
    {
        increment3dHeight(-diffPt.y()*8.0F/_3dZoomFactor);
        update();
    });

    altArea.setPaintHandler(this, &TFLView::altAreaPaintHandler);

    MouseArea& pitchArea = _mouseAreaManager.add("Pitch");

    pitchArea.setEnabledHandler([this]()
    {
        return isView3D();
    });

    pitchArea.setSizeHandler([](const QSize& sze)
    {
        const int dW = sze.width() < sze.height() ? sze.width() / 8 : sze.height()/8;
        return QRect(0, dW, dW, sze.height()-2*dW);
    });

    pitchArea.setMouseMoveListener([this](const QPoint& diffPt)
    {
        _camera._pitch += diffPt.y() / 4.0f/_3dZoomFactor;

        applyCameraPitchBounds();
        update();
    });

    pitchArea.setPaintHandler(this, &TFLView::pitchAreaPaintHandler);

    _mouseAreaManager.handleResize(size());
    setView3D(_b3DMode);
    emit onDisplayInitialised();
}

void TFLView::increment3dHeight(float fIncr)
{
    _gpsOrigin._height += fIncr;
    _gpsOrigin._height = std::max(5.0f, _gpsOrigin._height);
    _gpsOrigin._height = std::min(16800.0f, _gpsOrigin._height);
    _gpsOriginTarget._height = _gpsOrigin._height;
    updateSelectedVehicleVector();
    updateViewBox();
    update();
}

void TFLView::updateFlatButtonVisibility()
{
    _flatButtonManager.setImgVisibility(_flatShow2D, _b3DMode );
    _flatButtonManager.setImgVisibility(_flatShow3D, !_b3DMode );

    _flatButtonManager.setImgVisibility(_flatRadarToggleColor, !_proximityActive);

    _flatButtonManager.setImgVisibility(_flatProxmOn, _proximityActive && _proximityActiveSound);
    _flatButtonManager.setImgVisibility(_flatProxmOff, _proximityActive && !_proximityActiveSound);

    _flatButtonManager.setImgVisibility(_flatResetCompass, !_b3DMode);
    _flatButtonManager.setImgVisibility(_flatVR, _b3DMode);
}

void TFLView::setView3D(bool b3D)
{
    if( !b3D && _3d->isVRActive())
        _3d->triggerVR();

    _b3DMode = b3D;

    if( _b3DMode)
    {
        _3d->init();
    }

    updateFlatButtonVisibility();
    updateViewBox(true);
    update();
    emit onSwitchView3D(b3D);
}

void TFLView::setGlueReady()
{
    _glueReady = true;
}

void TFLView::setReady(bool bReady)
{
    _bIsReady = bReady;
    if( bReady)
        updateViewBox(true);
}

void TFLView::setUserInfo(QByteArray info)
{
    _userInfo = info;
}

void TFLView::setModel(std::shared_ptr<WorldModel> model)
{
    _model = model;
}

void TFLView::setOSMData(std::shared_ptr<OSMData> data)
{
    _osmRenderer.setOSMData(data);
}

void TFLView::paintRings(QPainter &p)
{
    GPSLocation center = _myLocation;
    QPoint myLocPt = toScreen(center);

    if( getShowProximityRings())
    {
        p.setPen(Qt::red);
        int pixDist = getProximityDistance() * getPixelLevel();
        p.drawEllipse(myLocPt, pixDist, pixDist );
    }
    else
    {
        const int sz = 2;
        p.setBrush(Qt::yellow);
        p.setPen(Qt::yellow);
        p.drawEllipse(myLocPt, sz, sz);
        p.setBrush(Qt::NoBrush);
    }
}

void TFLView::paintMiddle(QPainter &p)
{
    const QSize sz = middleOfScreen();

    if( _myLocation == GPSLocation())
    {
        p.drawPoint(sz.width(), sz.height());
        p.fillRect(sz.width()-2, sz.height()-2, 4, 4, Qt::red);
    }
    else
    {
        //Paint Center yellow cross
        QPen pen(isMapNight()? Qt::yellow: Qt::black);
        p.setPen(pen);
        const int length= 4;
        p.drawLine(sz.width()-length, sz.height(), sz.width()+length, sz.height());
        p.drawLine(sz.width(), sz.height()-length, sz.width(), sz.height()+length);
    }
}

void TFLView::render()
{
    sync();

    if( _bBlockRendering)
        return;

    QPainter painter(m_device.get());

    if( !_glueReady)
    {
        paintScreenMessage(painter, "Initialising, Please Wait.....");
        return;
    }

    float dt = 0.0f;

    if( !_elapsedDisplayTimer.isValid() )
    {
        _elapsedDisplayTimer.start();
        dt = 0.0f;
    }
    else
    {
        dt = _elapsedDisplayTimer.restart() / 1000.0f;
    }

    painter.beginNativePainting();
    paintNative(dt);
    painter.endNativePainting();

    paint(painter);
}

void TFLView::paint(QPainter &p)
{
    if( !isView3D())
        paintTFL(p);

    if( _topHeightWanted > 0)
        _flatButtonManager.paint(p, true, true);

    paintPerimeterHdgs(p);
    paintHttpInfo(p);
    paintUserUI(p);

    _mouseAreaManager.paint(p);

    if( _proximityActive && !isView3D())
        paintProximity(p);

    paintInAppMsgs(p);

    if( isRealTimeGPS())
        paintRealTimeGPSInfo(p);

    paintThumbnailInfo(p);

    if( (isView3D() && _3d->isReady()) || (!isView3D() && isReady()))
        paintRadarBottomMessage(p);
    else
    {
        paintScreenMessage(p, "Loading, Please Wait.....");
        paintLoadingInfo(p);
    }

#ifdef Q_OS_WIN

    QFont fn;
    fn.setFamily("Verdana");
    fn.setPixelSize(12);
    p.setFont(fn);
    p.setPen(Qt::black);
    p.setBackgroundMode(Qt::OpaqueMode);
    p.setBackground(Qt::white);
    p.drawText(0,size().height()-5, QString("Pix/M:%1, Counts %2")
                                           .arg(getPixelLevel(), 0, 'f', 0)
                                           .arg(_osmRenderer.debugCount()));
#endif
}

bool TFLView::hasGPSTimedOut() const
{
    const int ms = _realGPSInfo.lastUpdate.msecsTo(_lastUpdateTime);
    const bool bRecent = _realGPSInfo.lastUpdate.isValid() && ms < _updateGPSInterval + 1000;
    return !bRecent;
}

std::shared_ptr<WorldModel> TFLView::getWorldModel()
{
    return _model;
}

void TFLView::paintPausedInfo(QPainter &p)
{
    p.setPen(QColor(255, 255, 0));
    QString msg("PAUSED (press space)");
    QFont font;
    font.setFamily("Tahoma");
    font.setPointSize(18);

    QFontMetrics fm(font);
    int width = fm.horizontalAdvance(msg);

    QRect rc = geometry();
    p.setFont(font);

    p.setBackground(Qt::blue);
    p.setBackgroundMode(Qt::OpaqueMode);
    p.drawText(QPoint(rc.width()/2 - width/2, rc.height()), msg);
    font.setPointSize(8);
    p.setFont(font);
    p.setBackgroundMode(Qt::TransparentMode);
}

const QString& TFLView::getDataSourceName() const
{
    static QString source("Powered by TfL Open Data/Network Rail");
    return source;
}

void TFLView::setRealTimeMode(RealTimeMode mode)
{
    _bRealTimeMode = mode;
}

void TFLView::setGPSUpdateInterval(int interval)
{
    _updateGPSInterval = interval;
}

bool TFLView::isBlockRendering() const
{
    return _bBlockRendering;
}

void TFLView::setBlockRendering(bool block)
{
    _bBlockRendering = block;

    if( block)
    {
        _dirtyFlagTimer->stop();
    }
    else
    {
        _dirtyFlagTimer->start();
        update();
    }
}

void TFLView::setShowLondonMetro(bool show)
{
    _bShowLondonMetro = show;
    update();
}

bool TFLView::getShowLondonMetro() const
{
    return _bShowLondonMetro;
}

void TFLView::setBusStopVisible(bool show)
{
    _lineRenderer.setBusStopVisible(show);
    _lineRenderer.updateCache(createToScreenSnapshot());
}

bool TFLView::isBusStopVisible() const
{
    return _lineRenderer.isBusStopVisible();
}

void TFLView::setBusLinesVisible(bool show)
{
    _lineRenderer.setBusLinesVisible(show);
    _lineRenderer.updateCache(createToScreenSnapshot());
}

bool TFLView::isBusLinesVisible() const
{
    return _lineRenderer.isBusLinesVisible();
}

void TFLView::setTubeLinesVisible(bool show)
{
    _lineRenderer.setTubeLinesVisible(show);
    _lineRenderer.updateCache(createToScreenSnapshot());
}

bool TFLView::isTubeLinesVisible() const
{
    return _lineRenderer.isTubeLinesVisible();
}

void TFLView::setBusBlipVerbosity(BlipVerbosity v)
{
    _busBlipVerbosity = v;
}

void TFLView::setTrainBlipVerbosity(BlipVerbosity v)
{
    _trainBlipVerbosity = v;
}

void TFLView::setProximityDistance(float nm)
{
    _proximityDist = nm;
}

void TFLView::setShowProximityRings(bool b)
{
    _bShowProximityRings = b;
    update();
}

void TFLView::setProximityActive(bool bActive, bool bSound)
{
    _proximityActive = bActive;
    _proximityActiveSound = bSound;

    updateFlatButtonVisibility();
    update();
}

float TFLView::getProximityDistance() const
{
    return _proximityDist;
}

bool TFLView::getProximityActive() const
{
    return _proximityActive;
}

bool TFLView::getShowProximityRings() const
{
    return _bShowProximityRings;
}

bool TFLView::isProximityWarningInProgress() const
{
    return _proximityWarningInProgress;
}

bool TFLView::isProximityWarning(const std::shared_ptr<const Vehicle> &vehicle) const
{
    if( !getProximityActive())
        return false;

    float dist = vehicle->getPos().distanceTo(_myLocation);

    return dist < Units::NmToMeters(_proximityDist);
}

void TFLView::paintHttpInfo(QPainter &p)
{
    QPen pen;
    pen.setColor(Qt::white);
    p.setPen(pen);
    QFont font;
    font.setFamily("Tahoma");
    font.setPointSize(12);
    p.setFont(font);
    p.setBackground(Qt::blue);
    p.setBackgroundMode(Qt::OpaqueMode);

    QFontMetrics fm(font);
    QString strMsg;
    int iWidth(0);
    const int border = 5;
    int iHeight = fm.height();

    _resizeWanted = _topHeightWanted == 0 || _bottomHeightWanted == 0;
    _topHeightWanted = iHeight + border;
    _bottomHeightWanted = iHeight + border;

    const QRect& dims = rect();

    if(_showTopInfo && _bIsFullScreen)
    {
        QTime time = QTime::currentTime();
        strMsg = QString("%1 - %2%%3").arg(time.toString("HH:mm"))
                     .arg(_batteryCharge, _batteryStatus ? _strUp: _strDown);

        iWidth = fm.horizontalAdvance(strMsg);
        p.drawText(0, iHeight, strMsg);
    }

    strMsg = QString("Center: [%1, %2]").
            arg(_gpsOrigin._lat).
            arg(_gpsOrigin._lng);
    iWidth = fm.horizontalAdvance(strMsg);
    p.drawText(dims.right() - iWidth, dims.height() - border, strMsg);

    if(_showTopInfo)
        paintServerInfo(p, dims.right(), iHeight);

#ifdef Q_OS_WIN32__
    {
        p.setFont(QFont("verdana", 8));
        fm = p.fontMetrics();

        QString text;
        text += "Version[";
        text += (char*)glGetString(GL_VERSION);
        text += "]";
        p.drawText(10, fm.height()*3, text);

        text = "Vendor[";
        text += (char*)glGetString(GL_VENDOR);
        text += "]";
        p.drawText(10, fm.height()*4, text);

        text = "Renderer[";
        text += (char*)glGetString(GL_RENDERER);
        text += "]";
        p.drawText(10, fm.height()*5, text);
    }
#endif

    p.setBackgroundMode(Qt::TransparentMode);
}

void TFLView::paintServerInfo(QPainter &p, int x, int y)
{
    QFontMetrics fm = p.fontMetrics();
    p.setBackgroundMode(Qt::OpaqueMode);
    p.setBackground( _lastConnectionError.isEmpty() ?Qt::darkCyan : Qt::darkRed);

    if( !_flatButtonManager.getImgVisibility(_flatSubscription))
    {
        QString strMsg = QString("%1:%2").
                arg(getDataSourceName()).
                arg(getWorldModel()->vehicleCount());

        p.drawText(x-fm.horizontalAdvance(strMsg), y, strMsg);
    }

    QString status;

    if( !_lastConnectionError.isEmpty())
        status = _lastConnectionError;

    else if( isView3D() && _3d->getTileMapActive())
        status = _3d->getTileMapHost();

    if( !status.isEmpty())
    {
        QTextOption option;
        option.setAlignment(Qt::AlignRight|Qt::AlignTop);
        QRect rc(QPoint(0,y), QPoint(width(), height()/2));
        rc.adjust(7,7,-7,-7);
        p.drawText(rc, status, option);
    }

    p.setBackgroundMode(Qt::TransparentMode);
}

std::shared_ptr<View3D> TFLView::get3D()
{
    return _3d;
}

void TFLView::paintUserUI(QPainter &p)
{
    if( isView3D() && _bMouseCapture)
    {
        const QSize& sz = size();
        p.save();
        QPen pen;
        pen.setWidth(2);

        if( _mouseAreaManager.isMouseActive() || _bMouseDown)
            pen.setColor(QColor(255,255,255,100));
        else
            pen.setColor(Qt::white);

        p.setPen(pen);

        //Zoom markings
        {
            const int len = 50;
            const int arrlen = 15;
            QPoint center(sz.width()/2, sz.height()/2);

            QVector<QPoint> pts;

            pts << QPoint(center.x(), center.y());
            pts << QPoint(center.x()+len, center.y()-len);
            pts << QPoint(center.x()+len, center.y()-len);
            pts << QPoint(center.x()+len-arrlen, center.y()-len);
            pts << QPoint(center.x()+len, center.y()-len);
            pts << QPoint(center.x()+len, center.y()-len+arrlen);

            pts << QPoint(center.x(), center.y() );
            pts << QPoint(center.x()-len, center.y()+len);
            pts << QPoint(center.x()-len, center.y()+len);
            pts << QPoint(center.x()-len+arrlen, center.y()+len);
            pts << QPoint(center.x()-len, center.y()+len );
            pts << QPoint(center.x()-len, center.y()+len-arrlen);

            p.drawLines(pts);

            QPoint offset(3*len/4, -3*len/4);
            p.drawEllipse(center + offset, 3,3);
            p.drawEllipse(center - offset, 3,3);
        }

        if( _mouseAreaManager.isMouseActive() || !_bMouseDown)
            pen.setColor(QColor(255,255,255,100));
        else
            pen.setColor(Qt::white);

        p.setPen(pen);

        //Translation markings
        {
            const int len = 25;
            const int arrlen = 6;

            pen.setWidth(2);
            p.setPen(pen);

            QPoint center(sz.width()/2, 3*sz.height()/4);

            QVector<QPoint> pts;

            pts << QPoint(center.x(), center.y()-len);
            pts << QPoint(center.x(), center.y()+len);
            pts << QPoint(center.x()-len, center.y());
            pts << QPoint(center.x()+len, center.y());

            pts << QPoint(center.x()-len, center.y() );
            pts << QPoint(center.x()-len+arrlen, center.y()-arrlen);
            pts << QPoint(center.x()-len, center.y() );
            pts << QPoint(center.x()-len+arrlen, center.y()+arrlen);

            pts << QPoint(center.x()+len, center.y() );
            pts << QPoint(center.x()+len-arrlen, center.y()-arrlen);
            pts << QPoint(center.x()+len, center.y() );
            pts << QPoint(center.x()+len-arrlen, center.y()+arrlen);

            pts << QPoint(center.x(), center.y()-len );
            pts << QPoint(center.x()-arrlen, center.y()-len +arrlen );
            pts << QPoint(center.x(), center.y()-len );
            pts << QPoint(center.x()+arrlen, center.y()-len +arrlen );

            pts << QPoint(center.x(), center.y()+len);
            pts << QPoint(center.x()-arrlen, center.y()+len -arrlen );
            pts << QPoint(center.x(), center.y()+len );
            pts << QPoint(center.x()+arrlen, center.y()+len -arrlen );

            p.drawLines(pts);
        }

        p.restore();
    }

    _mouseAreaManager.paint(p);

    if(!isReady())
        paintLoadingInfo(p);
    else
        paintRadarBottomMessage(p);
}

void TFLView::paintThumbnailInfo(QPainter& p )
{
    const std::shared_ptr<Vehicle>& blipSelected = getWorldModel()->getSelectedVehicle(0);
    if( blipSelected == nullptr)
        return;

    if( !isView3D() && !ptInScreen(blipSelected->getPos()))
        return;

    const QSize& sze = size();

    p.setPen(Qt::white);
    QFont font;
    font.setFamily("Arial");
    font.setPointSize(12);
    p.setFont(font);

    QStringList lines;

    auto addIfBlank = [&lines](const QString & title, const QString& text) {
        if( !text.isEmpty())
            lines << (title  + text);
    };

    addIfBlank("Source:", blipSelected->getDataSource());
    addIfBlank("VehicleID:", blipSelected->_vehicleId);
    addIfBlank("Positioning:", blipSelected->_useVehicleBehaviour ? QStringLiteral("Location"):QStringLiteral("Ref Stn"));
    addIfBlank("Name:", blipSelected->_line->name());
    addIfBlank("NaptanID:", blipSelected->_naptanId);
    addIfBlank("Towards:", blipSelected->_towards);
    addIfBlank("Orign:", blipSelected->_originName);
    addIfBlank("Destn:", blipSelected->_destinationName);
    addIfBlank("Location:", blipSelected->_currentLocation);
    addIfBlank("Ref Stn:", blipSelected->_displayStationName + " [" + QString::number(blipSelected->_timeToStation) + "secs]");
    addIfBlank("Platform:", blipSelected->_platform);
    addIfBlank("Previous:", blipSelected->_previousLocation);
    addIfBlank("Next:", blipSelected->_nextLocation);
#ifdef Q_OS_WIN
    addIfBlank("Behaviour:[",blipSelected->currentBehaviour()+"]");
    addIfBlank("Want Cir:[", QString(blipSelected->_wantToBeCircle?"true":"false") +"]");

    addIfBlank("Key:[", blipSelected->_key +"]");
    addIfBlank("BranchID:", "[" +QString::number(blipSelected->_attachment.branch->getId()) + "]");
    addIfBlank("Dir:[",blipSelected->_direction +"]");
#endif

    if( _myLocation != GPSLocation())
    {
        float distFromMyLocation = _units.getDistance(blipSelected->getPos().distanceTo(_myLocation)/1609.3f);
        float bearing = blipSelected->getPos().bearingFrom(_myLocation);
        lines << QString("Dist: %1%2").
                 arg(distFromMyLocation, 3, 'f', 1).
                 arg(_units.getDistName());

        lines << QString("Brg: %1°").arg(bearing, 3, 'f', 0, QChar('0'));
    }

    QFontMetrics fm = p.fontMetrics();
    const int fontHeight = fm.height();

    QSize infoSize;
    infoSize.setHeight(fm.height()*lines.size());

    infoSize.setWidth(infoSize.width());

    for(const QString&str: std::as_const(lines))
        infoSize.setWidth(qMax(infoSize.width(), fm.horizontalAdvance(str)));

    const QPoint border(45, 30);
    const int iSep = 2;
    QPoint pt(sze.width()-infoSize.width()-border.x(), sze.height()-border.y());

    if( pt.x() < border.x())
        pt.setX(border.x());

    QColor c = Qt::blue;
    c.setAlpha(200);

    p.fillRect( pt.x(), pt.y()-infoSize.height()-iSep, infoSize.width(), infoSize.height()+iSep, c );

    int iLineNo = lines.size();
    for(QString&str: lines)
        p.drawText(pt.x(), pt.y()- fontHeight * --iLineNo-iSep, str);
}

void TFLView::paintProximity(QPainter &p)
{
    const QSize& sz = size();

    QFont f;
    f.setPointSize(10);
    p.setFont(f);
    p.setPen(Qt::white);
    p.setBackgroundMode(Qt::OpaqueMode);
    p.setBackground(_proximityWarningInProgress ? Qt::red:Qt::darkCyan);

    QFontMetrics fm = p.fontMetrics();

    const int b = (_flatButtonManager.getVisibility()?50:5)+_bottomHeightWanted;

    p.drawText(QPoint(15, sz.height()-3*fm.height()-b), "Proximity:");

    QString str = QString("dist:%1%2").arg(_units.getDistance(_proximityDist),0,'f', 2).arg(_units.getDistName());
    p.drawText(QPoint(15, sz.height()-2*fm.height()-b), str);

    p.setBackgroundMode(Qt::TransparentMode);
}

void TFLView::paintInAppMsgs(QPainter &p)
{
    QFont font;
    font.setFamily("Tahoma");
    font.setPointSize(11);
    p.setFont(font);

    const int height = p.fontMetrics().height();
    int pos = _topHeightWanted+5+height;

    auto displayNextLine = [&height, &p, &pos](const QString& str)
    {
        p.drawText(QPoint(15,pos+=height), str);
    };

    Qt::BGMode bgmode = p.backgroundMode();

    p.setBackgroundMode(Qt::OpaqueMode);

    QColor c;

    c = Qt::yellow;
    p.setBackground(c);
    p.setPen(Qt::black);

    for(const auto& str  : _inAppEvalMsgs )
        displayNextLine(str);

    if( !_inAppEvalMsgs.empty())
        displayNextLine(QLatin1String(""));

    p.setBackgroundMode(bgmode);
}

void TFLView::paintRealTimeGPSInfo(QPainter &p)
{
    QFont f;
    f.setPointSize(20);
    p.setFont(f);

    QFontMetrics fm = p.fontMetrics();

    p.setBackgroundMode(Qt::OpaqueMode);

    if( hasGPSTimedOut() )
        p.setBackground(Qt::darkRed);
    else
        p.setBackground(Qt::darkCyan);

    p.setPen(Qt::white);

    QString str;

    const int fHeight = fm.height();

    str = QString("Spd:%1 %2").arg( std::abs(int( _units.getSpeed(_realGPSInfo.spd))), 3, 10, QChar('0')).arg(_units.getSpeedName());
    p.drawText(15, height() - _bottomHeightWanted - fHeight, str );

#ifdef WANT_VSI
    if( _units.getAltitudeType() == Units::Altitude::Feet || _units.getAltitudeType() == Units::Altitude::Meters )
        str = QString("Alt:%1 %2").arg( int( _units.getAltitude(_realGPSInfo.alt)), 5, 10, QChar('0')).arg(_units.getAltitudeName());
    else
        str = QString("Alt:%1 %2").arg( GetRadarAltText( _realGPSInfo.alt) ).arg(_units.getAltitudeName());

    float fVsi = _units.getAltitude(std::abs(_realGPSInfo.vsi));
    fVsi = _units.getInterval(fVsi);

    QString vsiStr;

    auto type = _units.getAltitudeType();

    if( type == Units::Altitude::Feet || type == Units::Altitude::Meters)
        vsiStr = QString("%1").arg(int(fVsi));
    else
        vsiStr = QString("%1").arg(fVsi, 0, 'f', 2, 0);

    QString str2 = QString("Vsi:%2%3 %4%5")
            .arg(_realGPSInfo.vsi > 0 ? _strUp : _strDown)
            .arg(vsiStr)
            .arg(_units.getAltitudeName())
            .arg(_units.getVsiIntervalName());

    const int xPos = size().width()-qMax( fm.width(str),fm.width(str2)) - 10;

    p.drawText(xPos, yPos + fHeight, str);
    p.drawText(xPos, yPos + fHeight*2, str2);
#endif

    p.setBackgroundMode(Qt::TransparentMode);
}

void TFLView::setInAppEvalMsgs(const std::vector<QString> &evals)
{
    _inAppEvalMsgs = evals;
}

QString TFLView::GetRadarAltText(float ft, int dp)
{
    float fAlt = _units.getAltitude(ft);
    if( _units.getAltitudeType() == Units::Altitude::Feet || _units.getAltitudeType() == Units::Altitude::Meters)
        return QString("%1").arg(int(fAlt));
    else
        return QString("%1").arg(fAlt, 0, 'f', dp, QChar('0'));
}

void TFLView::paintRadarBottomMessage(QPainter & p)
{
    if( _displayMessageAtBottom.length() == 0)
        return;

    QFont font;
    font.setFamily("Tahoma");
    font.setPointSize(14);

    p.setFont(font);
    p.setPen(Qt::yellow);

    p.setBackground(QColor(0,0,255,128));
    p.setBackgroundMode(Qt::OpaqueMode);
    QRect rc = geometry();

    rc.adjust(0, _topHeightWanted, 0, -_bottomHeightWanted);

    QTextOption to;
    to.setWrapMode(QTextOption::WordWrap);
    to.setAlignment(Qt::AlignHCenter | Qt::AlignBottom);

    p.drawText(rc, _displayMessageAtBottom, to);
    p.setBackgroundMode(Qt::TransparentMode);
}

void TFLView::updateTime()
{
    return _model->update();
}

void TFLView::setGPSLimitBoundary(std::pair<GPSLocation, GPSLocation> b, bool active)
{
    _gpsLimitBoundary = b;
    _gpsLimitBoundaryActive = active;
}

GPSLocation TFLView::myLocation() const
{
    return _myLocation;
}

GPSLocation TFLView::gpsOrigin() const
{
    return _gpsOrigin;
}

float TFLView::getCompassValue() const
{
    return _camera._heading;
}

bool TFLView::isRealTimeGPS() const
{
    return _bRealTimeMode == RealTimeMode::GPS;
}

bool TFLView::isRealTimeCompass() const
{
    return _bRealTimeMode == RealTimeMode::Compass;
}

void TFLView::updateCache()
{
    if( !_glueReady)
        return;

    if( !_bResourcesLoaded)
        return;

    if( isView3D())
        return;

    _lineRenderer.updateCache(createToScreenSnapshot());
    _vehicleRenderer.updateCache(createToScreenSnapshot());
    updateOSM();
    _tileImages.clear();
}

void TFLView::setRenderingIsMultithreaded(bool b)
{
    _isMultiThreaded = b;
}

void TFLView::callBack(TFLViewCallback viewBack)
{
    viewBack(this);
}

void TFLView::updateOSM()
{
    // _osmRenderer.setViewBoundary(this->getBoundaryView(false));
    _osmRenderer.setDirty();

    setTilesDirty();
}

void TFLView::startMapRendering()
{
    ViewState snapshot = createToScreenSnapshot();
    QByteArray hash = generateSnapshotHash(snapshot);
    emit requestMapRender(snapshot, hash);
}

void TFLView::hdgAreaPaintHandler(QPainter &p, bool isMouseDown, const QRect &rc)
{
    if( isView3D())
    {
        const QSize& sz = size();
        p.setPen(Qt::black);
        p.setBrush(Qt::lightGray);
        const int radius = 25;
        QRect rcHdg(sz.width()/2-radius/2, sz.height()-radius*2, radius, radius);
        p.drawEllipse(rcHdg);
        p.setBrush(Qt::darkGray);
        p.drawPie(rcHdg, (-_camera._heading+45)*16, 16*90);
        p.setBrush(Qt::NoBrush);
    }

    if(!isMouseDown && !_bMouseCapture)
        return;

    p.save();
    QPen pen;
    QColor c = QColor(10,10,10, isMouseDown?20:5);
    p.fillRect(rc, c);
    pen.setColor(isMouseDown?Qt::white:QColor(255,255,255,100));
    pen.setWidth(2);
    p.setPen(pen);

    int yMid = rc.y() + rc.height()/2;
    int xMid = rc.x() + rc.width()/2;
    const int arrlen = 10;

    QVector<QPoint> pts;

    QPoint rPt(xMid + 10 + rc.width()/4, yMid );

    pts << QPoint(xMid+10, yMid);
    pts << rPt;

    pts << rPt;
    pts << QPoint(rPt.x() -arrlen, rPt.y()-arrlen );

    pts << rPt;
    pts << QPoint(rPt.x() -arrlen, rPt.y()+arrlen);

    QPoint lPt(xMid - 10 - rc.width()/4, yMid);

    pts << QPoint(xMid-10, yMid);
    pts << lPt;

    pts << lPt;
    pts << QPoint(lPt.x()+arrlen, lPt.y()-arrlen);

    pts << lPt;
    pts << QPoint(lPt.x()+arrlen, lPt.y()+arrlen);

    p.drawLines(pts);

    p.restore();
}

void TFLView::altAreaPaintHandler(QPainter &p, bool isMouseDown, const QRect &rc)
{
    QFont f;
    f.setPointSize(12);
    p.setFont(f);
    p.setPen(Qt::white);
    p.setBackgroundMode(Qt::OpaqueMode);
    p.setBackground(Qt::darkCyan);

    if( !isRealTimeGPS())
    {
        QFontMetrics fm = p.fontMetrics();
        QString str = QString("Alt:%1%2").arg(GetRadarAltText(Units::MetersToFeet(_gpsOrigin._height))).arg(_units.getAltitudeName());
        p.drawText(rc.x()+rc.width()-fm.horizontalAdvance(str)-10,rc.y()+rc.height()*3.0f/4-2*fm.height(), str);
    }
    p.setBackgroundMode(Qt::TransparentMode);

    if(!isMouseDown && !_bMouseCapture)
        return;

    vertMouseAreaPainter(p, rc, 10, isMouseDown);
}

void TFLView::pitchAreaPaintHandler(QPainter &p, bool isMouseDown, const QRect &rc)
{
    QFont f;
    f.setPointSize(12);
    p.setFont(f);
    p.setPen(Qt::white);
    p.setBackgroundMode(Qt::OpaqueMode);
    p.setBackground(Qt::darkCyan);

    const int fHeight = p.fontMetrics().height();
    const float pitch = _camera._pitch;

    QString str = QString("Pitch:%1%2°").arg(pitch<0?_strUp:_strDown).arg( abs( int(pitch)));
    p.drawText(15, rc.y()+rc.height()*3.0f/4-2*fHeight, str);

    str = QString("Zoom:%1").arg(_3dZoomFactor, 0, 'f', 1, QChar('0'));
    p.drawText(15, rc.y()+rc.height()*3.0f/4-fHeight , str);

    p.setBackgroundMode(Qt::TransparentMode);

    p.setPen(Qt::white);
    p.setBrush(Qt::lightGray);
    const int radius = 25;
    QRect rcPitch(radius+radius/2, rc.y()+rc.height()*3.0f/4, radius, radius);
    p.drawEllipse(rcPitch);
    p.setBrush(Qt::darkGray);
    p.drawPie(rcPitch, (-_camera._pitch-45)*16, 16*90);
    p.setBrush(Qt::NoBrush);

    if(!isMouseDown && !_bMouseCapture)
        return;

    vertMouseAreaPainter(p, rc, 10, isMouseDown);
}

void TFLView::vertMouseAreaPainter(QPainter &p, QRect rc, int arrlen, bool isMouseDown)
{
    p.save();
    QPen pen;
    QColor c = QColor(10,10,10, isMouseDown?20:5);
    p.fillRect(rc, c);
    pen.setColor(isMouseDown?Qt::white:QColor(255,255,255,100));
    pen.setWidth(2);
    p.setPen(pen);

    int yMid = rc.y() + rc.height()/2;
    int xMid = rc.x() + rc.width()/2;

    QVector<QPoint> pts;

    QPoint tPt(xMid, yMid+ 10 + 3*rc.height()/8 );

    pts << QPoint(tPt.x(), yMid+10);
    pts << tPt;

    pts << tPt;
    pts << QPoint(tPt.x() -arrlen, tPt.y()-arrlen );

    pts << tPt;
    pts << QPoint(tPt.x() +arrlen, tPt.y()-arrlen);

    QPoint bPt(xMid, yMid - 10 - 3*rc.height()/8);

    pts << QPoint(xMid, yMid-10);
    pts << bPt;

    pts << bPt;
    pts << QPoint(bPt.x()-arrlen, bPt.y()+arrlen);

    pts << bPt;
    pts << QPoint(bPt.x()+arrlen, bPt.y()+arrlen);

    p.drawLines(pts);
    p.restore();
}

void TFLView::mapImageRendered(const QImage image, const QByteArray hash)
{
    int i;
    i = 3;
    //currentImage = image;
    //isRendering = false;
    //update(); // Trigger repaint
}

void TFLView::paintTFL(QPainter& p)
{
    QTransform oldTransform = p.transform();

    const int padding = 200;
    ViewState viewState = createToScreenSnapshot();
    const bool noFingerAction = !_bMouseDown && !_bInPinch;

    if( noFingerAction)
    {
        _gpsClickLocation = _gpsOrigin;
    }

    _rebuildOsmImg |= _OSMImg.isNull();

    if( _rebuildOsmImg && noFingerAction)
    {
        _rebuildOsmImg = false;

        // Calculate the scaled dimensions based on the device pixel ratio
        const qreal scaledWidth = (width() + padding*2) * _devicePixelRatio;
        const qreal scaledHeight = (height() + padding*2) * _devicePixelRatio;
        const QSize scaledSize(scaledWidth, scaledHeight);

        if( _OSMImg.isNull() || _OSMImg.width() != scaledSize.width() || _OSMImg.height() != scaledSize.height())
        {
            // Create the QImage with scaled dimensions
            _OSMImg = QImage(scaledSize, QImage::Format_ARGB32);
            _VehicleImg = QImage(scaledSize, QImage::Format_ARGB32);
        }

        QPainter pLocal(&_OSMImg);

        QColor colGrnd;
        if (isMapNight())
            colGrnd = QColor(0.1f * 255, 0.1f * 255, 0.1f * 255);
        else
            colGrnd = QColor(0.85f * 255, 0.85f * 255, 0.85f * 255);

        pLocal.fillRect(0, 0, _OSMImg.width(), _OSMImg.height(), colGrnd);

        // Scale the QPainter's coordinate system
        pLocal.scale(_devicePixelRatio, _devicePixelRatio);

        // Set render hints and transformation
        pLocal.setRenderHint(QPainter::SmoothPixmapTransform);
        pLocal.setRenderHint(QPainter::Antialiasing);

        pLocal.translate(padding, padding);

        //paintRawOSMTiles(pLocal, scaledSize, colGrnd);

        _osmRenderer.paint(viewState, pLocal);
        _osmRenderer.paintText(viewState, pLocal);

        _lineRenderer.paint(viewState, pLocal);
        _lineRenderer.paintText(viewState, pLocal);
    }

    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    QSize sz = middleOfScreen();

    QPoint diff = toScreen(_gpsClickLocation) - QPoint(sz.width(), sz.height());

    applyCompassTransform(p, true);

    p.translate(diff);

    p.scale(1/_devicePixelRatio,1/_devicePixelRatio);
    p.drawImage(-padding*_devicePixelRatio,-padding*_devicePixelRatio,_OSMImg);

    if( noFingerAction )
    {
        _VehicleImg.fill(Qt::transparent);

        QPainter pLocal(&_VehicleImg);

        pLocal.scale(_devicePixelRatio, _devicePixelRatio);

        // Set render hints and transformation
        pLocal.setRenderHint(QPainter::LosslessImageRendering);

        pLocal.translate(padding, padding);

        _vehicleRenderer.paint(viewState, pLocal);
        _vehicleRenderer.paintText(viewState, pLocal);
    }

    p.drawImage(-padding*_devicePixelRatio,-padding*_devicePixelRatio,_VehicleImg);

    p.setTransform(oldTransform);
    applyCompassTransform(p);

    QPoint gpsPt = toScreen(myLocation());
    p.setPen(Qt::red);

    const int box = getPixelLevel() > 10.0f ? 10 : 5;

    QRect rc = QRect(gpsPt.x()-box/2, gpsPt.y()-box/2, box, box);
    p.setBrush(Qt::yellow);
    p.drawEllipse(rc);
    p.setBrush(Qt::NoBrush);

    paintRings(p);

    if( isRealTimeGPS())
        paintRealGPSHistory(p);

    p.setTransform(oldTransform);

    paintMiddle(p);
}

void TFLView::paintRealGPSHistory(QPainter &p)
{
    const auto& history = _model->_GPSHistory;

    QVector<QPoint> pts;
    pts.resize(history.size());

    std::transform(history.begin(), history.end(), pts.begin(), [this](const GPSLocation& loc) {
        return toScreen(loc);
    });

    p.setPen(Qt::darkGray);

    p.drawPoints(pts);
}

void TFLView::paintTiles(QPainter &p)
{
    if( !_tileManager.isReady())
        return;

    if( _tileImages.empty())
    {
        auto bounds = getBoundaryView();

        _tileManager.setZoomLevel(_PixelsPerMile);
        TileManager::spec specTopLeft = _tileManager.getIndex(bounds.first);
        TileManager::spec specBottomRight = _tileManager.getIndex(bounds.second);

        struct tileRef {
            bool init = false;
            int xIndex;
            int yIndex;
            QPoint pt;
            int width;
            int height;

            QPoint indexOffset(int cellX, int cellY)
            {
                return pt + QPoint(width*cellX, height*cellY);
            }
        } tileRef;

        const float scaleFactorX = _PixelsPerMile / specTopLeft.zoomLevel_X;
        const float scaleFactorY = _PixelsPerMile / specTopLeft.zoomLevel_Y;

        for(int y = specTopLeft.index_y; y <= specBottomRight.index_y; ++y)
        {
            for(int x = specTopLeft.index_x; x <= specBottomRight.index_x; ++x)
            {
                QString strFilename = _tileManager.getFilename(x, y, isMapNight());

                QImage img;

                img.load(strFilename);
                if( img.isNull())
                    continue;

                if( !tileRef.init)
                {
                    GPSLocation centerGPS = _tileManager.getCenterLocation(x, y);
                    tileRef.pt = toScreen(centerGPS);
                    tileRef.xIndex = x;
                    tileRef.yIndex = y;
                    tileRef.init = true;
                }

                tileData td;

                tileRef.width = img.width() * scaleFactorX;
                tileRef.height = img.height() * scaleFactorY;
                td.img= img;

                QPoint pt = tileRef.indexOffset(x-tileRef.xIndex, y-tileRef.yIndex);

                QRect rcTarget;
                rcTarget.setLeft(pt.x() - tileRef.width/2);
                rcTarget.setRight(pt.x() + tileRef.width/2);

                rcTarget.setTop(pt.y() - tileRef.height/2);
                rcTarget.setBottom(pt.y() + tileRef.height/2);
                td.target= rcTarget;

                _tileImages[strFilename] = td;
            }
        }
    }

    for(const auto & item : _tileImages)
    {
        const tileData& td = item.second;
        p.drawImage( td.target, td.img);
    }
}

void TFLView::paintRawOSMTiles(QPainter &pLocal, QSize scaledSize, QColor colGrnd)
{
#ifndef __DIRECT__

    ViewState viewState = createToScreenSnapshot();

    _osmRenderer.paint(viewState, pLocal);
    _osmRenderer.paintText(viewState, pLocal);

#else
    QImage& OSMRawImg = _OSMRawImgCache.img;

    const QString hash = QString::number(_PixelsPerMile)
                         + QString::fromStdString(_gpsOrigin.toString())
                         + QVariant(isMapNight()).toString();

    if( hash != _OSMRawImgCache.id)
    {
        _OSMRawImgCache.id = hash;

        if( OSMRawImg.isNull() || OSMRawImg.width() != scaledSize.width() || OSMRawImg.height() != scaledSize.height())
            OSMRawImg = QImage(scaledSize, QImage::Format_ARGB32);

        {
            QPainter p(&OSMRawImg);
            p.fillRect(0, 0, OSMRawImg.width(), OSMRawImg.height(), colGrnd);
            p.scale(_devicePixelRatio, _devicePixelRatio);
            p.setRenderHint(QPainter::SmoothPixmapTransform);
            p.setRenderHint(QPainter::Antialiasing);

            _osmRenderer.paint(p);
            _osmRenderer.paintText(p);
        }
    }

    pLocal.scale(1/_devicePixelRatio,1/_devicePixelRatio);
    pLocal.drawImage(0,0,OSMRawImg);

    pLocal.scale(_devicePixelRatio, _devicePixelRatio);
#endif
}

void TFLView::paintPerimeterHdgs(QPainter &p)
{
    const QSize &sz = size();
    QTransform previousTransform = p.transform();

    p.translate(0, sz.height()/2);
    p.scale(1, 1-float(_topHeightWanted+_bottomHeightWanted)/sz.height() );
    p.translate(0,-sz.height()/2);

    QPen pen;
    pen.setWidthF(1.5f);
    QColor labelColor(Qt::darkBlue);
    pen.setColor(isMapNight()? Qt::white: Qt::black);
    p.setPen(pen);

    float halfThetaRange = RadiansToDegrees( std::atan2(sz.width(), sz.height()));
    float halfVThetaRange = 90 - halfThetaRange;

    QFont font;
    font.setFamily("Tahoma");
    font.setPixelSize(12);
    p.setFont(font);
    QFontMetrics fm = p.fontMetrics();
    int h = fm.height();
    int w = fm.horizontalAdvance("[N]");
    int middY = sz.height()/2;
    int middX = sz.width()/2;

    QVector<QPointF> pts;
    const int diff = 3;
    const int border = 2;
    const int space = 4;

    float refDegree = 270 + _camera._heading;

    p.setBackground(labelColor);
    p.setBackgroundMode(Qt::OpaqueMode);

    _perimeterHeightWanted = fm.height()*4 + border+ diff;

    if( !isView3D() && _flatButtonManager.getVisibility())
    {
        const QString dirs[]= {"[N]", "NE", "[E]", "SE", "[S]", "SW", "[W]", "NW"};

        p.setPen(Qt::white);
        for( int i=refDegree-halfVThetaRange; i <= refDegree + halfVThetaRange; ++i)
        {
            int y = middY - (i-refDegree) / halfVThetaRange * middY;
            int dx = diff;
            if( i% 5==0)
                dx = diff*2;

            if( i % 45 == 0)
            {
                int ang = i %360;
                p.drawText(dx+space, y+h/2, dirs[ang < 0 ? (ang+360)/45:ang/45]);
                p.drawText(sz.width()-dx-w-border, sz.height()-y+h/2, dirs[((ang+180)%360)/45] );
            }

            if( i% 5 != 0)
                continue;

            pts << QPointF(0, y);
            pts << QPointF(dx, y);

            int iBottomY =  sz.height()-y;
            pts << QPointF(sz.width()-dx, iBottomY);
            pts << QPointF(sz.width(), iBottomY );
        }

        refDegree = _camera._heading;

        for( int i=refDegree-halfThetaRange; i <= refDegree + halfThetaRange; ++i)
        {
            int x = middX + (i-refDegree) / halfThetaRange  * middX;

            int dy = diff;
            if( i % 5==0)
                dy = diff*2;

            if( i % 45 == 0)
            {
                int ang = i %360;
                p.drawText(x-w/2, dy+space+h, dirs[ang < 0 ? (ang+360)/45:ang/45] );
                p.drawText(sz.width()-x-w/2, sz.height()-dy-border-h/2, dirs[((ang+180)%360)/45] );
            }

            if( i% 5 != 0)
                continue;

            pts << QPointF(x, 0);
            pts << QPointF(x, dy);

            pts << QPointF(sz.width()-x, sz.height()-dy);
            pts << QPointF(sz.width()-x, sz.height());
        }

        p.setPen(isMapNight()? Qt::white: Qt::black);
        p.drawLines(pts);
        p.drawRect(rect());
    }

    QString strHdg = QString("%1").arg(MathSupport<int>::normAng(_camera._heading), 3, 10, QChar('0'));

    if(isRealTimeGPS())
    {
        font.setPointSize(18);
        if( hasGPSTimedOut())
            p.setBackground(Qt::darkRed);
        else
            p.setBackground(Qt::darkCyan);
    }
    else if( isRealTimeCompass())
        font.setPointSize(24);
    else
        font.setPixelSize(18);

    p.setFont(font);
    w = p.fontMetrics().horizontalAdvance("000");
    h = p.fontMetrics().height();

    pen.setWidthF(1.0f);
    pen.setColor(Qt::black);
    p.setPen(pen);

    p.fillRect(middX-w/2-border, space+diff+border,w,h, Qt::white );
    p.setPen(Qt::white);
    p.drawText(middX-w/2-border, space+diff+h+border,strHdg);
    p.setTransform(previousTransform);
    p.setBackgroundMode(Qt::TransparentMode);
}

const QImage &TFLView::getTubeStnImage() const
{
    return _tubeStnImg;
}

const QImage &TFLView::getBusStnImage() const
{
    return _busStnImg;
}

const QImage &TFLView::getTramStnImage() const
{
    return _tramStnImg;
}

const QImage &TFLView::getOvergroundStnImage() const
{
    return _overGrndStnImg;
}

const QImage &TFLView::getDLRStnImage() const
{
    return _DLRStnImg;
}

const QImage &TFLView::getElizabethRailStnImage() const
{
    return _TFLRailStnImg;
}

const QImage &TFLView::getNationalRailImage() const
{
    return _NationalRailStnImg;
}

const QImage &TFLView::getRiverBusImage() const
{
    return _RiverBusStnImage;
}

const QImage &TFLView::getIFSCableCarImage() const
{
    return _IFSCableCarImage;
}

void TFLView::saveSettings()
{
    if( !_bSettingsLoaded)
        return;

    _osmRenderer.saveSettings();

    QSettings s;
    s.setValue("gpsOrigin", QString::fromStdString(gpsOrigin().toString()));
    s.setValue("myLocation", QString::fromStdString(myLocation().toString()));
    s.setValue("PixelsPerMile", _PixelsPerMile);
    s.setValue("camera/bank", _camera._bank);
    s.setValue("camera/heading", _camera._heading);
    s.setValue("camera/pitch", _camera._pitch);
    s.setValue("camera/zoom", _3dZoomFactor);
    s.setValue("GPS/MinSpdSensitivity", _gpsMinSpdSensitivity);

    s.setValue("Settings/proximity/Dist", _proximityDist);

    s.setValue("view/is3DView", isView3D());
    s.setValue("view/CompassValue", getCompassValue());
}

void TFLView::loadSettings()
{
    _bSettingsLoaded = true;

    QSettings s;

    setGPSOrigin(GPSLocation(s.value("gpsOrigin", QString::fromStdString(_gpsOrigin.toString())).toString().toStdString()), -1, true);
    _gpsClickLocation = _gpsOrigin;

    setMyLocation(GPSLocation(s.value("myLocation", "").toString().toStdString()));
    _PixelsPerMile = s.value("PixelsPerMile", _PixelsPerMile).toFloat();
    _camera._bank = 0.0f;//s.value("camera/bank", 0.0f).toFloat();
    _camera._heading = s.value("camera/heading", 0.0f).toFloat();
    _camera._pitch = s.value("camera/pitch", 0.0f).toFloat();
    _3dZoomFactor = s.value("camera/zoom", 1.0f).toFloat();
    _gpsMinSpdSensitivity = s.value("GPS/MinSpdSensitivity", _gpsMinSpdSensitivity).toInt();

    _proximityDist = s.value("Settings/proximity/Dist", _proximityDist).toFloat();

    const float compassValue = s.value("view/CompassValue", 0).toFloat();
    const bool is3DView = s.value("view/is3DView", isView3D()).toBool();

    setCompassValue(compassValue);
    setView3D(is3DView);
}

MapRenderer &TFLView::getOSMRenderer()
{
    return _osmRenderer;
}

Units &TFLView::getUnits()
{
    return _units;
}

void TFLView::setMyLocation(const GPSLocation &location)
{
    _myLocation = location;
    _bLookingforGPS = false;
}

void TFLView::setGPSOrigin(const GPSLocation &origin, float pm, bool bInstant, bool bUpdateCache)
{
    _gpsOriginTarget = origin;

    if( _gpsLimitBoundaryActive)
    {
        const GPSLocation& topLeft = _gpsLimitBoundary.first;
        const GPSLocation& bottomRight = _gpsLimitBoundary.second;

        if( _gpsOriginTarget._lat > topLeft._lat)
            _gpsOriginTarget._lat = topLeft._lat;

        if( _gpsOriginTarget._lng < topLeft._lng)
            _gpsOriginTarget._lng = topLeft._lng;

        if( _gpsOriginTarget._lat < bottomRight._lat)
            _gpsOriginTarget._lat = bottomRight._lat;

        if( _gpsOriginTarget._lng > bottomRight._lng)
            _gpsOriginTarget._lng = bottomRight._lng;
    }

    if( _gpsOrigin.distanceTo(_gpsOriginTarget) > Units::NmToMeters(60))
        bInstant = true;

    if (_gpsOriginTarget._lng > 179)
        _gpsOriginTarget._lng -= 360;

    if (_gpsOriginTarget._lng < -179)
        _gpsOriginTarget._lng += 360;

    if(bInstant)
        _gpsOrigin = _gpsOriginTarget;

    if( pm > 0.0f)
        onFinalisePixelPerMile(pm);

    _model->setGPSOrigin(_gpsOrigin);

    updateSelectedVehicleVector();
    updateViewBox(bUpdateCache);
    update();
}

void TFLView::setRealTimeGPSInfo(const RealTimeGPSNewInfo &info)
{
    _realGPSInfo.alt = Units::MetersToFeet(info.alt);
    if( _realGPSInfo.alt < 1.0f)
        _realGPSInfo.alt = 1.0f;

    if( info.hasSpeed)
    {
        _realGPSInfo.nativeSpd = info.spd;
        _realGPSInfo.spd = Units::MetersToKts(info.spd);
    }

    if( info.hasVSI)
        _realGPSInfo.vsi = Units::MetersToFeet(info.vsi) * 60.0f;

    if( info.hasHdg && _realGPSInfo.nativeSpd > _gpsMinSpdSensitivity)
        setCompassValue(info.hdg);

    _realGPSInfo.lastUpdate = info.lastUpdate;

    update();
}

void TFLView::TranslatePosition(QPoint diff)
{
    diff = _compassTransform.map(diff);

    if( isView3D())
        diff *= 4 * (10*_gpsOrigin._height /10000.0+1);
    else
        diff *= 1609.334f / _PixelsPerMile;

    setGPSOrigin( gpsOrigin()+ Vector3F(diff.x(), 0, diff.y()), -1, true);
}

float TFLView::getPixelLevel() const
{
    return _PixelsPerMile;
}

void TFLView::onFinalisePixelPerMile(float fPixelsPerMile)
{
//    auto zL = getZoomLevel();
    _PixelsPerMile = fPixelsPerMile;

    const float iMinLimit = 0.05f;
    const float iMaxLimit = 15600.0f;

    if (_PixelsPerMile < iMinLimit)
        _PixelsPerMile = iMinLimit;

    if (_PixelsPerMile > iMaxLimit)
        _PixelsPerMile = iMaxLimit;

    const float minPix = 0.2f;

    _PixelsPerMile = std::max(_PixelsPerMile,minPix);
}

EulerF TFLView::getCamera() const
{
    return _camera;
}

float TFLView::get3DZoom() const
{
    return _3dZoomFactor;
}

std::pair<GPSLocation, GPSLocation> TFLView::getBoundaryView(bool applyTransform) const
{
    if( isView3D())
        return getRadiusBoundaryView( Units::NmToMeters(10.0f) );

    std::vector<GPSLocation> pts;
    pts.push_back(toGPS(QPoint(), applyTransform));
    pts.push_back(toGPS(QPoint(width(),0), applyTransform));
    pts.push_back(toGPS(QPoint(0, height()), applyTransform));
    pts.push_back(toGPS(QPoint(width(), height()), applyTransform));

    double minLng = 360.0;
    double maxLng = -360.0;
    double minLat = 180.0;
    double maxLat = -180.0;

    for(const GPSLocation&loc: pts)
    {
        minLng = std::min(minLng, loc._lng);
        maxLng = std::max(maxLng, loc._lng);
        minLat = std::min(minLat, loc._lat);
        maxLat = std::max(maxLat, loc._lat);
    }

    return { GPSLocation( maxLat, minLng ), GPSLocation(minLat, maxLng )};
}

std::pair<GPSLocation, GPSLocation> TFLView::getRadiusBoundaryView(float meters) const
{
    GPSLocation topLeft = _gpsOrigin + Vector3F(-meters,0,-meters);
    GPSLocation bottomRight = _gpsOrigin + Vector3F(meters, 0, meters);
    return { topLeft, bottomRight };
}

QString TFLView::getBusyMsg()
{
    return _strBusyMsg;
}

void TFLView::setHttpLastErrMsg(QString errMsg)
{
    _lastConnectionError = errMsg;
}

void TFLView::paintLoadingInfo(QPainter &p)
{
    p.setPen(QColor(0, 255, 255));

    int count = _lastTime.secsTo(QTime::currentTime()) % 10;

    QString msg(getBusyMsg());
    for( int i=0; i < count; i++)
        msg += ".";

    QFont font;
    font.setFamily("Tahoma");
    font.setPointSize(14);

    QFontMetrics fm(font);
    int width = fm.horizontalAdvance(msg);

    p.setBackground(Qt::blue);
    p.setBackgroundMode(Qt::OpaqueMode);

    p.setFont(font);
    p.drawText(0, height()-fm.height(), width, fm.height(), Qt::AlignCenter, msg);
    p.setBackgroundMode(Qt::TransparentMode);
}

void TFLView::paintScreenMessage(QPainter &p, QString msg)
{
    p.setPen(Qt::black);
    QFont font;
    font.setFamily("Verdana");
    font.setPointSize(18);
    p.setFont(font);
    p.setBackground(Qt::yellow);
    p.setBackgroundMode(Qt::OpaqueMode);
    p.drawText(width()/2-p.fontMetrics().horizontalAdvance(msg)/2, height()/2, msg);
    p.setBackgroundMode(Qt::TransparentMode);

}

void TFLView::updateSelectedVehicleVector()
{
    const std::shared_ptr<Vehicle>& blipSelected = getWorldModel()->getSelectedVehicle(0);
    if( blipSelected== nullptr)
        return;

    double dist = blipSelected->getPos().distanceTo(_gpsOrigin);
    double bearing = blipSelected->getPos().bearingTo(_gpsOrigin);
    _blipVector = QVRotate( QHDG(bearing), Vector3F(0,0,-dist));
    _blipVector.y = Units::FtToMeters(0) - _gpsOrigin._height;
}

void TFLView::setTilesDirty()
{
    if( isView3D())
    {
        _3d->setFlat3DSphereDirty();
    }
    else
    {
            _rebuildOsmImg = true;
        //_osmRenderer.setDirty();
    }
}

TFLView::ZoomLevel TFLView::getZoomLevel(float pixm) const
{
    if( pixm > 100.0f)
        return ZoomLevel::VeryHigh;

    if( pixm > 15.0f)
        return ZoomLevel::High;

    if( pixm > 5.0f)
        return ZoomLevel::Medium;

    if( pixm > 1.5f)
        return ZoomLevel::Low;

    return ZoomLevel::VeryLow;
}

TFLView::ZoomLevel TFLView::getZoomLevel() const
{
    return getZoomLevel(_PixelsPerMile);
}

bool TFLView::refresh3DBlipSelected(const QPoint &topLeft, const QPoint &bottomRight)
{
    const std::shared_ptr<Vehicle>& pPreviousBlip = getWorldModel()->getSelectedVehicle(0);

    std::vector<const View3D::Blip3DCache*> blips;

    QRect rc(topLeft, bottomRight);

    const auto& cache = _3d->callSignCache();

    for(const auto& item:cache)
    {
         if(rc.contains(item.pt.x(), item.pt.y()))
             blips.push_back(&item);
    }

    std::sort(blips.begin(), blips.end(), [](const View3D::Blip3DCache* l, const View3D::Blip3DCache* r )
    {
        return l->fDistance < r->fDistance;
    }
    );

    if( !blips.empty())
        getWorldModel()->setSelectedVehicle(blips[0]->key);

    const std::shared_ptr<Vehicle>& pBlip = getWorldModel()->getSelectedVehicle(0);

    if( pPreviousBlip == pBlip)
    {
        getWorldModel()->clearSelectedVehicles();
        return pPreviousBlip != nullptr;
    }

    refreshBlipSelected();
    return true;
}

bool TFLView::refresh2DBlipSelected(GPSLocation topLeft, GPSLocation bottomRight)
{
    const std::shared_ptr<Vehicle>& pPreviousBlip = getWorldModel()->getSelectedVehicle(0);

    getWorldModel()->selectVehicleInArea(topLeft, bottomRight);

#ifdef Q_OS_WIN32
    qDebug() << "Selected : " << getWorldModel()->getSelectedVehiclesCount();
#endif

    const std::shared_ptr<Vehicle>& pBlip = getWorldModel()->getSelectedVehicle(0);

    if( pPreviousBlip == pBlip)
    {
        getWorldModel()->clearSelectedVehicles();
        return pPreviousBlip != nullptr;
    }

    refreshBlipSelected();
    return true;
}

void TFLView::refreshBlipSelected()
{
    updateSelectedVehicleVector();
}

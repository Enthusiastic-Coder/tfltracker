#define NOMINMAX
#include "TFLViewFrameBuffer.h"
#include "TFLView.h"
#include "touchEventdata.h"
#include <QMouseEvent>
#include <QImage>
#include <QUrlQuery>
#include <QMouseEvent>
#include <QTouchEvent>
#include <QPainter>
#include <QSettings>
#include <QNetworkReply>
#include <QVector>
#include <QDateTime>
#include <QQuickWindow>
#include <QThread>
#include <QDebug>
#include <QMutexLocker>

TFLViewFrameBuffer::TFLViewFrameBuffer()
{
    _frameThreadID = QThread::currentThreadId();
    setAcceptTouchEvents(true);
    setAcceptedMouseButtons(Qt::AllButtons);
    setMirrorVertically(true);

    _keepAliveTimer = new QTimer(this);
    connect(_keepAliveTimer, &QTimer::timeout, this, &TFLViewFrameBuffer::update);
    _keepAliveTimer->start(500);
}

TFLViewFrameBuffer::~TFLViewFrameBuffer()
{
#ifdef Q_OS_WIN
    qDebug() << Q_FUNC_INFO;
#endif
}

QQuickFramebufferObject::Renderer *TFLViewFrameBuffer::createRenderer() const
{
    TFLView* renderer = new TFLView(window()->devicePixelRatio());
    const_cast<TFLView*&>(_tflView) = renderer;

    connect( _tflView, &TFLView::aboutToBeDestroyed, this, [this] {

        const_cast<TFLView*&>(_tflView) = nullptr;
    });

    connect( _tflView, &TFLView::showMenuOptions, this, &TFLViewFrameBuffer::showMenuOptions);
    connect( _tflView, &TFLView::wantShowNormal, this, &TFLViewFrameBuffer::wantShowNormal);
    connect( _tflView, &TFLView::wantShowMaximum, this, &TFLViewFrameBuffer::wantShowMaximum);
    connect( _tflView, &TFLView::showQMLSelectStopPointPage, this, &TFLViewFrameBuffer::showQMLSelectStopPointPage);
    connect( _tflView, &TFLView::showQMLPage, this, &TFLViewFrameBuffer::showQMLPage);

    connect( _tflView, &TFLView::wantToGo, this, &TFLViewFrameBuffer::wantToGo);
    connect( _tflView, &TFLView::onWantGPSUpdate, this, &TFLViewFrameBuffer::onWantGPSUpdate);
    connect( _tflView, &TFLView::onWantRealTimeGPS, this, &TFLViewFrameBuffer::onWantRealTimeGPS);
    connect( _tflView, &TFLView::onWantCompassMode, this, &TFLViewFrameBuffer::onWantCompassMode);

    connect( _tflView, &TFLView::wantVR, this, &TFLViewFrameBuffer::wantVR);
    connect( _tflView, &TFLView::wantShow3D, this, &TFLViewFrameBuffer::wantShow3D);
    connect( _tflView, &TFLView::onDisplayInitialised, this, &TFLViewFrameBuffer::onDisplayInitialised);
    connect( _tflView, &TFLView::wantToggleColor, this, &TFLViewFrameBuffer::wantToggleColor);

    connect( _tflView, &TFLView::wantMuteProximity, this, &TFLViewFrameBuffer::wantMuteProximity);
    connect( _tflView, &TFLView::wantToSubscribe, this, &TFLViewFrameBuffer::wantToSubscribe);

    connect( _tflView, &TFLView::onProximityWarningActive, this, &TFLViewFrameBuffer::onProximityWarningActive);

    connect( this, &TFLViewFrameBuffer::postCallBack, _tflView, &TFLView::callBack, Qt::QueuedConnection);

    connect( this, &TFLViewFrameBuffer::activate3DVR, _tflView->get3D().get(), &View3D::triggerVR);

    const bool isThreaded =  _frameThreadID  !=  QThread::currentThreadId();
    renderer->setRenderingIsMultithreaded(isThreaded);

    TFLViewFrameBuffer* frame = const_cast<TFLViewFrameBuffer*>(this);

    QMutexLocker lock(&frame->_mutexReady);
    emit frame->rendererCreated();
    frame->_viewIsReady = true;

    return renderer;
}

void TFLViewFrameBuffer::executeOnRenderThread(TFLViewCallback callback, bool doUpdate)
{
    emit postCallBack(callback);

    if(doUpdate)
        update();
}

void TFLViewFrameBuffer::updateOSM()
{
    executeOnRenderThread([](TFLView* view) {
        view->updateOSM();
    });
}

void TFLViewFrameBuffer::triggerArrivalStatusPage(QString id, QString name, QString stopLetter, QString towards)
{
    executeOnRenderThread([id,name,stopLetter,towards](TFLView* view) {
        view->triggerArrivalStatusPage(id, name, stopLetter, towards);
    });
}

void TFLViewFrameBuffer::triggerArrivalNationalRailStatusPage(QString CRC, QString name)
{
    executeOnRenderThread([CRC,name](TFLView* view) {
        view->triggerArrivalNationalRailStatusPage(CRC, name);
    });
}

void TFLViewFrameBuffer::selectVehicle(QString id)
{
    executeOnRenderThread([id](TFLView* view) {
        view->selectVehicle(id);
    });
}

void TFLViewFrameBuffer::tellFullScreen(bool show)
{
    executeOnRenderThread([show](TFLView* view) {
        view->tellFullScreen(show);
    });
}

TFLView *TFLViewFrameBuffer::getTFLView()
{
    return _tflView;
}

void TFLViewFrameBuffer::hideRadarView(int ms)
{
    QOpenGLContext *context = QOpenGLContext::currentContext();
    qDebug() << "UI Thread OpenGL Context:" << context;


    QQuickWindow* win = window();

    if( win)
    {
        win->hide();
        QTimer::singleShot(ms, [win] {

            win->show();
        });
    }
}

void TFLViewFrameBuffer::persistenceOff()
{
    QQuickWindow* win = window();

    if( win)
    {
        win->setPersistentSceneGraph(false);
        win->setPersistentGraphics(false);
    }
}

void TFLViewFrameBuffer::saveSettings()
{
    executeOnRenderThread([](TFLView* view) {
        view->saveSettings();
    });
}

void TFLViewFrameBuffer::loadSettings()
{
    executeOnRenderThread([](TFLView* view) {
        view->loadSettings();
    });
}

void TFLViewFrameBuffer::mousePressEvent(QMouseEvent *e)
{
    executeOnRenderThread([pos=e->pos()](TFLView* view) {
        view->mousePressHandler(pos);
    });
}

void TFLViewFrameBuffer::mouseMoveEvent(QMouseEvent *e)
{
    executeOnRenderThread([pos=e->pos()](TFLView* view) {
        view->mouseMoveHandler(pos);
    });
    setCursor(Qt::OpenHandCursor);
}

void TFLViewFrameBuffer::mouseReleaseEvent(QMouseEvent *e)
{
    executeOnRenderThread([pos=e->pos()](TFLView* view) {
        view->mouseReleaseHandler(pos);
    });
    unsetCursor();
}

void TFLViewFrameBuffer::wheelEvent(QWheelEvent * e)
{
    executeOnRenderThread([angleDeleta=e->angleDelta()](TFLView* view) {
        view->wheelHandler(angleDeleta);
    });
}

void TFLViewFrameBuffer::touchEvent(QTouchEvent *event)
{
    touchEventData data;
    data.type = event->type();

    switch (event->type())
    {
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
    {
        QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);
        QList<QTouchEvent::TouchPoint> touchPoints = touchEvent->points();
        data.touchPointCounts = touchPoints.count();

        if (touchPoints.count() == 1)
        {
            QPoint pt = touchPoints[0].position().toPoint();
            data.singlePt = pt;
        }
        else if (touchPoints.count() == 2)
        {
            // determine scale factor
            const QTouchEvent::TouchPoint &touchPoint0 = touchPoints.first();
            const QTouchEvent::TouchPoint &touchPoint1 = touchPoints.last();

            QPointF diffPt = touchPoint0.position()- touchPoint1.position();
            QPointF diffPt0 = touchPoint0.pressPosition() - touchPoint1.pressPosition();

            qreal currentScaleFactor =
                QLineF(touchPoint0.position(), touchPoint1.position()).length()
                / QLineF(touchPoint0.pressPosition(), touchPoint1.pressPosition()).length();

            data.diffPt = diffPt;
            data.diffPt0 = diffPt0;
            data.currentScaleFactor = currentScaleFactor;
        }

        executeOnRenderThread([data](TFLView* view) {
            view->touchHandler(data);
        });
        break;
    }
    default:
        QQuickFramebufferObject::touchEvent(event);
    }
}

void TFLViewFrameBuffer::isViewReady(std::function<void (bool)> callback)
{
    QMutexLocker lock(&_mutexReady);
    callback(_viewIsReady);
}

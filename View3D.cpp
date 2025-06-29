#include <jibbs/opengl/OpenGLPipeline.h>
#include <jibbs/mesh/meshloader.h>
#include <jibbs/utilities/stdafx.h>
#include <jibbs/vector/vector4.h>

#define NOMINMAX
#include <QDebug>

#include <algorithm>

#include "View3D.h"
#include "TFLView.h"
#include "WorldModel.h"

View3D* g_view3D = nullptr;

Vector3F jniRotationVector;

#ifdef Q_OS_ANDROID
#include <QJniObject>
#include <QJniEnvironment>

std::deque<std::pair<float,float>> global_VRHdgSample;
std::deque<std::pair<float,float>> global_VROrSample;

void onNativeRotationVector(JNIEnv *env, jobject thiz, jfloat x, jfloat y, jfloat z)
{
    Q_UNUSED(env)
    Q_UNUSED(thiz)

    const float radX = DegreesToRadians(x);
    global_VRHdgSample.push_back({sin(radX), cos(radX)});

    global_VROrSample.push_back({y,z});

    if( global_VRHdgSample.size() > 5)
    {
        global_VRHdgSample.pop_front();
        global_VROrSample.pop_front();
    }

    std::pair<float, float> avg = {};
    for(const auto& value : global_VRHdgSample)
    {
        avg.first += value.first;
        avg.second += value.second;
    }

    avg.first /= global_VRHdgSample.size();
    avg.second /= global_VRHdgSample.size();

    float avgHdg = 90-RadiansToDegrees( std::atan2(avg.second, avg.first));

    std::pair<float, float> avgOr = {};
    for(const auto& value : global_VROrSample)
    {
        avgOr.first += value.first;
        avgOr.second += value.second;
    }

    avgOr.first /= global_VROrSample.size();
    avgOr.second /= global_VROrSample.size();


    jniRotationVector.x = avgHdg;
    jniRotationVector.y = avgOr.first;
    jniRotationVector.z = avgOr.second;

//    QMetaObject::invokeMethod(g_view3D, "onJavaRotationVector",
//                              Qt::QueuedConnection,
//                              Q_ARG(float, x),
//                              Q_ARG(float, y),
//                              Q_ARG(float, z));
//    qDebug() << Q_FUNC_INFO << "-Yaw:" << x << ", Pitch:" << y << ", Roll:" << z;
}

void onNativeVRStarted(JNIEnv *env, jobject thiz, jboolean started)
{
    Q_UNUSED(env)
    Q_UNUSED(thiz)

    QMetaObject::invokeMethod(g_view3D, "onJavaVRStarted",
                              Qt::QueuedConnection,
                              Q_ARG(bool, started));
}


void hookJNI(View3D* view3D)
{
    g_view3D = view3D;

    if( g_view3D == nullptr)
        return;

    JNINativeMethod methods[] = {
        {
            "onNativeRotationVector",
            "(FFF)V",
            reinterpret_cast<void*>(onNativeRotationVector)
        },
        {
            "onNativeVRStarted",
            "(Z)V",
            reinterpret_cast<void*>(onNativeVRStarted)
        }
    };

    QJniObject activity = QNativeInterface::QAndroidApplication::context();

    QJniEnvironment env;

    jclass objectClass = env->GetObjectClass(activity.object<jobject>());
    env->RegisterNatives(objectClass, methods, sizeof(methods) / sizeof(methods[0]));
    env->DeleteLocalRef(objectClass);

    // Checking for errors in the JNI
    if (env->ExceptionCheck()) {
        // Handle exception here.
        env->ExceptionClear();
    }
}
#endif

inline Vector4F from3DToHomogenous(const OpenGLPipeline& pipe, const Vector4F& pt)
{
    return pipe.GetMVP().toFloat() * pt;
}

inline QPointF fromHomogenousToScreen(const QSize& sz, const Vector4F& pt)
{
    Vector4F v = pt;
    v /= v.w;

    v.x *= 0.5f;
    v.x += 0.5f;
    v.y *= 0.5f;
    v.y += 0.5f;
    v.y = 1.0f - v.y;

    v.x *= sz.width();
    v.y *= sz.height();
    return QPointF(v.x, v.y);
}

View3D::View3D(TFLView *v)
    : _view(v)
{
#ifdef Q_OS_ANDROID
    hookJNI(this);
#endif

    struct SkyBoxItem {
        QString name;
        bool isDay;
        QRgb brightness;
    };

    QVector<SkyBoxItem> skyBoxIds = {
                                     {"TropicalSunnyDay", true, qRgb(205,205,205)},
                                     {"CloudyLightRays", true, qRgb(140,140,140) },
                                     {"DarkStormy", false, qRgb(100,100,100) },
                                     {"FullMoon", false, qRgb(65,65,65) },
                                     {"SunSet", false, qRgb(65,65,40)},
                                     {"ThickCloudsWater", true, qRgb(140,140,140)},
                                     };

    for(const auto& item:skyBoxIds)
        _skyBox.addCube(item.name, item.isDay, item.brightness);

    _skyBox.addDome("CloudyDomeDay", ResourcePath("images/skydome.jpg"), true, qRgb(225,225,225));
    _skyBox.addDome("CloudyDomeNight", ResourcePath("images/skydome.jpg"), false, qRgb(85,85,85));

//    connect(&_gyro, &QGyroscope::readingChanged, this, [this]
//    {
//        QGyroscopeReading* gyroReading = _gyro.reading();

//        if( _lastGyroTime == 0)
//        {
//            _lastGyroTime = gyroReading->timestamp();
//            return;
//        }

//        float timeDiff = (gyroReading->timestamp() - _lastGyroTime)/1000000.0f;
//        _lastGyroTime = gyroReading->timestamp();

//        Vector3F angVel(gyroReading->x(), gyroReading->y(), gyroReading->z());

//        angVel *= FLOAT_PI / 180.0f;

//        _qOrientation += _qOrientation * angVel * timeDiff / 2.0f;

//        _qOrientation.Normalize();

//        Vector3F orient = MathSupport<float>::MakeEuler(_qOrientation);
//        //HDG | BANK | PITCH
//        onJavaRotationVector(orient.y, orient.x, orient.z);
//    });

//    connect(&_gyro, &QGyroscope::activeChanged, this, [this]
//    {
//        if( _gyro.isActive())
//            _qOrientation = MathSupport<float>::MakeQHeading(0.0f);
//        else
//            onJavaRotationVector(_v->_camera._heading, _v->_camera._pitch, 0);
//    });
}

View3D::~View3D()
{
#ifdef Q_OS_ANDROID
    hookJNI(nullptr);
#endif
}

bool View3D::isReady() const
{
    return _isReady;
}

void View3D::setQtTexManager(std::shared_ptr<QtTextureManager> manager)
{
    _texManager = manager;
}

void View3D::extractModel(std::unique_ptr<meshLoader> &obj, QString path, callBackMeshLoader impl)
{
    obj.reset(new meshLoader( _texManager, _meshManager));
    QStringList assetFilenames;
    assetFilenames << path + ".obj";
    if( impl)
    {
        impl(obj);
    }
    obj->load(ResourcePath(assetFilenames[0]));
}

void View3D::init()
{
    if( _initDone)
        return;

    _initDone = true;
    updateMinMaxScaleFactors();
    _isReady = false;

    QOpenGLFunctions::initializeOpenGLFunctions();

    _renderer.reset(new Renderer);
    _flat3DSphere.init();
    _flat3DSphere.setTextureMinMag(QOpenGLTexture::Nearest, QOpenGLTexture::Nearest);

    extractModel(_unitBox, "models/unit/unitbox");
    extractModel(_unitSquare, "models/unit/unitsquare");
    extractModel(_unitRing, "models/unit/unitring" );
    extractModel( _unitMiniRing, "models/unit/unitminiring");
    extractModel(_bus, "models/bus/bus");
    extractModel(_train, "models/train/train");

    extractModel(_londonTube, "models/metro/londontube");
    extractModel(_londonDLR, "models/metro/londondlr");
    extractModel(_londonBus, "models/metro/londonbus");

    if( !_texShader.loadSrc(readAllRP("shaders/texShader.vert"), readAllRP("shaders/texShader.frag")))
        qDebug() << QString::fromStdString(_texShader.getError());

    if( !_shaderTex3D.loadSrc(readAllRP("shaders/shaderTex3d.vert"), readAllRP("shaders/shaderTex3d.frag")))
        qDebug() << QString::fromStdString(_shaderTex3D.getError());

    if( !_shaderColTex3d.loadSrc(readAllRP("shaders/shaderColTex3d.vert"), readAllRP("shaders/shaderColTex3d.frag")))
        qDebug() << QString::fromStdString(_shaderColTex3d.getError());

    if( !_flatShader3D.loadSrc(readAllRP("shaders/flatShader3d.vert"),readAllRP("shaders/flatShader3d.frag")))
        qDebug() << QString::fromStdString(_flatShader3D.getError());

    if( !_shader3D.loadSrc(readAllRP("shaders/shader3d.vert"), readAllRP("shaders/shader3d.frag")))
        qDebug() << QString::fromStdString(_shader3D.getError());

    if( !_fontShaderProgram.loadSrc(readAllRP("shaders/fontShader.vert"), readAllRP("shaders/fontShader.frag")))
        qDebug() << QString::fromStdString(_fontShaderProgram.getError());

    if( !_fontAlphaShaderProgram.loadSrc(readAllRP("shaders/fontAlphaShader.vert"), readAllRP("shaders/fontAlphaShader.frag")))
        qDebug() << QString::fromStdString(_fontAlphaShaderProgram.getError());

    if( !_runwayLiteProgram.loadSrc(readAllRP("shaders/runwayLite.vert"), readAllRP("shaders/runwayLite.frag")))
        qDebug() << QString::fromStdString(_runwayLiteProgram.getError());

    if( !_metroProgram.loadSrc(readAllRP("shaders/metroShader.vert"), readAllRP("shaders/metroShader.frag")))
        qDebug() << QString::fromStdString(_metroProgram.getError());

    _skyBox.init();

    _skyBox.selectId(_skyLineDeferredId);

    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    QFont font;
    font.setFamily("Verdana");
    font.setPixelSize(18);

    _myFontTexture.setFont(font);

    _fontRenderer.selectRenderer(_renderer.get());
    _fontRenderer.selectShader(&_fontShaderProgram);
    _fontRenderer.selectFont(&_myFontTexture);

    _alphaFontRenderer.selectRenderer(_renderer.get());
    _alphaFontRenderer.selectShader(&_fontAlphaShaderProgram);
    _alphaFontRenderer.selectFont(&_myFontTexture);

    _labelSticksVertexBuffer.create();
    _labelSticksVertexBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    _labelSticksVertexBuffer.bind();

    const std::vector<Vector3F> labelStickPts = { {0,0,0}, {0,1,0}};

    _labelSticksVertexBuffer.allocate(labelStickPts.data(), labelStickPts.size()* sizeof(labelStickPts[0]));
    _labelSticksVertexBuffer.release();

    _flat3DSphere.setDirty();
    _isReady = true;
}

void View3D::update(float dt)
{
    if( !_isReady)
        return;

    if( _view->isPaused())
        return;

    _lightHouseRotAng += qMin(dt * 45.0f, 8.0f);
    _pivotRotAng += qMin(240.0f * dt, 22.0f);
}

void View3D::prepareCameraView(OpenGLPipeline& pipe)
{
    pipe.GetProjection().LoadIdentity();
    float aspect = float(_view->width())/_view->height();
    pipe.GetProjection().SetPerspective(70, aspect, 1, 400 *1609 );

    pipe.GetView().LoadIdentity();
    pipe.GetView().Scale(1.0f, 1.0f, _view->_3dZoomFactor);

    pipe.GetView().Rotate(_view->_camera._pitch, _view->_camera._heading, _view->_camera._bank);
}

const std::vector<View3D::Blip3DCache> &View3D::callSignCache() const
{
    return  _blipCache;
}

void View3D::jniTriggerVR()
{
#ifdef Q_OS_ANDROID
    QJniObject activity = QNativeInterface::QAndroidApplication::context();
    activity.callMethod<void>("TriggerRotationVector", "()V");

    // Checking for errors in the JNI
    QJniEnvironment env;
    if (env->ExceptionCheck()) {
        // Handle exception here.
        env->ExceptionClear();
    }
#endif
}

void View3D::triggerVR()
{
#ifdef Q_OS_WIN
    qDebug() << Q_FUNC_INFO;
#endif
    if( !stopGyro())
        jniTriggerVR();
}


bool View3D::stopGyro()
{
    if( isVRActive())
    {
        jniTriggerVR();
        return true;
    }

    return false;
}

bool View3D::isVRActive() const
{
#ifdef Q_OS_ANDROID
    QJniObject activity = QNativeInterface::QAndroidApplication::context();
    return activity.callMethod<jboolean>("IsVRActive", "()Z");
#endif

    return false;
}

void View3D::setBlipVerbosity(BlipVerbosity v)
{
    _blipVerbosity = v;
}

BlipVerbosity View3D::getRadarBlipVerb() const
{
    return _blipVerbosity;
}

void View3D::updateMinMaxScaleFactors()
{
    _minModelScale = std::min(_fModelFactor, _minf3DModelFactor);
    _maxModelScale = std::max(_fModelFactor, _minf3DModelFactor);
}

void View3D::setShowYou(bool bShow)
{
    _bDrawYou = bShow;
}

bool View3D::getShowYou() const
{
    return _bDrawYou;
}

void View3D::setSkyLineId(const QString &id)
{
    if( _isReady)
        _skyBox.selectId(id);
    else
        _skyLineDeferredId = id;
}

QStringList View3D::getSkyBoxIds() const
{
    return _skyBox.getIds();
}

void View3D::setTileMapActive(bool b)
{
    _flat3DSphere.setTileActive(b);
}

QString View3D::getTileMapHost() const
{
    return _flat3DSphere.getTileMapHost();
}

void View3D::setTileMapURLs(QStringList urls)
{
    for(int i=0; i < urls.size(); ++i)
        _flat3DSphere.setTileMapURL(i, urls[i]);
}

void View3D::setTileMapShowZoom(bool show)
{
    _flat3DSphere.setTileShowZoom(show);
}

void View3D::setTileMapUser(QString user)
{
    _flat3DSphere.setTileMapUser(user);
}

void View3D::seTileShowRunway(bool show)
{
    _flat3DSphere.setTileShowRunway(show);
}

void View3D::setTileMapPassword(QString password)
{
    _flat3DSphere.setTileMapPassword(password);
}

void View3D::setTileZoomVisibility(int zoom, bool visible)
{
    _flat3DSphere.setZoomVisibility(zoom, visible);
}

bool View3D::getTileMapActive() const
{
    return _flat3DSphere.getTileActive();
}

QString View3D::getTileMapURL(int idx) const
{
    return _flat3DSphere.getTileMapURL(idx);
}

void View3D::onJavaRotationVector(float x, float y, float z)
{
    _view->_camera._heading = x;
    _view->_camera._pitch = y;
    _view->_camera._bank = z;
    _view->setCompassValue(x, true);
    _view->applyCameraPitchBounds();
    _view->update();
}

void View3D::onJavaVRStarted(bool started)
{
    Q_UNUSED(started);
    _VRStarted = started;
    if( !started)
        onJavaRotationVector(jniRotationVector.x, 0, 0);

    _view->update();
}

void View3D::paintSkyLine(OpenGLPipeline& pipe)
{
    pipe.GetModel().LoadIdentity();
    _renderer->camID = pipe.GetCamID();
    glDepthMask(GL_FALSE);
    _skyBox.Render(_renderer.get(), pipe);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDepthMask(GL_TRUE);
}

void View3D::paintGL()
{
    if( !_isReady)
        return;

    if(_VRStarted)
        onJavaRotationVector(jniRotationVector.x, jniRotationVector.y, jniRotationVector.z);

    OpenGLPipeline& pipe = OpenGLPipeline::Get(0);

    if( _skyBox.isDayTime())
        glClearColor(119.0f/255, 181.0f/255, 254.0f/255,1);
    else
        glClearColor(0,0,0,1);//Dark

    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

    prepareCameraView(pipe);
    paintSkyLine(pipe);

    _flat3DSphere.setCamera(_view->getCamera());
    const auto minmag = _view->gpsOrigin()._height > 6000 ? QOpenGLTexture::Linear : QOpenGLTexture::Nearest;
    _flat3DSphere.setTextureMinMag(minmag, minmag);
    _flat3DSphere.setMod(_skyBox.rBrightness()/255.0f, _skyBox.gBrightness()/255.0f, _skyBox.bBrightness()/255.0f, 1.0f);

    glEnable(GL_DEPTH_TEST);

    _flat3DSphere.render(_view->gpsOrigin(), _skyBox.isDayTime());

    buildVehicleCache(pipe);
    paintVehicles(pipe);

    paintVehicleLabel(pipe);

    paintLines(pipe);

    paintLondonUnderground(pipe);

    paintCurrentLocation(pipe);

    paintHistoryTrail(pipe);

    paintSelected(pipe);

    paintPlaceText(pipe);
    paintRunwayText(pipe);

    paintYouText(pipe);

    glDisable(GL_DEPTH_TEST);
}

void View3D::setFlat3DSphereDirty()
{
    _flat3DSphere.setDirty();
}

float View3D::getModelScale() const
{
    return _fModelFactor;
}

void View3D::buildVehicleCache(OpenGLPipeline& pipe)
{
    auto& modelMatrix = pipe.GetModel();

    //const auto* pSelectedRadarBlip = _v->getWorldModel()->getSelectedVehicle(0);
    std::vector<Blip3DCache> localCache;

    const QSize& sze = _view->size();
    //const int border = sze.width()/4;

    const auto& vehicles = _view->getWorldModel()->getTrains();

    for( const auto& vehicle : vehicles)
    {
        const float fDistance = vehicle->getPos().distanceTo(_view->_gpsOrigin);

        if( fDistance > _visibilityDistance )
            continue;

        const float fScale = getModelScale();

        modelMatrix.LoadIdentity();

        GPSLocation g = vehicle->getPos();
        g._height = 1.0;
        TranslateLocation(modelMatrix, g);

        const Vector4F& v = from3DToHomogenous(pipe, _aboveGroundPt);
        const QPointF& pt = fromHomogenousToScreen(sze, v);

        float fHdg = 0;

        auto* trackPoint = vehicle->getTrackPoint();

        if( trackPoint != nullptr)
            fHdg = trackPoint->hdg;

        meshLoader* meshToUse = (vehicle->isBus() ? _bus : _train).get();

        auto topMatrix =  modelMatrix.Top();

        modelMatrix.Rotate(0, fHdg, 0);
        modelMatrix.Scale(fScale, fScale, fScale);

        pipe.updateFustrum();

        if( !meshToUse->InFrustum(pipe.GetFrustum()))
            continue;

        Blip3DCache cache;
        cache.blip = vehicle;
        cache.key = vehicle->_key;
        cache.fDistance = fDistance;
        cache.fHeight = g._height;
        cache.fHdg = fHdg;
        cache.pt = pt;
        cache.M = topMatrix;
        cache.scale = fScale;
        cache.meshToUse = meshToUse;

        if( v.z <0)
            cache.fDistance = -fDistance;

        localCache.push_back(cache);
    }

    std::sort( localCache.begin(), localCache.end());

    _blipCache = std::vector<Blip3DCache>(localCache.begin(), localCache.begin() + qMin(_maxPlaneCount,localCache.size()));
}

void View3D::paintVehicles(OpenGLPipeline &pipe)
{
    auto& modelMatrix = pipe.GetModel();
    const float rotAngle = getRotAng();

    auto defaultDiffColor = [](OpenGLShaderProgram& prog)
    {
        prog.sendUniform("ambientColor", 0.40f, 0.40f, 0.40f);
        prog.sendUniform("diffuseColor", 0.85f, 0.85f, 0.85f);
    };

    auto proximityWarningColor = [](OpenGLShaderProgram& prog)
    {
        prog.sendUniform("ambientColor", 0.45f, 0.0f, 0.0f);
        prog.sendUniform("diffuseColor", 1.0f, 0.0f, 0.0f);
    };

    auto selectedBlipColor = [](OpenGLShaderProgram& prog)
    {
        prog.sendUniform("ambientColor", 0.45f, 0.45f, 0.45f);
        prog.sendUniform("diffuseColor", 1.0f, 1.0f, 1.0f);
    };

    _shaderTex3D.use();
    _shaderTex3D.sendUniform("useLightDir", 1);
    _shaderTex3D.sendUniform("lightDir", 45, 45, 45);
    defaultDiffColor(_shaderTex3D);

    const std::shared_ptr<Vehicle>& pSelectedRadarBlip = _view->getWorldModel()->getSelectedVehicle(0);

    const bool proxWarning = _view->_proximityWarningInProgress;
    bool updatedProxWarning = false;

    for(const Blip3DCache& item : _blipCache)
    {
        const std::shared_ptr<const Vehicle>& vehicle = item.blip;

        bool bProxmityWarning = _view->isProximityWarning(vehicle);

        updatedProxWarning |= bProxmityWarning;

        bool shaderChanged = false;

        modelMatrix.Load(item.M);

        modelMatrix.Scale(item.scale, item.scale, item.scale);
        modelMatrix.Rotate(0, item.fHdg, 0);

        pipe.bindMatrices(_shaderTex3D);

        if( bProxmityWarning)
        {
            shaderChanged = true;
            proximityWarningColor(_shaderTex3D);
        }
        else if( pSelectedRadarBlip == vehicle)
        {
            shaderChanged = true;
            selectedBlipColor(_shaderTex3D);
        }

        const auto& meshes = item.meshToUse->getMeshes();

        for(auto mesh :meshes)
        {
            bool rot = false;
            bool isStrobe = false;
            bool drawStrobe = false;

            if( mesh->hasPivot())
            {
                pivotData pd;
                pd.rotation = rotAngle;

                modelMatrix.Push();
                mesh->applyPivot(modelMatrix, pd);
                pipe.bindMatrices(_shaderTex3D);
                modelMatrix.Pop();
                rot = true;
            }

            if( !isStrobe || drawStrobe)
                mesh.draw(_shaderTex3D);

            if(rot) pipe.bindMatrices(_shaderTex3D);

            if( drawStrobe)
            {
                _shaderTex3D.sendUniform("useLightDir", 1);

                if( bProxmityWarning)
                    proximityWarningColor(_shaderTex3D);
                else if( pSelectedRadarBlip == vehicle)
                    selectedBlipColor(_shaderTex3D);
                else
                    defaultDiffColor(_shaderTex3D);
            }
        }

        if( shaderChanged)
            defaultDiffColor(_shaderTex3D);
    }

    if( updatedProxWarning != proxWarning)
    {
        _view->_proximityWarningInProgress = updatedProxWarning;
        emit _view->onProximityWarningActive(updatedProxWarning);
    }
}

void View3D::paintLines(OpenGLPipeline &pipe)
{
    auto& modelMatrix = pipe.GetModel();

    const auto& lines = _view->getWorldModel()->getLines();

    _flatShader3D.use();

    for(const auto& line: lines)
    {
        if( !line->isVisible())
            continue;

        QColor c = line->getColor();

        _flatShader3D.sendUniform("Color", c.redF(), c.greenF(), c.blueF(), 1.0f);

        const Branches& branches = line->getBranches();

        for( const auto& branch : branches)
        {
            for(const auto& smoothPt : branch->getSmoothPoints())
            {
                modelMatrix.LoadIdentity();
                TranslateModelLocation(modelMatrix, smoothPt->position);
                modelMatrix.Rotate(0,smoothPt->hdg,0);
                modelMatrix.Scale(10, 1.0f, 50);
                modelMatrix.Translate(0.0f, 5.0f, -0.5f);

                if( !objectInScreen(pipe))
                    continue;

                const float fDistance = _view->_gpsOrigin.distanceTo(smoothPt->position);

                if( fDistance > _visibilityDistance)
                    continue;

                pipe.bindMatrices(_flatShader3D);
//                _unitSquare->draw(_flatShader3D);
//                _unitRing->draw(_flatShader3D);
                _unitBox->draw(_flatShader3D);
            }
        }
    }
}

void View3D::paintLondonUnderground(OpenGLPipeline &pipe)
{
    auto& modelMatrix = pipe.GetModel();

    _metroProgram.use();
    _metroProgram.sendUniform("factor", 1.2f);

    const auto& lines = _view->getWorldModel()->getLines();

    for(const auto& line: lines)
    {
        if( !line->isVisible())
            continue;

        const auto& stopPoints = line->getStopPoints();

        QColor c = line->getColor();

        _flatShader3D.sendUniform("Color", c.redF(), c.greenF(), c.blueF(), 1.0f);

        const float scale = line->isBus()? 0.5f: 1.0f;

        meshLoader* meshToUse = _londonTube.get();

        if( line->isDLR())
            meshToUse = _londonDLR.get();
        else if( line->isBus())
            meshToUse = _londonBus.get();

        for(const auto& stopPoint : stopPoints)
        {
            if( stopPoint.second->isPassPoint)
                continue;

            modelMatrix.LoadIdentity();
            TranslateModelLocation(modelMatrix, stopPoint.second->position);
            modelMatrix.Rotate(0, _view->_camera._heading, 0);
            modelMatrix.Scale(scale, scale, scale);

            if( !objectInScreen(pipe))
                continue;

            const float fDistance = _view->_gpsOrigin.distanceTo(stopPoint.second->position);

            if( fDistance > _visibilityDistance)
                continue;

            pipe.bindMatrices(_metroProgram);
            meshToUse->draw(_metroProgram);
        }
    }

    glEnable(GL_BLEND);
    _alphaFontRenderer.setColor(_white);
    _alphaFontRenderer.beginRender();

    for(const auto& line: lines)
    {
        if( !line->isVisible())
            continue;

        const auto& stopPoints = line->getStopPoints();

        for(const auto& stopPoint : stopPoints)
        {
            if( stopPoint.second->isPassPoint)
                continue;

            const float scale = line->isBus()? 0.5f: 1.0f;
            modelMatrix.LoadIdentity();
            TranslateModelLocation(modelMatrix, stopPoint.second->position);

            if( !objectInScreen(pipe))
                continue;

            const float fDistance = _view->_gpsOrigin.distanceTo(stopPoint.second->position);

            if( fDistance > _visibilityDistance)
                continue;

            modelMatrix.Translate(0,100*scale,0);
            modelMatrix.Scale(scale, -scale, scale);
            modelMatrix.Rotate(-_view->_camera._pitch, _view->_camera._heading,0);
            modelMatrix.Translate(-_alphaFontRenderer.getWidth(stopPoint.second->displayName)/2, 0, 0);

            _alphaFontRenderer.bindMatrices();
            _alphaFontRenderer.renderText( 0, 0, stopPoint.second->displayName);
        }
    }

    _alphaFontRenderer.endRender();
    glDisable(GL_BLEND);
}

void View3D::paintCurrentLocation(OpenGLPipeline& pipe)
{
    //Draw current location
    if( !_bDrawYou || _view->myLocation() == GPSLocation())
        return;

    auto& modelMatrix = pipe.GetModel();

    _flatShader3D.use();
    _flatShader3D.sendUniform("Color", 0.5f, 0.5f, 0.0f, 1.0f);

    GPSLocation myLoc = _view->_myLocation;
    myLoc._height = 0.0f;
    modelMatrix.LoadIdentity();
    TranslateLocation(modelMatrix, myLoc);

    modelMatrix.Scale(50.0f, 1.0f, 50.0f);
    modelMatrix.Translate(0,5,0);
    pipe.bindMatrices(_flatShader3D);
    _unitRing->draw(_flatShader3D);
}

void View3D::paintHistoryTrail(OpenGLPipeline &pipe)
{
//    if( !_v->isShow3DTrail())
//        return;

//    auto& modelMatrix = pipe.GetModel();

//    const IRadarBlip* pSelectedRadarBlip = _v->getWorldModel()->getSelectedBlip(0);
//    _shader3D.use();
//    _shader3D.sendUniform("ambientColor", 50, 50, 0);
//    _shader3D.sendUniform("diffuseColor", 200, 200, 0);

//    for(const auto& cache:_blipCache)
//    {
//        IRadarBlip* pRadarBlip = cache.blip;

//        if( pRadarBlip == pSelectedRadarBlip)
//            continue;

//        if( pRadarBlip->GetGAlt() < 400)
//            continue;

//        const std::deque<GPSLocation>& trail = pRadarBlip->GetHistory();

//        const size_t trailSize = trail.size();
//        const size_t trailInterval = RadarBlip::blipTrailInterval();

//        if( trailSize == 0)
//            continue;

//        for(size_t i =0; i < trailSize; i+=trailInterval)
//        {
//            const GPSLocation& loc = trail[i];
//            modelMatrix.LoadIdentity();
//            TranslateLocation(modelMatrix, loc);
//            modelMatrix.Scale(_fModelFactor, _fModelFactor, _fModelFactor);
//            modelMatrix.Translate(0,0.5f,0);
//            pipe.bindMatrices(_shader3D);
//            _unitBox->draw(_shader3D);
//        }
//    }
}

void View3D::paintSelected(OpenGLPipeline &pipe)
{
//    const IRadarBlip* pSelectedRadarBlip = _v->getWorldModel()->getSelectedBlip(0);

//    if( pSelectedRadarBlip != nullptr && pSelectedRadarBlip->GetGAlt() > 400)
//    {
//        auto& modelMatrix = pipe.GetModel();

//        _shader3D.use();

//        _shader3D.sendUniform("ambientColor", 0.45f, 0.45f, 0.0f);
//        _shader3D.sendUniform("diffuseColor", 0.85f, 0.85f, 0.0f);

//        const std::deque<GPSLocation>& history = pSelectedRadarBlip->GetHistory();

//        const size_t historySize = history.size();

//        ADSBMeshInfo::Type t = _models.GetAircraftType(pSelectedRadarBlip);

//        const float fScale = getModelScale(pSelectedRadarBlip, t);
//        const int blipTrailInterval = 2;

//        //Ignore last history which is in the plane
//        if( historySize >0)
//        for(size_t i =0; i < historySize-1; i+=blipTrailInterval)
//        {
//            const GPSLocation& loc = history[i];
//            modelMatrix.LoadIdentity();
//            TranslateLocation(modelMatrix, loc);
//            modelMatrix.Scale(fScale, fScale, fScale);

//            pipe.bindMatrices(_shader3D);
//            _unitBox->draw(_shader3D);
//        }

//        //Shadow of trails
//        _shader3D.sendUniform("ambientColor", 0.25f, 0.25f, 0.25f);
//        _shader3D.sendUniform("diffuseColor", 0.25f, 0.25f, 0.25f);

//        for(size_t i =0; i < historySize; i+=blipTrailInterval)
//        {
//            GPSLocation loc = history[i];
//            loc._height = 0.0f;
//            modelMatrix.LoadIdentity();
//            TranslateLocation(modelMatrix, loc);
//            modelMatrix.Scale(25, 1, 25);
//            pipe.bindMatrices(_shader3D);
//            _unitBox->draw(_shader3D);
//        }
//    }
}

void View3D::paintYouText(OpenGLPipeline &pipe)
{
    if( !_bDrawYou)
        return;

    if( _view->myLocation() == GPSLocation())
        return;

    const float fDistance = _view->_myLocation.distanceTo(_view->_gpsOrigin);
    if( fDistance > _FiftyMiles)
        return;
    auto loc =  _view->_myLocation;
    loc._height = 0.0f;

    auto& modelMatrix = pipe.GetModel();
    modelMatrix.LoadIdentity();
    TranslateLocation(modelMatrix, loc);
    modelMatrix.Translate(0,0,0);
    const float Sc = qMax(1.0f, 1.0f * fDistance/20000);
    modelMatrix.Scale(Sc, -Sc, Sc);
    modelMatrix.Rotate(-_view->_camera._pitch, _view->_camera._heading,0);
    modelMatrix.Translate(-_alphaFontRenderer.getWidth("YOU")/2, 0, 0);

    glEnable(GL_BLEND);
    _alphaFontRenderer.setColor(_yellow);
    _alphaFontRenderer.beginRender();
    _alphaFontRenderer.bindMatrices();
    _alphaFontRenderer.renderText( 0, 0, "YOU");
    _alphaFontRenderer.endRender();
    glDisable(GL_BLEND);
}

void View3D::paintRunwayText( OpenGLPipeline &pipe)
{
//    RadarVisualItems& rvi = _v->getWorldModel()->getVisualItems();
//    auto& modelMatrix = pipe.GetModel();

//    glEnable(GL_BLEND);
//    _alphaFontRenderer.setColor(_yellow);
//    _alphaFontRenderer.beginRender();

//    for ( auto it = rvi.runwayBeginItr(); it != rvi.runwayEndItr(); ++it)
//    {
//        const RunwayVisualItem* rvi = it->second.get();

//        const float fDistFrom = _v->_gpsOrigin.distanceTo(rvi->From());
//        const float fDistTo = _v->_gpsOrigin.distanceTo(rvi->To());

//        const int fromWidth = _alphaFontRenderer.getWidth(rvi->IdFrom())/2;
//        const int toWidth = _alphaFontRenderer.getWidth(rvi->IdTo())/2;

//        {

//            const float Sc = qMax(5.0f, 5.0f * fDistFrom/20000);
//            modelMatrix.LoadIdentity();
//            TranslateModelLocation(modelMatrix, rvi->From());
//            modelMatrix.Translate(0,5,0);
//            modelMatrix.Scale(Sc, -Sc, Sc);
//            modelMatrix.Rotate(-90, rvi->record()->fBearing,0);

//            modelMatrix.Translate(-fromWidth, 0, 0);
//            _alphaFontRenderer.bindMatrices();
//            _alphaFontRenderer.renderText( 0, 0, rvi->IdFrom());

//            const float ScTo = qMax(5.0f, 5.0f * fDistTo/20000);
//            modelMatrix.LoadIdentity();
//            TranslateModelLocation(modelMatrix, rvi->To());
//            modelMatrix.Translate(0,5,0);
//            modelMatrix.Scale(ScTo, -ScTo, ScTo);
//            modelMatrix.Rotate(-90, rvi->record()->fBearing+180,0);

//            modelMatrix.Translate(-toWidth, 0, 0);

//            _alphaFontRenderer.bindMatrices();
//            _alphaFontRenderer.renderText( 0, 0, rvi->IdTo());
//        }
//    }

//    glDisable(GL_BLEND);

//    _fontRenderer.setColor(_skyBox.isDayTime()? _white :_black);
//    _fontRenderer.setBkColor(_skyBox.isDayTime()? _black:_white);
//    _fontRenderer.beginRender();

//    for ( auto it = rvi.runwayBeginItr(); it != rvi.runwayEndItr(); ++it)
//    {
//        const RunwayVisualItem* rvi = it->second.get();

//        const float fDistFrom = _v->_gpsOrigin.distanceTo(rvi->From());
//        const float fDistTo = _v->_gpsOrigin.distanceTo(rvi->To());

//        const int fromWidth = _fontRenderer.getWidth(rvi->IdFrom())/2;
//        const int toWidth = _fontRenderer.getWidth(rvi->IdTo())/2;

//        const AptNavRunwayRecord *r = rvi->record();

//        int iMaxMiles = 10;

//        if( r->fDistance < 2000)
//            iMaxMiles = 4;

//        if( r->fDistance < 1000)
//            iMaxMiles = 2;

//        if( r->fDistance < 500)
//            iMaxMiles = 1;

//        const float length = Units::NmToMeters(iMaxMiles);
//        const float Sc = qMax(10.0f, 10.0f * fDistFrom/20000);
//        const float height = 50;

//        {
//            modelMatrix.LoadIdentity();
//            GPSLocation from = rvi->From() -length* r->vBearingUnit;
//            TranslateModelLocation(modelMatrix, from);
//            modelMatrix.Scale(Sc, -Sc, Sc);
//            modelMatrix.Rotate(-90, rvi->record()->fBearing,0);
//            modelMatrix.Translate(-fromWidth, -height, 0);

//            _fontRenderer.bindMatrices();
//            _fontRenderer.renderText( 0, 0, rvi->IdFrom());

//            const float ScTo = qMax(10.0f, 10.0f * fDistTo/20000);
//            modelMatrix.LoadIdentity();
//            GPSLocation to = rvi->To() +length* r->vBearingUnit;
//            TranslateModelLocation(modelMatrix, to);
//            modelMatrix.Scale(ScTo, -ScTo, ScTo);
//            modelMatrix.Rotate(-90, rvi->record()->fBearing+180,0);
//            modelMatrix.Translate(-toWidth, -height, 0);
//            _fontRenderer.bindMatrices();
//            _fontRenderer.renderText( 0, 0, rvi->IdTo());
//        }

//        ///////////////////////////////////////////////

//        {
//            modelMatrix.LoadIdentity();
//            GPSLocation from = rvi->From() - _nm2 * r->vBearingUnit;
//            TranslateModelLocation(modelMatrix, from);
//            modelMatrix.Scale(Sc, -Sc, Sc);
//            modelMatrix.Rotate(-90, rvi->record()->fBearing,0);
//            modelMatrix.Translate(-fromWidth, -height, 0);
//            _fontRenderer.bindMatrices();
//            _fontRenderer.renderText( 0, 0, rvi->IdFrom());

//            const float ScTo = qMax(10.0f, 10.0f * fDistTo/20000);
//            modelMatrix.LoadIdentity();
//            GPSLocation to = rvi->To() +_nm2* r->vBearingUnit;
//            TranslateModelLocation(modelMatrix, to);
//            modelMatrix.Scale(ScTo, -ScTo, ScTo);
//            modelMatrix.Rotate(-90, rvi->record()->fBearing+180,0);
//            modelMatrix.Translate(-toWidth, -height, 0);
//            _fontRenderer.bindMatrices();
//            _fontRenderer.renderText( 0, 0, rvi->IdTo());
//        }
//    }

//    _fontRenderer.endRender();
}

void View3D::paintVehicleLabel(OpenGLPipeline &pipe)
{
    if( getRadarBlipVerb() == BlipVerbosity::none)
        return;

    auto& modelMatrix = pipe.GetModel();

    auto* r= _renderer.get();

    r->useProgram(_flatShader3D);

    _flatShader3D.sendUniform("Color", _skyBox.isDayTime()? _black:_white);

    _labelSticksVertexBuffer.bind();
    r->bindVertex(Renderer::Vertex, 3);

    const float scaleWeight = 20.0f;
    const float distWeight = 200.0f;

    for(const auto& cache:_blipCache)
    {
        const std::shared_ptr<const Vehicle>& pRadarBlip = cache.blip;

        modelMatrix.LoadIdentity();
        TranslateModelLocation(modelMatrix, pRadarBlip->getPos());
        modelMatrix.Translate(0,cache.fHeight,0);

        modelMatrix.Scale(1.0f, cache.scale*scaleWeight+ cache.fDistance/distWeight, 1.0f);

        pipe.bindMatrices(_flatShader3D);

        r->drawArrays(GL_LINES, 2);
    }

    r->unBindBuffers();

    glEnable(GL_BLEND);
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    _fontRenderer.setColor(_white);
    _fontRenderer.setBkColor(_skyBox.isDayTime()? Vector4F(0,0,0,0.45f): Vector4F(0,0,0,0.25f));

    Vector4F lineColor;

    _fontRenderer.beginRender();

    for(const auto& cache:_blipCache)
    {
        const std::shared_ptr<const Vehicle>& vehicle = cache.blip;

        QStringList labels;

        QString strText = vehicle->_displayTowards;

        if( strText.isEmpty())
            strText = vehicle->_destinationName;

        if( !strText.isEmpty())
            labels << strText;

        if( vehicle->_line != nullptr)
        {
            const bool busLineId = vehicle->isBus() && (_view->_busBlipVerbosity == BlipVerbosity::LineId || _view->_busBlipVerbosity == BlipVerbosity::All);
            const bool trainLineId = !vehicle->isBus() && (_view->_trainBlipVerbosity == BlipVerbosity::LineId || _view->_trainBlipVerbosity == BlipVerbosity::All);

            if( busLineId || trainLineId)
                labels << vehicle->_line->name();
        }

        const bool busVehicleId = vehicle->isBus() && (_view->_busBlipVerbosity == BlipVerbosity::VehicleId || _view->_busBlipVerbosity == BlipVerbosity::All);
        const bool trainVehicleId = !vehicle->isBus() && (_view->_trainBlipVerbosity == BlipVerbosity::VehicleId || _view->_trainBlipVerbosity == BlipVerbosity::All);

        if(busVehicleId || trainVehicleId)
        {
            if( !vehicle->_vehicleId.isEmpty())
                labels << vehicle->_vehicleId;

            if( vehicle->isFast())
                labels << QStringLiteral("(fast)");
        }

        const auto& items = labels.toVector();
        std::vector<QString> vectorLabels = {items.begin(), items.end()};

        modelMatrix.LoadIdentity();
        TranslateModelLocation(modelMatrix, vehicle->getPos());

        const float Sc = 1.0;//qMin(100.0f, 3.0f * fDistance/1500);

        modelMatrix.Translate(0, cache.fHeight, 0);

        modelMatrix.Translate(0, cache.scale*scaleWeight + cache.fDistance/distWeight, 0);
        const float sFactor = Sc;///_v->_3dZoomFactor;

        modelMatrix.Scale(sFactor, -sFactor, sFactor);

        modelMatrix.Rotate(-_view->_camera._pitch, _view->_camera._heading,0);

        _fontRenderer.bindMatrices();
        _fontRenderer.renderText( 0, 0, vectorLabels);
    }

    _fontRenderer.endRender();
    glDisable(GL_BLEND);
}

void View3D::TranslateModelLocation(OpenGLMatrixStack &pipe, const GPSLocation &loc)
{
    TranslateLocation(pipe, loc, true);
}

void View3D::TranslateLocation(OpenGLMatrixStack &pipe, const GPSLocation &loc, bool pinToSeaLevel)
{
    float bearing = _view->_gpsOrigin.bearingTo(loc);
    float dist = _view->_gpsOrigin.distanceTo(loc);
    float height = pinToSeaLevel? - _view->_gpsOrigin._height :  loc._height - _view->_gpsOrigin._height;

    QuarternionF q = MathSupport<float>::MakeQHeading(bearing);
    Vector3F vF = QVRotate(q, Vector3F(0,height,-dist) );

    pipe.Translate(vF.x, vF.y, vF.z);
}

float View3D::getRotAng() const
{
    return _pivotRotAng;
}

bool View3D::objectInScreen(const OpenGLPipeline &pipe) const
{
    const Vector4F& v = from3DToHomogenous(pipe, _aboveGroundPt);
    const QPointF& pt = fromHomogenousToScreen(_view->size(), v);
    return _view->ptInScreen(pt.toPoint());
}

void View3D::paintPlaceText(OpenGLPipeline &pipe)
{
//    if(!_v->isShowPlaces())
//        return;

//    auto& modelMatrix = pipe.GetModel();

//    RadarVisualItems& rvi = _v->getWorldModel()->getVisualItems();

//    glEnable(GL_BLEND);
//    _alphaFontRenderer.setColor(_white);
//    _alphaFontRenderer.beginRender();

//    const float zF = qMin(1.0f,_v->_3dZoomFactor/2);

//    for ( auto it = rvi.placeBeginItr(); it != rvi.placeEndItr(); ++it)
//    {
//        const GPSLocation& pos = it->second;
//        const float fDistance = _v->_gpsOrigin.distanceTo(pos);
//        modelMatrix.LoadIdentity();
//        TranslateLocation(modelMatrix, pos);

//        if( !objectInScreen(pipe))
//            continue;

//        const float Sc = qMax(5.0f, 5.0f * fDistance/20000)/zF;
//        modelMatrix.Scale(Sc, -Sc, Sc);
//        modelMatrix.Rotate(-_v->_camera._pitch, _v->_camera._heading,0);
//        modelMatrix.Translate(-_alphaFontRenderer.getWidth(it->first)/2, 0, 0);

//        _alphaFontRenderer.bindMatrices();
//        _alphaFontRenderer.renderText( 0, 0, it->first);
//    }

//    _alphaFontRenderer.endRender();
//    glDisable(GL_BLEND);
}

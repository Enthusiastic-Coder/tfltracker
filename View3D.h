#ifndef VIEW3D_H
#define VIEW3D_H

#include <jibbs/math/euler.h>
#include <jibbs/opengl/OpenGLPipeline.h>
#include <jibbs/opengl/OpenGLShaderProgram.h>
#include <jibbs/opengl/QtTextureManager.h>
#include <jibbs/opengl/OpenGLFontRenderer.h>
#include <jibbs/opengl/OpenGLFontTexture.h>
#include <jibbs/opengl/OpenGLFontRenderer.h>
#include <jibbs/opengl/SkyBox.h>
#include <jibbs/maptiles/Flat3DSpere.h>
#include <jibbs/mesh/meshloader.h>
#include <jibbs/mesh/AssimpMeshManager.h>

#include <QPainter>
#include <QVector>
#include <QOpenGLFunctions>
#include <memory>

#include "BlipVerbosity.h"
#include "Units.h"
#include "Units.h"
#include "BlipVerbosity.h"

//#include <QGyroscope>
//#include <QGyroscopeReading>

class TFLView;
struct Vehicle;
//struct IRadarBlip;

class View3D : public QObject, public QOpenGLFunctions
{
    Q_OBJECT
public:
    struct Blip3DCache
    {
        std::shared_ptr<const Vehicle> blip;
        QString key;
        QPointF pt;
        float fDistance;
        float fHeight;
        float fHdg;
        meshLoader* meshToUse = nullptr;
        Matrix4x4F M;
        mutable float scale;
        bool operator<(const Blip3DCache& rhs) const
        {
            return fDistance < rhs.fDistance;
        }
    };

    View3D(TFLView*);
    ~View3D();

    bool isReady() const;

    void setQtTexManager(std::shared_ptr<QtTextureManager> manager);
    void init();
    void update(float dt);
    void paintGL();

    void setFlat3DSphereDirty();

    const std::vector<Blip3DCache> &callSignCache() const;

    void setPinSeaLevel(bool bPin);
    bool isPinSeaLevel() const;

    bool isVRActive() const;

    bool stopGyro();

    void setBlipVerbosity(BlipVerbosity v);
    BlipVerbosity getRadarBlipVerb() const;

    void setShowYou(bool bShow);
    bool getShowYou() const;

    void setSkyLineId(const QString& id);
    QStringList getSkyBoxIds() const;

    void setTileMapActive(bool b);
    QString getTileMapHost() const;

    void setTileMapURLs(QStringList urls);
    void setTileMapShowZoom(bool show);
    void setTileMapUser(QString user);
    void seTileShowRunway(bool show);
    void setTileMapPassword(QString password);
    void setTileZoomVisibility(int zoom, bool visible);
    bool getTileMapActive() const;
    QString getTileMapURL(int idx) const;

public slots:
    void onJavaRotationVector(float x, float y, float z);
    void triggerVR();

private slots:
    void onJavaVRStarted(bool started);

private:
    void prepareCameraView(OpenGLPipeline& pipe);
    void paintSkyLine(OpenGLPipeline& pipe);
    void buildVehicleCache(OpenGLPipeline &pipe);
    void paintVehicles(OpenGLPipeline& pipe);
    void paintLines(OpenGLPipeline& pipe);

    void paintLondonUnderground(OpenGLPipeline& pipe);

    void paintCurrentLocation(OpenGLPipeline &pipe);
    void paintPlaceText(OpenGLPipeline &pipe);

    void paintHistoryTrail(OpenGLPipeline &pipe);
    void paintSelected(OpenGLPipeline &pipe);

    void paintYouText(OpenGLPipeline &pipe);
    void paintRunwayText(OpenGLPipeline &pipe);
    void paintVehicleLabel(OpenGLPipeline & pipe);

    void jniTriggerVR();

private:
    float getRotAng() const;
    float getSweepAng() const;
    bool objectInScreen(const OpenGLPipeline &pipe) const;
    void updateMinMaxScaleFactors();
    void TranslateModelLocation(OpenGLMatrixStack &pipe, const GPSLocation & loc);
    void TranslateLocation(OpenGLMatrixStack& pipe, const GPSLocation& loc, bool pinToSeaLevel=false);
    float getModelScale() const;

    using callBackMeshLoader = std::function<void(std::unique_ptr<meshLoader> &)>;
    void extractModel(std::unique_ptr<meshLoader>& obj, QString path,callBackMeshLoader impl=nullptr);

private:
    bool _initDone = false;
    bool _isReady = false;

    bool _bDrawYou = true;

    const float _visibilityDistance = Units::NmToMeters(5.0f);

    float _pinchCompassReading = 0.0f;
    float _oldCompassReading = 0.0f;
    float _finalCompassReading=0.0f;
    QTransform _compassTransform;
    QTransform _invCompassTransform;

    const Vector4F _white = Vector4F(1,1,1,1);
    const Vector4F _black = Vector4F(0,0,0,1);
    const Vector4F _yellow = Vector4F(1,1,0,1);

    const Vector4F _groundPt = Vector4F(0,0,0,1);
    const Vector4F _aboveGroundPt = Vector4F(0,8,0,1);
    const float _minf3DModelFactor = 5.0f;
    float _fModelFactor = 10.0f;
    float _minModelScale = 0.0f;
    float _maxModelScale = 0.0f;
    mutable float _pivotRotAng = 0.0f;
    float _lightHouseRotAng = 0.0f;
    const float _FiftyMiles = Units::NmToMeters(50.0f);
    const size_t _maxPlaneCount = 50;

    TFLView* const _view = nullptr;
    std::vector<Blip3DCache> _blipCache;

    std::shared_ptr<AssimpMeshManager> _meshManager = std::make_shared<AssimpMeshManager>();

    Flat3DSphere _flat3DSphere;
    std::unique_ptr<Renderer> _renderer;
    SkyBox _skyBox;

    std::shared_ptr<QtTextureManager> _texManager;

    OpenGLShaderProgram _flatShader3D;
    OpenGLShaderProgram _shader3D;
    OpenGLShaderProgram _texShader;
    OpenGLShaderProgram _shaderTex3D;
    OpenGLShaderProgram _fontShaderProgram;
    OpenGLShaderProgram _fontAlphaShaderProgram;
    OpenGLShaderProgram _runwayLiteProgram;
    OpenGLShaderProgram _metroProgram;
    OpenGLShaderProgram _shaderColTex3d;

    std::unique_ptr<meshLoader> _unitBox;
    std::unique_ptr<meshLoader> _unitSquare;
    std::unique_ptr<meshLoader> _unitRing;
    std::unique_ptr<meshLoader> _unitMiniRing;
    std::unique_ptr<meshLoader> _unitRunway;
    std::unique_ptr<meshLoader> _bus;
    std::unique_ptr<meshLoader> _train;

    std::unique_ptr<meshLoader> _londonTube;
    std::unique_ptr<meshLoader> _londonDLR;
    std::unique_ptr<meshLoader> _londonBus;

    BlipVerbosity _blipVerbosity = BlipVerbosity::LineId;

    QString _skyLineDeferredId;

    bool _VRStarted = false;

    OpenGLFontRenderer _alphaFontRenderer;
    OpenGLFontRenderer _fontRenderer;
    OpenGLFontTexture _myFontTexture;

    QOpenGLBuffer _labelSticksVertexBuffer = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
};

#endif // VIEW3D_H

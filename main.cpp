#include <jibbs/utilities/QmlClipboardAdapter.h>
#include <jibbs/maptiles/MapTileEntries.h>

#include <QGuiApplication>
#include <QSurfaceFormat>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSurfaceFormat>
#include <QIcon>
#include <QQuickStyle>
#include <QSettings>
#include <QQuickWindow>


#include "TrackerGlue.h"
#include "TFLViewFrameBuffer.h"
#include "UI.h"
#include "TFLLine.h"

#ifdef Q_OS_WIN
#include <Windows.h>
#endif

#ifdef TILE_MANAGER_TEST
#include "TileManager.h"
#endif

int main(int argc, char *argv[])
{
#ifdef TILE_MANAGER_TEST
    TileManager m;
    m.setFolder("C:/Project/GIT/TFLTest/tiles");

    TileManager::spec spec = m.getSpec(GPSLocation(51.4964, -0.300198), 319);

    QImage img = m.getImage(spec, true);

    img.save(QString("C:/Project/GIT/TFLTest/tiles/%1_%2_%3.png").arg(spec.zoomLevel).arg(spec.index_x).arg(spec.index_y));
    return 0;
#endif

#ifdef Q_OS_WIN
        SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED);
#endif

    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGLRhi);

    QSurfaceFormat fmt;
    fmt.setDepthBufferSize(24);
    fmt.setAlphaBufferSize(8);
    fmt.setStencilBufferSize(8);
    QSurfaceFormat::setDefaultFormat(fmt);

    QGuiApplication a(argc, argv);

    a.setOrganizationName("com.enthusiasticcoder.tfltracker");
    a.setOrganizationDomain("enthusiasticcoder");
    a.setApplicationName("TFLTracker");

    QIcon::setThemeName("gallery");

    QStringList builtInStyles = { QLatin1String("Basic"), QLatin1String("Fusion"),
                                 QLatin1String("Imagine"), QLatin1String("Material"), QLatin1String("Universal") };

#if defined(Q_OS_WINDOWS)
    builtInStyles << QLatin1String("Windows");
#endif

    QQuickStyle::setFallbackStyle(builtInStyles[0]);

    QSettings settings;
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE"))
        QQuickStyle::setStyle(settings.value("qml/style").toString());

    // If this is the first time we're running the application,
    // we need to set a style in the settings so that the QML
    // can find it in the list of built-in styles.
    const QString styleInSettings = settings.value("qml/style").toString();
    if (styleInSettings.isEmpty())
        settings.setValue(QLatin1String("qml/style"), QQuickStyle::name());

    TrackerGlue w;
    QQmlApplicationEngine engine;

    engine.setInitialProperties({{ "builtInStyles", builtInStyles }});

    qmlRegisterType<TFLViewFrameBuffer>("CppClassLib", 1, 0, "TFLView");
    qmlRegisterType<QmlClipboardAdapter>("CppClassLib", 1, 0, "QClipboard");

    qmlRegisterUncreatableType<TFLLine>("CppClassLib", 1, 0, "TFLLine", "Can't create TFLLine");
    qmlRegisterUncreatableType<StopPoint>("CppClassLib", 1, 0, "StopPoint", "Can't create StopPoint");
    qmlRegisterUncreatableType<MapTileEntries>("CppClassLib", 1, 0, "MapTileEntries", "Can't create MapTileEntries");

    engine.rootContext()->setContextProperty("cppGlue", &w);
    engine.rootContext()->setContextProperty("ui", w.getUI());
    engine.load(QUrl(QStringLiteral("qrc:/qmlglue/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return a.exec();
}

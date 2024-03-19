#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QSGRendererInterface>

#include "mpvobject.h"

int main(int argc, char* argv[])
{
    // QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGLRhi);
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGLRhi);

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    const QUrl url(u"qrc:/SimpleQmlWithLibMPV/main.qml"_qs);

    qmlRegisterType<MpvObject>("MPV", 1, 0, "MpvObject");

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [url](QObject* obj, const QUrl& objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}

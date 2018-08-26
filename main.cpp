#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQml>
#include "cognitesdk/cognitesdk.h"

int main(int argc, char *argv[]) {
  qmlRegisterType<CogniteSDK>("Cognite", 1, 0, "CogniteSDK");
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

  QGuiApplication app(argc, argv);

  QQmlApplicationEngine engine;
  engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
  if (engine.rootObjects().isEmpty()) return -1;

  return app.exec();
}

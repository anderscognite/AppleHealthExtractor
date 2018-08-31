#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQml>
#include "cognitesdk/v0.5/cognitesdk.h"
#include "datahandler.h"
#include "healthkit/hkmanager.h"

int main(int argc, char *argv[]) {
  qmlRegisterType<CogniteSDK>("Cognite", 1, 0, "CogniteSDK");
  qmlRegisterType<HKManager>("Cognite", 1, 0, "HKManager");
  qmlRegisterType<DataHandler>("Cognite", 1, 0, "DataHandler");

  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

  QGuiApplication app(argc, argv);

  QQmlApplicationEngine engine;
  engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
  if (engine.rootObjects().isEmpty()) return -1;

  return app.exec();
}

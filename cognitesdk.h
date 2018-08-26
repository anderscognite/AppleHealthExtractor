#ifndef COGNITESDK_H
#define COGNITESDK_H

#include "types.h"

#include <QNetworkReply>
#include <QObject>
#include <QString>

#include <functional>

class CogniteSDK : public QObject {
  Q_OBJECT
 private:
  void connect(std::function<void(QNetworkReply *reply)> callback);
  QNetworkAccessManager *m_manager;

 public:
  bool m_debug = true;
  explicit CogniteSDK(QObject *parent = nullptr);
  ~CogniteSDK() {}
  Q_INVOKABLE void test();
  QJsonDocument parseResponse(const QByteArray &response);
  void get(QString url, QMap<QString, QString> params,
           std::function<void(QNetworkReply *reply)> callback);
  void getAssetsWithName(QString name,
                         std::function<void(QVector<Asset>, bool)> callback);
  void getAssets(std::function<void(QVector<Asset>, bool)> callback);
};

#endif  // COGNITESDK_H

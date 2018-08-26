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
  QString m_version = "0.5";
  QNetworkAccessManager *m_manager;
  QNetworkRequest createRequest(QUrl url);
  QUrlQuery createQuery(QMap<QString, QString> params);

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
  void getTimeSeriesWithName(
      QString name, std::function<void(QVector<TimeSeries>, bool)> callback);
};

#endif  // COGNITESDK_H

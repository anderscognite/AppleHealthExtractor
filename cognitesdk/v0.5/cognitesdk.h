#ifndef COGNITESDK_H
#define COGNITESDK_H

#include "types.h"

#include <QNetworkReply>
#include <QObject>
#include <QString>
#include <QVector>

#include <functional>

class CogniteSDK : public QObject {
  Q_OBJECT
  Q_PROPERTY(
      float progress READ progress WRITE setProgress NOTIFY progressChanged)
 private:
  void connect(std::function<void(QNetworkReply *reply)> callback);
  QString m_version = "0.5";
  QNetworkAccessManager *m_manager;
  QNetworkRequest createRequest(QUrl url);
  QUrlQuery createQuery(QMap<QString, QString> params);
  float m_progress = 0.0;

 public:
  bool m_debug = true;
  explicit CogniteSDK(QObject *parent = nullptr);
  ~CogniteSDK();
  QJsonDocument parseResponse(const QByteArray &response);
  void get(QString url, QMap<QString, QString> params,
           std::function<void(QNetworkReply *reply)> callback);
  void post(QString url, const QByteArray &body,
            std::function<void(QNetworkReply *reply)> callback);
  void getAssetsWithName(QString name,
                         std::function<void(QVector<Asset>, bool)> callback);
  void getAssets(std::function<void(QVector<Asset>, bool)> callback);
  void getTimeSeriesWithName(
      QString name, std::function<void(QVector<TimeSeries>, bool)> callback);
  void createDataPointsInTimeSeries(QString name,
                                    const QVector<DataPoint> &dataPoints,
                                    std::function<void(bool)> callback);
  float progress() const;
 public slots:
  void setProgress(float progress);
 signals:
  void progressChanged(float progress);
};

#endif  // COGNITESDK_H

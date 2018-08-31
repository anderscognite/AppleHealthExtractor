#include "cognitesdk.h"
#include "constants.h"

#include <QColor>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStandardPaths>
#include <QUrlQuery>
#include <QVariantMap>

CogniteSDK::CogniteSDK(QObject *parent)
    : QObject(parent), m_manager(new QNetworkAccessManager(this)) {}

CogniteSDK::~CogniteSDK() {}

QJsonDocument CogniteSDK::parseResponse(const QByteArray &response) {
  QJsonParseError error;
  QJsonDocument document = QJsonDocument::fromJson(response, &error);
  if (error.error != QJsonParseError::NoError) {
    return QJsonDocument();
  }

  return document;
}

QNetworkRequest CogniteSDK::createRequest(QUrl url) {
  auto request = QNetworkRequest(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader,
                    QVariant::fromValue(QString("application/json")));

  request.setRawHeader("Api-key", Constants::apiKey.toLocal8Bit());
  return request;
}

QUrlQuery CogniteSDK::createQuery(QMap<QString, QString> params) {
  QUrlQuery query;
  for (auto key : params.keys()) {
    query.addQueryItem(key, params[key]);
  }
  return query;
}

void CogniteSDK::get(QString url, QMap<QString, QString> params,
                     std::function<void(QNetworkReply *reply)> callback) {
  auto url_ = QUrl(url);
  url_.setQuery(createQuery(params));
  auto request = createRequest(url_);

  // TODO(anders.hafreager): This will be in GET, POST, DELETE etc, so
  // move the connect function into a executeRequest or something
  QNetworkReply *reply = m_manager->get(request);
  QObject::connect(reply, &QNetworkReply::finished, [reply, callback]() {
    callback(reply);
    reply->deleteLater();
  });
}

void CogniteSDK::post(QString url, const QByteArray &body,
                      std::function<void(QNetworkReply *reply)> callback) {
  auto url_ = QUrl(url);
  auto request = createRequest(url_);
  QString dataDirPath =
      QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
  dataDirPath =
      QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation)[0];
  if (!QDir(dataDirPath).exists()) {
    if (!QDir().mkdir(dataDirPath)) {
      qDebug()
          << "Fatal error: Insufficient premissions to create directory -> "
          << dataDirPath;
      return;
    }
  }
  QString fileName = dataDirPath + QString("/request%1.txt").arg(m_count++);
  // QFile f(dataDirPath + "/request.txt");
    QFile f(fileName);

  if (!f.open(QIODevice::WriteOnly)) {
    qDebug() << "Could not open file: " << f.fileName();
  } else {
    qDebug() << "Wrote body to file: " << f.fileName();
    f.write(body);
  }
  f.close();

  QByteArray postDataSize = QByteArray::number(body.size());
  request.setHeader(QNetworkRequest::ContentLengthHeader, postDataSize);

  QNetworkReply *reply = m_manager->post(request, body);
  QObject::connect(reply, &QNetworkReply::finished, [reply, callback]() {
    callback(reply);
    reply->deleteLater();
  });
}

void CogniteSDK::getAssetsWithName(
    QString name, std::function<void(QVector<Asset>, bool)> callback) {
  QString url = QString(Constants::baseUrl + m_version + "/projects/" +
                        Constants::project + "/assets/search");
  get(url, {{"name", name}}, [this, callback](QNetworkReply *reply) {
    if (reply->error() != QNetworkReply::NoError) {
      qDebug() << "Got error: " << reply->errorString();
      callback({}, true);
      return;
    }
    QByteArray response = reply->readAll();
    auto document = parseResponse(response);
    auto j_assets = document["data"]["items"].toArray();
    QVector<Asset> assets;
    for (auto j_asset : j_assets) {
      Asset asset = Asset::fromJSON(j_asset.toObject());
      assets.push_back(asset);
    }
    callback(assets, false);
  });
}

void CogniteSDK::getAssets(std::function<void(QVector<Asset>, bool)> callback) {
  QString url = QString(Constants::baseUrl + m_version + "/projects/" +
                        Constants::project + "/assets");
  get(url, {}, [this, callback](QNetworkReply *reply) {
    if (reply->error() != QNetworkReply::NoError) {
      qDebug() << "  Got error: " << reply->errorString();
      callback({}, true);
      return;
    }
    QByteArray response = reply->readAll();
    auto document = parseResponse(response);
    auto j_assets = document["data"]["items"].toArray();
    QVector<Asset> assets;
    for (auto j_asset : j_assets) {
      Asset asset = Asset::fromJSON(j_asset.toObject());
      assets.push_back(asset);
    }
    callback(assets, false);
  });
}

void CogniteSDK::getTimeSeriesWithName(
    QString name, std::function<void(QVector<TimeSeries>, bool)> callback) {
  QString url = QString(Constants::baseUrl + m_version + "/projects/" +
                        Constants::project + "/timeseries/search");
  get(url, {{"name", name}}, [this, callback](QNetworkReply *reply) {
    if (reply->error() != QNetworkReply::NoError) {
      qDebug() << "Got error: " << reply->errorString();
      callback({}, true);
      return;
    }
    QByteArray response = reply->readAll();
    auto document = parseResponse(response);
    QVector<TimeSeries> timeSeries;
    for (auto j_timeSeries : document["data"]["items"].toArray()) {
      TimeSeries ts = TimeSeries::fromJSON(j_timeSeries.toObject());
      timeSeries.push_back(ts);
    }
    callback(timeSeries, false);
  });
}

void CogniteSDK::createDataPointsInTimeSeries(
    QString name, const QVector<DataPoint> &dataPointsIn,
    std::function<void(bool)> callback) {
  QString url = QString(Constants::baseUrl + m_version + "/projects/" +
                        Constants::project + "/timeseries/data");

  QVariantMap data;

  QVariantList dataPoints;
  for (const DataPoint &p : dataPointsIn) {
    dataPoints.push_back(
        QVariantMap({{"timestamp", p.timestamp}, {"value", p.value}}));
  }

  QVariantList items;

  items.push_back(QVariantMap({{"name", name}, {"datapoints", dataPoints}}));
  data["items"] = items;
  QJsonDocument json = QJsonDocument::fromVariant(data);
  QByteArray body = json.toJson();
  //  qDebug() << "Got document: " << json.toJson();

  post(url, body, [this, callback](QNetworkReply *reply) {
    if (reply->error() != QNetworkReply::NoError) {
      qDebug() << "Got error: " << reply->errorString();
      callback(true);
      return;
    }
    callback(false);
  });
}

float CogniteSDK::progress() const { return m_progress; }

void CogniteSDK::setProgress(float progress) {
  qWarning("Floating point comparison needs context sanity check");
  if (qFuzzyCompare(m_progress, progress)) return;

  m_progress = progress;
  emit progressChanged(m_progress);
}

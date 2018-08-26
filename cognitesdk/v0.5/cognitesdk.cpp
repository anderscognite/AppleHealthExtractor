#include "cognitesdk.h"
#include "constants.h"

#include <QColor>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrlQuery>

CogniteSDK::CogniteSDK(QObject *parent)
    : QObject(parent), m_manager(new QNetworkAccessManager(this)) {}

void CogniteSDK::test() {
  getAssetsWithName("Heart Rate", [&](QVector<Asset> assets, bool error) {
    qDebug() << "Got error: " << error;

    qDebug() << "Got assets actually: " << assets.size();
    for (Asset asset : assets) {
      qDebug() << "Asset: " << asset.name;
    }
  });

  getTimeSeriesWithName("Heart Rate",
                        [&](QVector<TimeSeries> timeSeries, bool error) {
                          qDebug() << "Got error: " << error;

                          qDebug() << "Got ts actually: " << timeSeries.size();
                          for (TimeSeries ts : timeSeries) {
                            qDebug() << "TS: " << ts.name;
                          }
                        });
}

QJsonDocument CogniteSDK::parseResponse(const QByteArray &response) {
  qDebug() << "Got response to parse: " << response;
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
    reply->deleteLater();
    callback(reply);
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

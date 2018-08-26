#include "types.h"

#include <QDebug>
#include <QVariant>

Asset Asset::fromJSON(const QJsonObject &object) {
  QString name = object["name"].toString();
  QString source = object["source"].toString();
  QString description = object["description"].toString();
  uint64_t id = object["id"].toVariant().toLongLong();
  uint64_t parentId = object["parentId"].toVariant().toLongLong();
  return Asset({name, source, description, id, parentId});
}

TimeSeries TimeSeries::fromJSON(const QJsonObject &object) {
  QString name = object["name"].toString();
  QString unit = object["unit"].toString();
  uint64_t assetId = object["assetId"].toVariant().toLongLong();
  bool isStep = object["isStep"].toBool();
  QString description = object["description"].toString();
  QString source = object["source"].toString();
  uint64_t id = object["id"].toVariant().toLongLong();

  return TimeSeries({name, unit, assetId, isStep, description, source, id});
}

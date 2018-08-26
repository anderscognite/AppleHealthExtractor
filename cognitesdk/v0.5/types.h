#ifndef TYPES_H
#define TYPES_H
#include <QJsonObject>
#include <QString>

struct Asset {
  static Asset fromJSON(const QJsonObject &object);
  QString name;
  QString source;
  QString description;
  uint64_t id;
  uint64_t parentid;
};

struct TimeSeries {
  static TimeSeries fromJSON(const QJsonObject &object);
  QString name;
  QString unit;
  uint64_t assetId;
  bool isStep;
  QString description;
  QString source;
  uint64_t id;
};

struct DataPoint {
  uint64_t timestamp;
  double value;
};

#endif  // TYPES_H

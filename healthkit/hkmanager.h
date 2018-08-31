#ifndef HEALTHMANAGER_H
#define HEALTHMANAGER_H

#include <QDateTime>
#include <QElapsedTimer>
#include <QObject>
#include <QPair>
#include <QQmlEngine>
#include <QVector>

#include "cognitesdk/v0.5/types.h"

#include <functional>

class HKManager : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString status READ status WRITE setStatus NOTIFY statusChanged)
  Q_PROPERTY(
      float progress READ progress WRITE setProgress NOTIFY progressChanged)
 private:
  void *m_healthStore = nullptr;
  QVector<DataPoint> m_data;
  QElapsedTimer m_timer;
  float m_progress = 0.0;
  QString m_status;

 public:
  enum DataType {
    HeartRate = 0,
    StepCount = 1,
    DistanceWalkingRunning = 2,
    DistanceCycling = 3,
    BasalEnergyBurned = 4,
    ActiveEnergyBurned = 5,
    FlightsClimbed = 6,
    AppleExerciseTime = 7
  };
  Q_ENUMS(DataType)

  enum EnStyle { STYLE_RADIAL, STYLE_ENVELOPE, STYLE_FILLED };
  Q_ENUMS(EnStyle)

  HKManager();
  void setStatusMessage(QString message, uint64_t count, uint64_t maxCount);
  void requestAuthorization();
  void requestData(bool allData, int daysAgo, DataType dataType);
  void requestData2(bool allData, int daysAgo, DataType dataType);
  const QVector<DataPoint> &getData() { return m_data; };
  float progress() const;
  QString status() const;
  void convertData(void *data, void *unit, QVector<DataPoint> &array);
 public slots:
  void setProgress(float progress);
  void setStatus(QString status);

 signals:
  void dataReady();
  void progressChanged(float progress);
  void statusChanged(QString status);
};

#endif  // HEALTHMANAGER_H

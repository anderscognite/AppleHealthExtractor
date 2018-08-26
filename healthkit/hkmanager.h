#ifndef HEALTHMANAGER_H
#define HEALTHMANAGER_H

#include <QDateTime>
#include <QElapsedTimer>
#include <QObject>
#include <QPair>
#include <QVector>
#include "cognitesdk/v0.5/types.h"

class HKManager : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString status READ status WRITE setStatus NOTIFY statusChanged)
  Q_PROPERTY(
      float progress READ progress WRITE setProgress NOTIFY progressChanged)
 private:
  void *m_healthStore = nullptr;
  QVector<DataPoint> m_heartRate;
  QElapsedTimer m_timer;
  float m_progress = 0.0;
  QString m_status;

 public:
  HKManager();
  void setStatusMessage(QString message, uint64_t count, uint64_t maxCount);
  void requestAuthorization();
  void requestHeartRate(int daysAgo);
  const QVector<DataPoint> &getHeartRate() { return m_heartRate; };
  float progress() const;
  QString status() const;

 public slots:
  void setProgress(float progress);
  void setStatus(QString status);

 signals:
  void heartRateReady();
  void progressChanged(float progress);
  void statusChanged(QString status);
};

#endif  // HEALTHMANAGER_H

#ifndef HEALTHMANAGER_H
#define HEALTHMANAGER_H

#include <QDateTime>
#include <QElapsedTimer>
#include <QObject>
#include <QPair>
#include <QVector>

class HKManager : public QObject {
  Q_OBJECT
  void *m_healthStore = nullptr;
  QVector<QPair<QDateTime, float>> m_heartRate;
  QElapsedTimer m_timer;

 public:
  HKManager();
  Q_INVOKABLE void requestAuthorization();
  Q_INVOKABLE void getHeartRate();
  bool readHeartRate() const;
};

#endif  // HEALTHMANAGER_H

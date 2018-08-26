#ifndef DATAHANDLER_H
#define DATAHANDLER_H

#include <QObject>
#include "cognitesdk/v0.5/cognitesdk.h"
#include "cognitesdk/v0.5/types.h"
#include "healthkit/hkmanager.h"

class DataHandler : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool busy READ busy WRITE setBusy NOTIFY busyChanged)
  Q_PROPERTY(CogniteSDK* sdk READ sdk WRITE setSdk NOTIFY sdkChanged)
  Q_PROPERTY(QString status READ status WRITE setStatus NOTIFY statusChanged)
  Q_PROPERTY(HKManager* hkManager READ hkManager WRITE setHkManager NOTIFY
                 hkManagerChanged)

 private:
  CogniteSDK* m_sdk;
  HKManager* m_hkManager;
  bool m_busy = false;
  QString m_status;

 public:
  explicit DataHandler(QObject* parent = nullptr);
  CogniteSDK* sdk() const;
  HKManager* hkManager() const;
  Q_INVOKABLE void syncHeartRate(int daysAgo);
  bool busy() const;
  QString status() const;

 signals:
  void sdkChanged(CogniteSDK* sdk);
  void hkManagerChanged(HKManager* hkManager);
  void busyChanged(bool busy);
  void statusChanged(QString status);

 public slots:
  void setSdk(CogniteSDK* sdk);
  void setHkManager(HKManager* hkManager);
  void uploadHeartRate();
  void setBusy(bool busy);
  void setStatus(QString status);
};

#endif  // DATAHANDLER_H

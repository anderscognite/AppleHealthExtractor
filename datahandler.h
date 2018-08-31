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
  HKManager::DataType m_currentDataType;
  bool m_busy = false;
  QString m_status;

  CogniteSDK* sdk() const;
  HKManager* hkManager() const;

 public:
  explicit DataHandler(QObject* parent = nullptr);
  Q_INVOKABLE void sync(bool allData, int daysAgo, QString dataType);
  bool busy() const;
  QString status() const;

 signals:
  void sdkChanged(CogniteSDK* sdk);
  void hkManagerChanged(HKManager* hkManager);
  void busyChanged(bool busy);
  void statusChanged(QString status);

 public slots:
  void uploadData();
  void setSdk(CogniteSDK* sdk);
  void setHkManager(HKManager* hkManager);
  void setBusy(bool busy);
  void setStatus(QString status);
  void printData();
};

#endif  // DATAHANDLER_H

#ifndef DATAHANDLER_H
#define DATAHANDLER_H

#include <QObject>
#include <QStringList>
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
  Q_PROPERTY(float syncProgress READ syncProgress WRITE setSyncProgress NOTIFY
                 syncProgressChanged)

 private:
  QStringList m_syncList;
  CogniteSDK* m_sdk;
  HKManager* m_hkManager;
  HKManager::DataType m_currentDataType;
  bool m_busy = false;
  bool m_allData = false;
  int m_daysAgo = 0;
  int m_uploadsLeft = 0;
  int m_numSyncs = 0;
  QString m_status;

  bool popSyncList();
  CogniteSDK* sdk() const;
  HKManager* hkManager() const;

  float m_syncProgress;

 public:
  explicit DataHandler(QObject* parent = nullptr);
  Q_INVOKABLE void sync(bool allData, int daysAgo, QStringList syncList);
  bool busy() const;
  QString status() const;

  float syncProgress() const;

 signals:
  void sdkChanged(CogniteSDK* sdk);
  void hkManagerChanged(HKManager* hkManager);
  void busyChanged(bool busy);
  void statusChanged(QString status);

  void syncProgressChanged(float syncProgress);

 public slots:
  void uploadData();
  void setSdk(CogniteSDK* sdk);
  void setHkManager(HKManager* hkManager);
  void setBusy(bool busy);
  void setStatus(QString status);
  void setSyncProgress(float syncProgress);
};

#endif  // DATAHANDLER_H

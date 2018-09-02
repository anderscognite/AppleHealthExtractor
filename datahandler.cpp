#include "datahandler.h"
#include "healthkit/hkmanager.h"

#include <QDebug>
#include <QThread>

QMap<HKManager::DataType, QString> nameMap;
QMap<QString, HKManager::DataType> inverseNameMap;

DataHandler::DataHandler(QObject *parent) : QObject(parent) {
  nameMap.insert(HKManager::HeartRate, "Heart Rate");
  nameMap.insert(HKManager::StepCount, "Steps");
  nameMap.insert(HKManager::DistanceWalkingRunning,
                 "Walking and running distance");
  nameMap.insert(HKManager::DistanceCycling, "Cycling distance");
  nameMap.insert(HKManager::BasalEnergyBurned, "Resting energy");
  nameMap.insert(HKManager::ActiveEnergyBurned, "Active energy");
  nameMap.insert(HKManager::FlightsClimbed, "Flights climbed");
  nameMap.insert(HKManager::AppleExerciseTime, "Exercise");

  for (auto key : nameMap.keys()) {
    inverseNameMap[nameMap[key]] = key;
  }
}

CogniteSDK *DataHandler::sdk() const { return m_sdk; }

HKManager *DataHandler::hkManager() const { return m_hkManager; }

bool DataHandler::popSyncList() {
  if (m_syncList.size() == 0) {
    m_currentDataType = HKManager::None;
    return false;
  }

  m_currentDataType = inverseNameMap[m_syncList.back()];
  m_syncList.pop_back();
  return true;
}

void DataHandler::sync(bool allData, int daysAgo, QStringList syncList) {
  setBusy(true);
  m_syncList = syncList;
  m_allData = allData;
  m_daysAgo = daysAgo;
  m_currentDataType = inverseNameMap[syncList.front()];
  m_numSyncs = syncList.size();
  setSyncProgress(0.0);

  if (popSyncList()) {
    m_hkManager->requestData(m_allData, m_daysAgo, m_currentDataType);
  }
}

bool DataHandler::busy() const { return m_busy; }

QString DataHandler::status() const { return m_status; }

float DataHandler::syncProgress() const { return m_syncProgress; }

void DataHandler::setSdk(CogniteSDK *sdk) {
  if (m_sdk == sdk) return;

  m_sdk = sdk;
  emit sdkChanged(m_sdk);
}

void DataHandler::setHkManager(HKManager *hkManager) {
  if (m_hkManager == hkManager) return;

  m_hkManager = hkManager;
  if (m_hkManager) {
    connect(m_hkManager, &HKManager::dataReady, this, &DataHandler::uploadData);
  }
  emit hkManagerChanged(m_hkManager);
}

void DataHandler::setBusy(bool busy) {
  qDebug() << "Setting data handler busy: " << busy;
  if (m_busy == busy) return;

  m_busy = busy;
  emit busyChanged(m_busy);
}

void DataHandler::setStatus(QString status) {
  if (m_status == status) return;

  m_status = status;
  emit statusChanged(m_status);
}

void DataHandler::setSyncProgress(float syncProgress) {
  m_syncProgress = syncProgress;
  emit syncProgressChanged(m_syncProgress);
}

void DataHandler::uploadData() {
  setStatus("Uploading data to CDP...");
  uint64_t chunkSize = 100000;
  const auto &data = m_hkManager->getData();
  uint64_t numChunks = data.size() / chunkSize + 1;

  qDebug() << "Looping through...";
  m_uploadsLeft = numChunks;
  for (uint64_t i = 0; i < numChunks; i++) {
    uint64_t start = i * chunkSize;
    uint64_t stop = (i + 1) * chunkSize;
    qDebug() << "Chunk from" << start << " to " << stop;
    stop = std::min(stop, uint64_t(data.size()));  // cap to size of data
    QVector<DataPoint> points;
    points.reserve(stop - start);
    for (uint64_t j = start; j < stop; j++) {
      points.push_back(data[j]);
    }
    qDebug() << "Starting with start " << start << " and stop " << stop;
    m_sdk->createDataPointsInTimeSeries(
        nameMap[m_currentDataType], points, [this](bool error) {
          qDebug() << "Created data points with error: " << error;
          if (error) {
            setStatus("Error uploading");
          } else {
            setStatus("Done");
          }
          if (--m_uploadsLeft == 0) {
            int syncsLeft = m_syncList.size();
            float progress = 1.0 - float(syncsLeft) / m_numSyncs;
            setSyncProgress(progress);

            if (popSyncList()) {
              m_hkManager->requestData(m_allData, m_daysAgo, m_currentDataType);
            } else {
              setBusy(false);
            }
          }
        });
    qDebug() << "Finished with start " << start << " and stop " << stop;
  }
}

#include "datahandler.h"
#include <QDebug>

DataHandler::DataHandler(QObject *parent) : QObject(parent) {}

CogniteSDK *DataHandler::sdk() const { return m_sdk; }

HKManager *DataHandler::hkManager() const { return m_hkManager; }

void DataHandler::syncHeartRate(int daysAgo) {
  setBusy(true);
  m_hkManager->requestHeartRate(daysAgo);
}

void DataHandler::syncSteps(int daysAgo) {
  setBusy(true);
  m_hkManager->requestSteps(daysAgo);
}

bool DataHandler::busy() const { return m_busy; }

QString DataHandler::status() const { return m_status; }

void DataHandler::setSdk(CogniteSDK *sdk) {
  if (m_sdk == sdk) return;

  m_sdk = sdk;
  emit sdkChanged(m_sdk);
}

void DataHandler::setHkManager(HKManager *hkManager) {
  if (m_hkManager == hkManager) return;

  m_hkManager = hkManager;
  emit hkManagerChanged(m_hkManager);
  if (m_hkManager) {
    QObject::connect(m_hkManager, &HKManager::heartRateReady, this,
                     &DataHandler::uploadHeartRate);
    QObject::connect(m_hkManager, &HKManager::stepsReady, this,
                     &DataHandler::uploadSteps);
  }
}

void DataHandler::setBusy(bool busy) {
  if (m_busy == busy) return;

  m_busy = busy;
  emit busyChanged(m_busy);
}

void DataHandler::setStatus(QString status) {
  if (m_status == status) return;

  m_status = status;
  emit statusChanged(m_status);
}

void DataHandler::uploadSteps() {
  setStatus("Uploading data to CDP...");

  uint64_t chunkSize = 25000;
  const auto &steps = m_hkManager->getSteps();
  uint64_t numChunks = steps.size() / chunkSize;
  numChunks = std::max(static_cast<uint64_t>(1),
                       numChunks);  // At least one chunk

  for (uint64_t i = 0; i < numChunks; i++) {
    uint64_t start = i * chunkSize;
    uint64_t stop = (i + 1) * chunkSize;
    stop = std::min(stop, uint64_t(steps.size()));  // cap to size of data
    QVector<DataPoint> points;
    points.reserve(stop - start);
    for (uint64_t j = start; j < stop; j++) {
      points.push_back(steps[j]);
    }

    m_sdk->createDataPointsInTimeSeries("Steps", points, [this](bool error) {
      qDebug() << "Created data points with error: " << error;
      if (error) {
        setStatus("Error uploading");
      } else {
        setStatus("Done");
      }
      setBusy(false);
    });
  }
}

void DataHandler::uploadHeartRate() {
  setStatus("Uploading data to CDP...");

  uint64_t chunkSize = 25000;
  const auto &heartRateData = m_hkManager->getHeartRate();
  uint64_t numChunks = heartRateData.size() / chunkSize;
  numChunks = std::max(static_cast<uint64_t>(1),
                       numChunks);  // At least one chunk

  for (uint64_t i = 0; i < numChunks; i++) {
    uint64_t start = i * chunkSize;
    uint64_t stop = (i + 1) * chunkSize;
    stop =
        std::min(stop, uint64_t(heartRateData.size()));  // cap to size of data
    QVector<DataPoint> points;
    points.reserve(stop - start);
    for (uint64_t j = start; j < stop; j++) {
      points.push_back(heartRateData[j]);
    }

    m_sdk->createDataPointsInTimeSeries(
        "Heart Rate", points, [this](bool error) {
          qDebug() << "Created data points with error: " << error;
          if (error) {
            setStatus("Error uploading");
          } else {
            setStatus("Done");
          }
          setBusy(false);
        });
  }
}

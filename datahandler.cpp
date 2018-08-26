#include "datahandler.h"
#include <QDebug>

DataHandler::DataHandler(QObject *parent) : QObject(parent) {}

CogniteSDK *DataHandler::sdk() const { return m_sdk; }

HKManager *DataHandler::hkManager() const { return m_hkManager; }

void DataHandler::syncHeartRate(int daysAgo) {
  setBusy(true);
  m_hkManager->requestHeartRate(daysAgo);
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

void DataHandler::uploadHeartRate() {
  m_sdk->getTimeSeriesWithName(
      "Heart Rate", [&](QVector<TimeSeries> timeSeries, bool error) {
        if (timeSeries.size() == 0 || error) {
          setStatus("Error, could not find time series");
          return;
        }

        auto heartRateData = m_hkManager->getHeartRate();
        qDebug() << "Got " << heartRateData.size() << " HR";
        setBusy(false);
      });
}

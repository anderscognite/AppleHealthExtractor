#include "hkmanager.h"
#import <HealthKit/HealthKit.h>
#include <QDebug>
#include <QThread>

HKManager::HKManager() { m_healthStore = [[HKHealthStore alloc] init]; }

void HKManager::requestAuthorization() {
  NSMutableArray *readTypes = [NSMutableArray new];
  [readTypes addObject:[HKObjectType quantityTypeForIdentifier:HKQuantityTypeIdentifierHeartRate]];
  [readTypes addObject:[HKObjectType quantityTypeForIdentifier:HKQuantityTypeIdentifierStepCount]];
  [readTypes
      addObject:[HKObjectType
                    quantityTypeForIdentifier:HKQuantityTypeIdentifierDistanceWalkingRunning]];
  [readTypes
      addObject:[HKObjectType quantityTypeForIdentifier:HKQuantityTypeIdentifierDistanceCycling]];
  [readTypes
      addObject:[HKObjectType quantityTypeForIdentifier:HKQuantityTypeIdentifierBasalEnergyBurned]];
  [readTypes addObject:[HKObjectType
                           quantityTypeForIdentifier:HKQuantityTypeIdentifierActiveEnergyBurned]];
  [readTypes
      addObject:[HKObjectType quantityTypeForIdentifier:HKQuantityTypeIdentifierFlightsClimbed]];
  //  [readTypes addObject:[HKObjectType
  //  quantityTypeForIdentifier:HKCategoryTypeIdentifierAppleStandHour]];
  [readTypes
      addObject:[HKObjectType quantityTypeForIdentifier:HKQuantityTypeIdentifierAppleExerciseTime]];

  if ([HKHealthStore isHealthDataAvailable] == NO) {
    // If our device doesn't support HealthKit -> return.
    NSLog(@"Does not have HK stuff");
    return;
  }

  NSArray *writeTypes = @[];

  [m_healthStore requestAuthorizationToShareTypes:[NSSet setWithArray:writeTypes]
                                        readTypes:[NSSet setWithArray:readTypes]
                                       completion:^(BOOL success, NSError *error) {
                                         NSLog(@"Done");
                                       }];
}

void HKManager::requestHeartRate(int daysAgo) {
  setProgress(0.0);
  m_heartRate.clear();
  NSDate *now = [NSDate date];
  NSCalendar *calendar = [NSCalendar autoupdatingCurrentCalendar];
  NSDate *t0 = [NSDate dateWithTimeIntervalSince1970:0];

  if (daysAgo > 0) {
    NSDate *sevenDaysAgo = [now dateByAddingTimeInterval:-daysAgo * 24 * 60 * 60];
    t0 = sevenDaysAgo;
  }

  NSPredicate *predicate = [HKQuery predicateForSamplesWithStartDate:t0
                                                             endDate:now
                                                             options:HKQueryOptionStrictStartDate];
  HKSampleQuery *sampleQuery = [[HKSampleQuery alloc]
      initWithSampleType:[HKObjectType quantityTypeForIdentifier:HKQuantityTypeIdentifierHeartRate]
               predicate:predicate
                   limit:0
         sortDescriptors:nil
          resultsHandler:^(HKSampleQuery *_Nonnull query,
                           NSArray<__kindof HKSample *> *_Nullable results,
                           NSError *_Nullable error) {
            HKUnit *unit = [[HKUnit countUnit] unitDividedByUnit:[HKUnit minuteUnit]];
            convertData(static_cast<void *>(results), static_cast<void *>(unit), m_heartRate);
            emit heartRateReady();
          }];

  m_timer.start();
  setStatus("Executing query...");
  [m_healthStore executeQuery:sampleQuery];
}

void HKManager::requestSteps(int daysAgo) {
  setProgress(0.0);
  m_steps.clear();
  NSDate *now = [NSDate date];
  NSCalendar *calendar = [NSCalendar autoupdatingCurrentCalendar];
  NSDate *t0 = [NSDate dateWithTimeIntervalSince1970:0];

  if (daysAgo > 0) {
    NSDate *sevenDaysAgo = [now dateByAddingTimeInterval:-daysAgo * 24 * 60 * 60];
    t0 = sevenDaysAgo;
  }

  NSPredicate *predicate = [HKQuery predicateForSamplesWithStartDate:t0
                                                             endDate:now
                                                             options:HKQueryOptionStrictStartDate];
  HKSampleQuery *sampleQuery = [[HKSampleQuery alloc]
      initWithSampleType:[HKObjectType quantityTypeForIdentifier:HKQuantityTypeIdentifierStepCount]
               predicate:predicate
                   limit:0
         sortDescriptors:nil
          resultsHandler:^(HKSampleQuery *_Nonnull query,
                           NSArray<__kindof HKSample *> *_Nullable results,
                           NSError *_Nullable error) {
            HKUnit *unit = [HKUnit countUnit];
            convertData(static_cast<void *>(results), static_cast<void *>(unit), m_steps);
            emit stepsReady();
          }];

  m_timer.start();
  setStatus("Executing query...");
  [m_healthStore executeQuery:sampleQuery];
}

void HKManager::setStatusMessage(QString message, uint64_t count, uint64_t maxCount) {
  message = QString("%1 (%2/%3)").arg(message).arg(count).arg(maxCount);
  setStatus(message);
}

float HKManager::progress() const { return m_progress; }

void HKManager::setProgress(float progress) {
  m_progress = progress;
  emit progressChanged(m_progress);
}

QString HKManager::status() const { return m_status; }

void HKManager::convertData(void *results_, void *unit_, QVector<DataPoint> &array) {
  setStatus("Converting data...");
  auto results = static_cast<NSArray<__kindof HKSample *> *>(results_);
  auto unit = static_cast<HKUnit *>(unit_);

  uint64_t numSamples = results.count;
  uint64_t count = 0;
  qDebug() << "Converting " << numSamples << " samples...";
  for (HKQuantitySample *sample in results) {
    if (++count % 100 == 0) {
      double percentage = double(count) / numSamples;
      setProgress(percentage);
      setStatusMessage("Converting data...", count, numSamples);
    }

    NSDate *date = sample.startDate;
    uint64_t unixTimestamp =
        static_cast<uint64_t>(date.timeIntervalSince1970) * 1000;  // CDP wants ms
    double value = [sample.quantity doubleValueForUnit:unit];

    array.push_back({unixTimestamp, value});
  }

  setProgress(0.0);
  setStatus("");
}

void HKManager::setStatus(QString status) {
  if (m_status == status) return;

  m_status = status;
  emit statusChanged(m_status);
}

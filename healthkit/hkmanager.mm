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
  qDebug() << "Predicate ready. I am thread " << QThread::currentThread();
  HKSampleQuery *sampleQuery = [[HKSampleQuery alloc]
      initWithSampleType:[HKObjectType quantityTypeForIdentifier:HKQuantityTypeIdentifierHeartRate]
               predicate:predicate
                   limit:0
         sortDescriptors:nil
          resultsHandler:^(HKSampleQuery *_Nonnull query,
                           NSArray<__kindof HKSample *> *_Nullable results,
                           NSError *_Nullable error) {
            qDebug() << "Query finished after " << m_timer.restart() / 1000.
                     << " seconds, looping through " << results.count << " things";
            setStatus("Converting data...");

            uint64_t numSamples = results.count;
            uint64_t count = 0;
            for (HKQuantitySample *sample in results) {
              if (++count % 100 == 0) {
                double percentage = double(count) / numSamples;
                setProgress(percentage);
                setStatusMessage("Converting data...", count, numSamples);
              }
              HKUnit *unit = [[HKUnit countUnit] unitDividedByUnit:[HKUnit minuteUnit]];
              NSDate *date = sample.startDate;
              uint64_t unixTimestamp =
                  static_cast<uint64_t>(date.timeIntervalSince1970) * 1000;  // CDP wants ms
              double value = [sample.quantity doubleValueForUnit:unit];

              m_heartRate.push_back({unixTimestamp, value});
            }

            qDebug() << "Done after " << m_timer.elapsed() / 1000. << " seconds.";
            setProgress(0.0);
            setStatus("");
            emit heartRateReady();
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

void HKManager::setStatus(QString status) {
  if (m_status == status) return;

  m_status = status;
  emit statusChanged(m_status);
}

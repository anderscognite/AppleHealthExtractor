#include "hkmanager.h"
#import <HealthKit/HealthKit.h>
#include <QDebug>
#include <QDir>
#include <QMap>
#include <QStandardPaths>
#include <QThread>
// QMap<HKManager::DataType, HKQuantityTypeIdentifier> typeMap;
// QMap<HKManager::DataType, HKUnit *> unitMap;

NSDictionary *typeMap = [[NSDictionary alloc]
    initWithObjectsAndKeys:HKQuantityTypeIdentifierHeartRate, @(HKManager::HeartRate),
                           HKQuantityTypeIdentifierStepCount, @(HKManager::StepCount),
                           HKQuantityTypeIdentifierDistanceWalkingRunning,
                           @(HKManager::DistanceWalkingRunning),
                           HKQuantityTypeIdentifierDistanceCycling, @(HKManager::DistanceCycling),
                           HKQuantityTypeIdentifierBasalEnergyBurned,
                           @(HKManager::BasalEnergyBurned),
                           HKQuantityTypeIdentifierActiveEnergyBurned,
                           @(HKManager::ActiveEnergyBurned), HKQuantityTypeIdentifierFlightsClimbed,
                           @(HKManager::FlightsClimbed), HKQuantityTypeIdentifierAppleExerciseTime,
                           @(HKManager::AppleExerciseTime), nil];
NSDictionary *unitMap = [[NSDictionary alloc]
    initWithObjectsAndKeys:[[HKUnit countUnit] unitDividedByUnit:[HKUnit minuteUnit]],
                           @(HKManager::HeartRate), [HKUnit countUnit], @(HKManager::StepCount),
                           [HKUnit meterUnitWithMetricPrefix:HKMetricPrefixKilo],
                           @(HKManager::DistanceWalkingRunning),
                           [HKUnit meterUnitWithMetricPrefix:HKMetricPrefixKilo],
                           @(HKManager::DistanceCycling), [HKUnit kilocalorieUnit],
                           @(HKManager::BasalEnergyBurned), [HKUnit kilocalorieUnit],
                           @(HKManager::ActiveEnergyBurned), [HKUnit countUnit],
                           @(HKManager::FlightsClimbed), [HKUnit minuteUnit],
                           @(HKManager::AppleExerciseTime), nil];

NSArray *readTypes = [[NSArray alloc] initWithArray:[typeMap allValues]];

HKManager::HKManager() {
  m_healthStore = [[HKHealthStore alloc] init];

  NSLog(@"I have type map with count %d", typeMap.count);
  NSLog(@"Also, this one type is %@", typeMap[@(HKManager::HeartRate)]);
}

void HKManager::requestAuthorization() {
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

void HKManager::requestData2(bool allData, int daysAgo, HKManager::DataType dataType) {
  setProgress(0.0);
  m_data.clear();

  NSCalendar *calendar = [NSCalendar currentCalendar];
  NSDate *anchorDate =
      [calendar dateByAddingUnit:NSCalendarUnitYear value:-3 toDate:[NSDate date] options:0];

  NSDateComponents *interval = [[NSDateComponents alloc] init];
  interval.minute = 1;

  HKQuantityType *quantityType = [HKObjectType quantityTypeForIdentifier:typeMap[@(dataType)]];

  // Create the query
  HKStatisticsCollectionQuery *query =
      [[HKStatisticsCollectionQuery alloc] initWithQuantityType:quantityType
                                        quantitySamplePredicate:nil
                                                        options:HKStatisticsOptionCumulativeSum
                                                     anchorDate:anchorDate
                                             intervalComponents:interval];

  // Set the results handler
  query.initialResultsHandler =
      ^(HKStatisticsCollectionQuery *query, HKStatisticsCollection *results, NSError *error) {
        if (error) {
          // Perform proper error handling here
          NSLog(@"*** An error occurred while calculating the statistics: %@ ***",
                error.localizedDescription);
          abort();
        }
        NSLog(@"Did do query");

        NSDate *endDate = [NSDate date];

        // Plot the weekly step counts over the past 3 months
        [results enumerateStatisticsFromDate:anchorDate
                                      toDate:endDate
                                   withBlock:^(HKStatistics *result, BOOL *stop) {
                                     HKQuantity *quantity = result.sumQuantity;
                                     if (quantity) {
                                       NSDate *date = result.startDate;
                                       uint64_t unixTimestamp =
                                           static_cast<uint64_t>(date.timeIntervalSince1970) *
                                           1000;  // CDP wants ms
                                       double value =
                                           [quantity doubleValueForUnit:unitMap[@(dataType)]];
                                       bool skipValue = m_data.size() > 0 &&
                                                        fabs(value - m_data.last().value) < 1e-4;
                                       if (!skipValue) {
                                         m_data.push_back({unixTimestamp, value});
                                       }
                                     }
                                   }];
        emit dataReady();
      };

  [m_healthStore executeQuery:query];
  NSLog(@"Executing...");
}

void HKManager::requestData(bool allData, int daysAgo, HKManager::DataType dataType) {
  setProgress(0.0);
  m_data.clear();
  NSDate *t0 = [NSDate dateWithTimeIntervalSince1970:0];

  if (!allData) {
    t0 = [[NSDate date] dateByAddingTimeInterval:-daysAgo * 24 * 60 * 60];
    NSLog(@"Will do days ago: %@", t0);
  }

  NSPredicate *predicate = [HKQuery predicateForSamplesWithStartDate:t0
                                                             endDate:[NSDate date]
                                                             options:HKQueryOptionStrictStartDate];
  NSLog(@"Will find quantity with type: %@", typeMap[@(dataType)]);
  NSLog(@"Will find quantity with t0: %@", t0);
  HKSampleQuery *sampleQuery = [[HKSampleQuery alloc]
      initWithSampleType:[HKObjectType quantityTypeForIdentifier:typeMap[@(dataType)]]
               predicate:predicate
                   limit:HKObjectQueryNoLimit
         sortDescriptors:nil
          resultsHandler:^(HKSampleQuery *_Nonnull query,
                           NSArray<__kindof HKSample *> *_Nullable results,
                           NSError *_Nullable error) {
            convertData(static_cast<void *>(results), static_cast<void *>(unitMap[@(dataType)]),
                        m_data);
            emit dataReady();
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

  for (HKQuantitySample *sample in results) {
    if (++count % 100 == 0) {
      double percentage = double(count) / numSamples;
      setProgress(percentage);
      setStatusMessage("Converting data...", count, numSamples);
    }

    NSDate *date = sample.startDate;
    NSDate *endDate = sample.endDate;
    uint64_t unixTimestamp =
        static_cast<uint64_t>(date.timeIntervalSince1970) * 1000;  // CDP wants ms
    double value = [sample.quantity doubleValueForUnit:unit];

    array.push_back({unixTimestamp, value});
  }
  qDebug() << "Done with converting";
  setProgress(0.0);
  setStatus("");
}

void HKManager::setStatus(QString status) {
  if (m_status == status) return;

  m_status = status;
  emit statusChanged(m_status);
}

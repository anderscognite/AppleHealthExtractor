#include "hkmanager.h"
#import <HealthKit/HealthKit.h>
#include <QDebug>

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
//  [readTypes addObject:[HKObjectType quantityTypeForIdentifier:HKCategoryTypeIdentifierAppleStandHour]];
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

void HKManager::getHeartRate() {
  NSDate *now = [NSDate date];
  NSCalendar *calendar = [NSCalendar autoupdatingCurrentCalendar];
  NSDate *beginOfTime = [NSDate dateWithTimeIntervalSince1970:0];

  NSPredicate *predicate = [HKQuery predicateForSamplesWithStartDate:beginOfTime
                                                             endDate:now
                                                             options:HKQueryOptionStrictStartDate];
  qDebug() << "Predicate ready";
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

            for (HKQuantitySample *sample in results) {
              HKUnit *unit = [[HKUnit countUnit] unitDividedByUnit:[HKUnit minuteUnit]];
              NSDate *date = sample.startDate;
              float unixTimestamp = date.timeIntervalSince1970;
              float value = [sample.quantity doubleValueForUnit:unit];
              QPair<QDateTime, float> point(QDateTime::fromMSecsSinceEpoch(1000 * unixTimestamp),
                                            value);
              m_heartRate.push_back(point);
              qDebug() << "Heart rate: " << value;
            }

            qDebug() << "Done after " << m_timer.elapsed() / 1000. << " seconds.";
            //        for(auto p : m_heartRate) {
            //            qDebug() << p;
            //        }
          }];

  qDebug() << "Executing query";
  m_timer.start();
  [m_healthStore executeQuery:sampleQuery];
}

import QtQuick 2.11
import QtQuick.Window 2.11
import Cognite 1.0
import QtQuick.Controls 2.2
/*
    HeartRate = 0,
    StepCount = 1,
    DistanceWalkingRunning = 2,
    DistanceCycling = 3,
    BasalEnergyBurned = 4,
    ActiveEnergyBurned = 5,
    FlightsClimbed = 6,
    AppleExerciseTime = 7
*/
Window {
    CogniteSDK {
        id: sdk
    }
    HKManager {
        id: hkManager
        Component.onCompleted: {
            console.log('HKManager.HeartRate: ', HKManager.HeartRate)
            console.log('HKManager.StepCount: ', HKManager.StepCount)
        }
    }
    DataHandler {
        id: dataHandler
        sdk: sdk
        hkManager: hkManager
    }

    Column {
        spacing: 5
        Button {
            text: "Authorize"
            enabled: !dataHandler.busy
            onClicked: {
                hkManager.requestAuthorization()
            }
        }
        Row {
            Label {
                text: "Period: "
            }

            Slider {
                id: slider
                property string buttonText: "Last 7 days"
                property int daysAgo: 7
                from: 1
                to:5
                stepSize: 1
                value: 2

                onValueChanged: {
                    if (value == 1) {
                        buttonText = "Last 24 hours"
                        daysAgo = 1
                    } else if (value == 2) {
                        buttonText = "Last 7 days"
                        daysAgo = 7
                    } else if (value == 3) {
                        buttonText = "Last 31 days"
                        daysAgo = 31
                    } else if (value == 4) {
                        buttonText = "Last 6 months"
                        daysAgo = 183
                    } else if (value == 5) {
                        buttonText = "Last year"
                        daysAgo = 365
                    }
                }
            }
        }

        HKButtons {
            buttonText: "Sync heart rate"
            dataType: "Heart rate"
        }

        HKButtons {
            buttonText: "Sync steps"
            dataType: "Steps"
        }

        HKButtons {
            buttonText: "Sync walking distance"
            dataType: "Walking and running distance"
        }

        HKButtons {
            buttonText: "Sync cycling distance"
            dataType: "Cycling distance"
        }

        HKButtons {
            buttonText: "Sync resting energy"
            dataType: "Resting energy"
        }

        HKButtons {
            buttonText: "Sync active energy"
            dataType: "Active energy"
        }

        HKButtons {
            buttonText: "Sync flights"
            dataType: "Flights climbed"
        }

        HKButtons {
            buttonText: "Sync exercise time"
            dataType: "Exercise"
        }

        Label {
            text: {
                if (hkManager.status !== "") {
                    return hkManager.status;
                }
                if (dataHandler.status !== "") {
                    return dataHandler.status;
                }
                return ""
            }
            visible: dataHandler.busy
        }

        ProgressBar {
            value: Math.max(hkManager.progress, sdk.progress)
            visible: dataHandler.busy
        }
    }


    visible: true
    width: 640
    height: 480
    title: qsTr("Hello World")
}

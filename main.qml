import QtQuick 2.11
import QtQuick.Window 2.11
import Cognite 1.0
import QtQuick.Controls 2.2

Window {
    CogniteSDK {
        id: sdk
    }
    HKManager {
        id: hkManager
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

        Row {
            spacing: 5
            Button {
                text: "Sync heart rate"
                enabled: !dataHandler.busy
                onClicked: {
                    dataHandler.syncHeartRate(true, 0)
                }
            }
            Button {
                text: slider.buttonText
                enabled: !dataHandler.busy
                onClicked: {
                    dataHandler.syncHeartRate(false, slider.daysAgo)
                }
            }
        }
        Row {
            spacing: 5
            Button {
                text: "Sync steps"
                enabled: !dataHandler.busy
                onClicked: {
                    dataHandler.syncSteps(true, 0)
                }
            }
            Button {
                text: slider.buttonText
                enabled: !dataHandler.busy
                onClicked: {
                    dataHandler.syncSteps(false, slider.daysAgo)
                }
            }
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

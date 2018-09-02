import QtQuick 2.11
import QtQuick.Window 2.11
import Cognite 1.0
import QtQuick.Controls 2.2
//import Qt.labs.settings 1.0

Window {
    id: root
    property bool authorized: false

    function createSyncList() {
        var items = []
        if (heartRate.checked) items.push("Heart Rate");
        if (steps.checked) items.push("Steps");
        if (walkingDistance.checked) items.push("Walking and running distance");
        if (cyclingDistance.checked) items.push("Cycling distance");
        if (restingEnergy.checked) items.push("Resting energy");
        if (activeEnergy.checked) items.push("Active energy");
        if (flightsClimbed.checked) items.push("Flights Climbed");
        return items;
    }

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
                authorized = true
            }
        }

        CheckBox {
            id: heartRate
            text: "Heart Rate"
            checked: true;
            enabled: !dataHandler.busy
        }
        CheckBox {
            id: steps
            text: "Steps"
            checked: true;
            enabled: !dataHandler.busy
        }

        CheckBox {
            id: walkingDistance
            text: "Walking/running distance"
            checked: true;
            enabled: !dataHandler.busy
        }
        CheckBox {
            id: cyclingDistance
            text: "Cycling distance"
            checked: true;
            enabled: !dataHandler.busy
        }

        CheckBox {
            id: restingEnergy
            text: "Resting energy"
            checked: true;
            enabled: !dataHandler.busy
        }

        CheckBox {
            id: activeEnergy
            text: "Active energy"
            checked: true;
            enabled: !dataHandler.busy
        }


        CheckBox {
            id: flightsClimbed
            text: "Flights climbed"
            checked: true;
            enabled: !dataHandler.busy
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
                enabled: !dataHandler.busy

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
            Button {
                text: "Sync"
                enabled: !dataHandler.busy
                onClicked: {
                    dataHandler.sync(true, 0, createSyncList())
                }
            }

            Button {
                text: slider.buttonText
                enabled: !dataHandler.busy
                onClicked: {
                    dataHandler.sync(false, slider.daysAgo, createSyncList());
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
            value: dataHandler.syncProgress
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

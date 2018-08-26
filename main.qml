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
            spacing: 5
            Button {
                text: "Sync heart rate"
                enabled: !dataHandler.busy
                onClicked: {
                    dataHandler.syncHeartRate(0)
                }
            }
            Button {
                text: "Last 7 days"
                enabled: !dataHandler.busy
                onClicked: {
                    dataHandler.syncHeartRate(7)
                }
            }
        }

        Label {
            text: {
                if (hkManager.status !== "") {
                    return hkManager.status;
                }
                if (sdk.status !== "") {
                    return sdk.status;
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

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
    Column {
        Row {
            Button {
                text: "Authorize"
                onClicked: {
                    hkManager.requestAuthorization()
                }
            }
            Button {
                text: "Sync heart rate"
                onClicked: {
                    hkManager.getHeartRate()
                }
            }
        }
    }
//    Button {
//        text: "Request"
//        onClicked: sdk.test()
//    }

    visible: true
    width: 640
    height: 480
    title: qsTr("Hello World")
}

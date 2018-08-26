import QtQuick 2.11
import QtQuick.Window 2.11
import Cognite 1.0
import QtQuick.Controls 2.2

Window {
    CogniteSDK {
        id: sdk
    }
    Button {
        text: "Request"
        onClicked: sdk.test()
    }

    CogniteSDK {
        id: sdk2
    }
    Button {
        x: 50
        text: "Request 2"
        onClicked: sdk2.test()
    }

    visible: true
    width: 640
    height: 480
    title: qsTr("Hello World")
}

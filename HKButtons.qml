import QtQuick 2.9
import QtQuick.Controls 2.2

Row {
    property string buttonText
    property string dataType
    spacing: 5
    Button {
        text: buttonText
        enabled: !dataHandler.busy
        onClicked: {
            dataHandler.sync(true, 0, dataType)
        }
    }
    Button {
        text: slider.buttonText
        enabled: !dataHandler.busy
        onClicked: {
            dataHandler.sync(false, slider.daysAgo, dataType)
        }
    }
}

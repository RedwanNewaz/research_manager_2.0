import QtQuick 2.0

Rectangle {
    property string info: ""
    width: 60
    height: width

    color: "lightblue"
    Text {
        text: info
        anchors.centerIn: parent
    }
}

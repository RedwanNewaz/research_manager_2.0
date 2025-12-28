import QtQuick
import ResearchManager
import QtQuick.Controls


Window {
    width: mainScreen.width
    height: mainScreen.height

    visible: true
    title: "ResearchManager"

    Workspace {
        id: mainScreen
        property string projectName: ""
        property string appCurrentPage: "Workspace.qml"
        anchors.centerIn: parent
    }

}


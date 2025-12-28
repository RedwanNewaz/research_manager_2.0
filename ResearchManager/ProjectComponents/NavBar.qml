import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Column {
    anchors.fill: parent
    ToolBar {
        Row{
            ToolButton {
                id: menuButton
                text: "\u2630"
                font.pixelSize: 24
                height: 40  // Set an explicit height
                onClicked: drawer.open()
                background:Rectangle{
                    color: "transparent"
                }
            }
            // ToolButton {
            //     id: syncButton
            //     text: "\u21BB" // Unicode character for a circular arrow
            //     font.pixelSize: 24
            //     height: 40  // Set an explicit height
            //     onClicked: console.log("sync button clicked")
            //     background:Rectangle{
            //         color: "transparent"
            //     }
            // }
            ToolButton {
                id: syncButton
                Layout.alignment: Qt.AlignBottom | Qt.AlignHCenter
                width: 40
                height: 40
                contentItem: Text {
                    text: "\u21BB"
                    font.pixelSize: 24
                    color: syncButton.state === "clicked" ? "green" : "black"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    color: syncButton.state === "clicked" ? "#a84e7c8f" : "transparent"
                    radius: width * 2
                }
                states: [
                    State {
                        name: "clicked"
                        PropertyChanges { target: syncButton; scale: 0.5 }
                    }
                ]
                transitions: [
                    Transition {
                        from: ""; to: "clicked"; reversible: true
                        ParallelAnimation {
                            NumberAnimation { properties: "scale"; duration: 200; easing.type: Easing.InOutQuad }
                            ColorAnimation { duration: 200 }
                        }
                    }
                ]
                onClicked: {
                    state = "clicked"
                    syncTimer.start()
                    console.log("Sync button clicked")
                    // Add your sync functionality here
                }
                Timer {
                    id: syncTimer
                    interval: 1000 // 1 second
                    onTriggered: syncButton.state = ""
                }
                
            }    
        }
    }

    Drawer {
        id: drawer
        width: 0.11 * parent.width
        height: parent.height
        opacity: 0.65
        background: Rectangle {
            color: "navy"
        }

        ListView {
            anchors.fill: parent
            model: rootModel
            delegate: ItemDelegate {
                text: model.name
                width: parent.width
                contentItem: Label {
                    text: parent.text
                    color: "white"  // Set text color to white for better visibility
                    font: parent.font
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    color: parent.hovered ? Qt.lighter("navy", 1.2) : "transparent"
                }
                onClicked: {
                    const projectName = model.name 
                    const folder_dir = wsModel.message + "/" + projectName
                    console.log("Clicked on:", folder_dir)
                    switchScreen(folder_dir, projectName)                    
                    drawer.close()
                }
            }
        }
    }

    function switchScreen(folder_dir, projectName) {
        fileModel.load_directory(folder_dir)
        taskModel.load_database(projectName, folder_dir)
        // project, root_path
        cdModel.load_database(projectName, folder_dir)
        dlModel.load_database(projectName, folder_dir)
        lnModel.load_database(projectName, folder_dir)
        pcModel.load_database(projectName, folder_dir)
        cmModel.load_database(projectName, folder_dir)
        app.projectName = projectName
    }
}
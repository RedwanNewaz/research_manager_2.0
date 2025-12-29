import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "ProjectComponents"

Item {
    id: rootRecentProjects
    width: Screen.width
    height: Screen.height -70

    Image {
        width: 50
        height: 50
        source: "images/recent-svgrepo-com.svg"
        anchors.centerIn: parent
    }

    // ---------------- LEFT BAR ----------------
        Item {
            id: leftbar
            width: parent.width / 4
            height: parent.height

            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom

            Rectangle {

                id: leftbar_bg
                anchors.fill: parent
                color: "#282828"
                anchors.right : folder_bg.left

                Rectangle{
                    x: 77
                    y: 37
                    width: leftbar.width - 85 //365
                    height: 85
                    color: "#3c3c3c"
                    border.color: "#555555"
                    border.width: 1

                    TextEdit {
                        id: taskTextInput
                        anchors.fill: parent



                        color: "#d6ffffff"
                        // text: qsTr("TypeHere")
                        font.pixelSize: 16
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.styleName: "Italic"
                        property string placeholderText: "Add task here..."
                        selectByMouse: true
                        wrapMode: Text.WordWrap
                        Text {
                            text: taskTextInput.placeholderText
                            font.styleName: "Italic"
                            font.pointSize: 16
                            color: "#d6ffffff"
                            visible: !taskTextInput.text && !taskTextInput.activeFocus
                            anchors.centerIn: parent
                        }
                    }
                }





            RowLayout{
                // y: 115
                // x: 20
                anchors.left:taskListRect.left
                anchors.bottom:taskListRect.top
                anchors.bottomMargin: 10
                anchors.right: taskListRect.right
                width: parent.width - 40
                spacing: 15
                
                RoundButton {
                    id: buttonAdd
                    Layout.preferredWidth: 100
                    Layout.preferredHeight: 36
                    text: qsTr("Add")
                    onClicked: {
                        task.addTask(taskTextInput.text)
                        taskTextInput.text = ""
                    }
                }

                RoundButton {
                    id: buttonEdit
                    Layout.preferredWidth: 100
                    Layout.preferredHeight: 36
                    text: qsTr("Edit")
                    onClicked: noteViewPopup.open()
                }

                RoundButton {
                    id: buttonDelete
                    Layout.preferredWidth: 100
                    Layout.preferredHeight: 36
                    text: qsTr("Delete")
                    onClicked: task.deleteTasks()
                }
            }

            Rectangle
            {
                id: taskListRect

                x: 14
                y: 181
                width: parent.width - 80
                height: parent.height - 180
                color: "#153584e4"
                anchors.right: parent.right

                NoteViewer{
                    id: noteViewPopup
                }

                TaskViewer{
                    id: taskViewer
                }
            }

        }
    }

    // ---------------- RIGHT BAR ----------------
    Rectangle {
        id: rightbar_bg
        width: leftbar.width
        height: parent.height
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        color: "#282828"

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 5

            TabBar {
                id: bar
                Layout.fillWidth: true
                height: 44

                background: Rectangle {
                    anchors.fill: parent
                    color: "#1a1a1a"
                    border.color: "#404040"
                    border.width: 1
                    radius: 8
                }

                TabButton {
                    id: dates
                    text: qsTr("Calendar")
                    implicitWidth: bar.availableWidth / 3
                    implicitHeight: 44

                    background: Rectangle {
                        anchors.fill: parent
                        color: dates.checked ? "#2A5ACD" : (dates.hovered ? "#333333" : "transparent")
                        border.color: dates.checked ? "#4169E1" : "transparent"
                        border.width: 1
                        radius: 6

                        Behavior on color {
                            ColorAnimation { duration: 200 }
                        }
                    }
                    contentItem: Text {
                        text: dates.text
                        color: dates.checked ? "#ffffff" : "#d0d0d0"
                        font.pixelSize: 14
                        font.weight: dates.checked ? Font.DemiBold : Font.Normal
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter

                        Behavior on color {
                            ColorAnimation { duration: 200 }
                        }
                    }
                }
                TabButton {
                    id: deadline
                    text: qsTr("Deadlines")
                    implicitWidth: bar.availableWidth / 3
                    implicitHeight: 44

                    background: Rectangle {
                        anchors.fill: parent
                        color: deadline.checked ? "#DC143C" : (deadline.hovered ? "#333333" : "transparent")
                        border.color: deadline.checked ? "#FF1493" : "transparent"
                        border.width: 1
                        radius: 6

                        Behavior on color {
                            ColorAnimation { duration: 200 }
                        }
                    }
                    contentItem: Text {
                        text: deadline.text
                        color: deadline.checked ? "#ffffff" : "#d0d0d0"
                        font.pixelSize: 14
                        font.weight: deadline.checked ? Font.DemiBold : Font.Normal
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter

                        Behavior on color {
                            ColorAnimation { duration: 200 }
                        }
                    }
                }
                TabButton {
                    id: linksTab
                    text: qsTr("Links")
                    implicitWidth: bar.availableWidth / 3
                    implicitHeight: 44

                    background: Rectangle {
                        anchors.fill: parent
                        color: linksTab.checked ? "#228B22" : (linksTab.hovered ? "#333333" : "transparent")
                        border.color: linksTab.checked ? "#32CD32" : "transparent"
                        border.width: 1
                        radius: 6

                        Behavior on color {
                            ColorAnimation { duration: 200 }
                        }
                    }
                    contentItem: Text {
                        text: linksTab.text
                        color: linksTab.checked ? "#ffffff" : "#d0d0d0"
                        font.pixelSize: 14
                        font.weight: linksTab.checked ? Font.DemiBold : Font.Normal
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter

                        Behavior on color {
                            ColorAnimation { duration: 200 }
                        }
                    }
                }
            }

            // Container for tab content
            StackLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                currentIndex: bar.currentIndex

                Item{
                    // Calendar tab content
                    CalendarView{
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }

                }
                // Deadlines tab content
                Item {
                    DeadlineView {
                        visible: bar.currentIndex == 1
                        anchors.fill: parent
                    }
                }

               Item{
                   // Links tab content
                   LinkView {
                       id: linkView
                       Layout.fillWidth: true
                       Layout.fillHeight: true
                   }
               }
            }
        }
    }

    // ---------------- CENTER PANEL ----------------
    Rectangle {
        id: folder_bg
        anchors {
            top: parent.top
            bottom: parent.bottom
            left: leftbar.right
            right: rightbar_bg.left
        }
        color: "#181818"



        FolderViewer{
               id: mainScreen
               width: parent.width
               height: parent.height
        }
    }


}

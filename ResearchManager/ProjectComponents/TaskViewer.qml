import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ColumnLayout {
    id: taskViewer
    anchors.fill: parent
    spacing: 10

    ListView {
        id: taskListView
        Layout.columnSpan: 10
        model: task
        Layout.fillWidth: true
        Layout.fillHeight: true
        clip: true
        spacing: 5
        interactive: true

        delegate: Item {
            id: delegateRoot
            width: taskListView.width
            height: Math.max(50, textItem.contentHeight + 20)

            property int visualIndex: index

            Rectangle {
                id: contentItem
                width: delegateRoot.width
                height: delegateRoot.height
                x: 0
                y: 0
                color: "transparent"
                radius: 4

                Drag.active: dragArea.drag.active
                Drag.source: delegateRoot
                Drag.hotSpot.x: width / 2
                Drag.hotSpot.y: height / 2

                border.color: dragArea.drag.active ? "#80ffffff" : "transparent"
                border.width: dragArea.drag.active ? 2 : 0
                opacity: dragArea.drag.active ? 0.7 : 1.0
                scale: dragArea.drag.active ? 1.05 : 1.0

                Behavior on opacity { NumberAnimation { duration: 100 } }
                Behavior on scale { NumberAnimation { duration: 100 } }
                Behavior on border.color { ColorAnimation { duration: 100 } }

                states: State {
                    when: dragArea.drag.active

                    ParentChange {
                        target: contentItem
                        parent: taskListView
                    }

                    PropertyChanges {
                        target: contentItem
                        color: "#40ffffff"
                        z: 999
                    }
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 5
                    spacing: 10

                    Rectangle {
                        id: dragHandle
                        Layout.preferredWidth: 30
                        Layout.fillHeight: true
                        color: dragArea.containsMouse ? "#30ffffff" : "transparent"
                        radius: 3

                        Column {
                            anchors.centerIn: parent
                            spacing: 3

                            Repeater {
                                model: 3
                                Rectangle {
                                    width: 14
                                    height: 2
                                    color: "#90ffffff"
                                    radius: 1
                                }
                            }
                        }

                        MouseArea {
                            id: dragArea
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.SizeVerCursor

                            drag.target: contentItem
                            drag.axis: Drag.YAxis

                            onPressed: {
                                taskListView.interactive = false
                            }

                            onReleased: {
                                contentItem.x = 0
                                contentItem.y = 0
                                taskListView.interactive = true
                            }
                        }
                    }

                    CheckBox {
                        id: taskCheckBox
                        checked: model.checked || false
                        onCheckedChanged: {
                            task.updateCheckedBox(visualIndex, checked)
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        TextEdit {
                            id: textItem
                            anchors.fill: parent
                            text: model.title
                            wrapMode: Text.WordWrap
                            color: "#d6ffffff"
                            font.pixelSize: 16
                            readOnly: true
                            selectByMouse: false
                            focus: false
                            cursorVisible: false
                        }

                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onDoubleClicked: {
                                onEditButtonClicked(task, visualIndex)
                            }
                        }
                    }
                }
            }

            DropArea {
                anchors.fill: parent

                onEntered: function(drag) {
                    var from = drag.source.visualIndex
                    var to = delegateRoot.visualIndex

                    if (from !== to && from !== undefined && to !== undefined) {
                        task.moveItem(from, to)
                    }
                }

            }
        }

        displaced: Transition {
            NumberAnimation {
                properties: "x,y"
                duration: 250
                easing.type: Easing.OutQuad
            }
        }
    }

    function onEditButtonClicked(model, index, text) {
        model.taskIndex = index
        noteViewPopup.open()
    }
}

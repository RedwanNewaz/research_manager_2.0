import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Effects

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

        delegate: Item {
            id: taskItem
            width: parent ? parent.width : undefined
            height: Math.max(50, textItem.contentHeight + 20)

            Rectangle{
                id:taskRect
                anchors.fill: parent
                color: "transparent"

                RowLayout {
                    anchors.fill: parent
                    spacing: 10

                    CheckBox {
                        id: taskCheckBox

                        onCheckedChanged:task.updateCheckedBox(index, checked)
                    }

                    TextEdit {
                        id: textItem
                        text: model.title
                        wrapMode: Text.WordWrap
                        color: "#d6ffffff"
                        font.pixelSize: 16
                        Layout.fillWidth: true

                        readOnly: true
                        selectByMouse: true
                        focus: false
                        cursorVisible: false

                        // Make it clickable
                        MouseArea{
                            anchors.fill: parent
                            hoverEnabled: true
                            onDoubleClicked: onEditButtonClicked(task, index)
                        }
                    }
                }


            }
        }

    }




    // Logic functions

    function onEditButtonClicked(model, index, text) {
        model.taskIndex = index
        noteViewPopup.open()
    }
}

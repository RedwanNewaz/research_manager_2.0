import QtQuick 2.15
import QtQuick.Controls 2.15
import Qt.labs.qmlmodels 1.0
import QtQuick.Layouts 1.15

ColumnLayout {
    anchors.fill: parent

    /* ---------------- Top Spacer ---------------- */
    Rectangle {
        id: rectangle
        Layout.fillWidth: true
        Layout.preferredHeight: parent.height / 4
        color: "#181818"
        Label {
            id: deadlineLabel
            text: "Press on event text"
            font.pixelSize: 20
            font.bold: true
            font.weight: Font.DemiBold
            color: "#2d3748"
            // Layout.fillWidth: true
            anchors.fill: parent
            wrapMode: Text.WordWrap
        }
    }

    /* ---------------- Input Row ---------------- */
    RowLayout {
        Layout.fillWidth: true

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 100
            color: "#3c3c3c"
            border.color: "#555555"
            border.width: 1
            radius: 4

            TextInput {
                id: textDeadlineInput
                anchors.fill: parent
                anchors.margins: 10
                font.pixelSize: 14
                color: "white"
                verticalAlignment: Text.AlignVCenter
                selectByMouse: true
                text: dlModel.deadlineTxt

                property string placeholderText:
                    "Add deadline ... e.g., 12/17/2025 <event_name>\nOne or multiple deadlines are allowed"

                Text {
                    text: textDeadlineInput.placeholderText
                    font.styleName: "Italic"
                    font.pointSize: 12
                    color: "#888888"
                    visible: !textDeadlineInput.text && !textDeadlineInput.activeFocus
                    anchors.centerIn: parent
                    wrapMode: Text.WordWrap
                }
            }
        }

        Button {
            Layout.preferredWidth: 50
            Layout.preferredHeight: 50
            icon.source: "qrc:/ResearchManager/ResearchManager/images/send.svg"
            display: AbstractButton.IconOnly
            onClicked: {
                console.log(textDeadlineInput.text)
                dlModel.deadlineTxt = textDeadlineInput.text
            }
        }
    }

    /* ---------------- Deadline Viewer ---------------- */
    Rectangle {
        id: deadlineViewer
        Layout.fillWidth: true
        Layout.fillHeight: true
        color: "#153584e4"

        TableView {
            id: tableView
            anchors.fill: parent
            clip: true
            columnSpacing: 3
            rowSpacing: 1
            model: dlModel

            property int tableRowIndex: -1

            delegate: Rectangle {
                implicitWidth: (column % 2 != 0)? 2 * deadlineViewer.width / 3 : deadlineViewer.width / 3
                implicitHeight: 50
                color: "transparent"
                border.width: 1
                border.color: "#404040"

                Text {
                    anchors.fill: parent
                    anchors.margins: 8
                    text: display
                    wrapMode: Text.WordWrap
                    color: "white"
                    verticalAlignment: Text.AlignVCenter
                }

                MouseArea {
                    anchors.fill: parent
                    acceptedButtons: Qt.LeftButton | Qt.RightButton

                    onPressed: (mouse) => {
                        if (mouse.button === Qt.RightButton) {
                            tableView.tableRowIndex = row
                            delMenu.popup(mouse.scenePosition)
                        }
                        else{
                            deadlineLabel.text = dlModel.getEventCountdown(row)
                            deadlineLabel.color = "#206ced"

                        }
                    }

                    onExited: {
                        deadlineLabel.text = "Press on event text";
                        deadlineLabel.color = "#2d3748"
                    }

                }
            }
        }
    }

    /* ---------------- Context Menu ---------------- */
    Menu {
        id: delMenu

        MenuItem {
            text: "Delete"
            onTriggered: deleteDeadLine(tableView.tableRowIndex, dlModel)
        }
    }

    /* ---------------- Helpers ---------------- */
    function deleteDeadLine(index, model) {
        if (index < 0)
            return

        const rows = model.rowCount()
        const row = index % rows
        model.deleteRow(row)
    }
}

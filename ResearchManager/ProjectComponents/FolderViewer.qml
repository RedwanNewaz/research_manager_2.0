import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    id: column
    anchors.fill: parent
    spacing: 8

    /* ---------- Search Row ---------- */
    RowLayout {
        Layout.fillWidth: true
        Layout.preferredHeight: 50

        Button {
            id: buttonBack
            Layout.preferredWidth: 50
            Layout.preferredHeight: 50
            visible: false

            icon.source: "../images/back.svg"
            display: AbstractButton.IconOnly

            onClicked: {
                myTextEdit.readOnly = true
                myTextEdit.cursorVisible = false
                project.projectDescription = myTextEdit.text
                visible = false
            }
        }

        BusyIndicator {
            id: downloadBusyIndicator
            Layout.preferredWidth: 50
            Layout.preferredHeight: 50
            running: fileDownloader.isDownloading
            visible: fileDownloader.isDownloading
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 50
            border.color: "white"
            border.width: 2
            color: "#181818"

            TextInput {
                id: textInputSearch
                anchors.fill: parent
                padding: 8
                text: project.linkText
                font.italic: true
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.Wrap
                onAccepted: {
                      console.log("Enter pressed:", text)
                      project.linkText = text
                      // trigger search / submit here
                  }
            }
        }

        Button {
            Layout.preferredWidth: 50
            Layout.preferredHeight: 50
            visible: project.searchButton
            icon.source: "../images/search.svg"
            display: AbstractButton.IconOnly
            onClicked: project.downloadFile(textInputSearch.text)
        }
    }

    /* ---------- Project Description ---------- */
    Rectangle {
        Layout.fillWidth: true
        Layout.preferredHeight: 80
        color: "transparent"

        TextEdit {
            id: myTextEdit
            anchors.fill: parent
            text: project.projectDescription
            font.pointSize: 18
            font.bold: true
            color: "white"
            wrapMode: TextEdit.WordWrap
            horizontalAlignment: Text.AlignHCenter
            readOnly: true
            cursorVisible: false

            MouseArea {
                anchors.fill: parent
                onDoubleClicked: {
                    myTextEdit.readOnly = false
                    myTextEdit.focus = true
                    myTextEdit.cursorVisible = true
                    buttonBack.visible = true
                    enabled = false
                }
            }
        }
    }

    /* ---------- Folder / File View ---------- */
    Rectangle {
        Layout.fillWidth: true
        Layout.fillHeight: true
        color: "transparent"

        ScrollView {
            anchors.fill: parent
            ScrollBar.vertical.policy: ScrollBar.AlwaysOn

            FileViewer {
                anchors.fill: parent
            }
        }
    }
}

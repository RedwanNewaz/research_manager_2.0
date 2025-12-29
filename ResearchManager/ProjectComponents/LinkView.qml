import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    width: parent.width
    height: parent.height
    anchors.fill: parent  // Changed from anchors.top: parent.bottom
    color: "#282828"

    ColumnLayout {
        id: linkViewer
        anchors.fill: parent
        anchors.margins: 10
        spacing: 5

        // ------------- Add Links ---------------
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 50
                color: "#3c3c3c"
                border.color: "#555555"
                border.width: 1
                radius: 4

                TextInput {
                    id: textLinkInput
                    anchors.fill: parent
                    anchors.margins: 10
                    property string placeholderText: "Add external link here..."

                    Text {
                        text: textLinkInput.placeholderText
                        font.styleName: "Italic"
                        font.pointSize: 12
                        color: "#888888"
                        visible: !textLinkInput.text && !textLinkInput.activeFocus
                        anchors.centerIn: parent
                    }

                    font.pixelSize: 14
                    color: "white"
                    verticalAlignment: Text.AlignVCenter
                    selectByMouse: true
                }
            }

            Button {
                id: addLinkBtn
                Layout.preferredWidth: 50
                Layout.preferredHeight: 50
                text: qsTr("Button")
                icon.source: "qrc:/ResearchManager/ResearchManager/images/send.svg"
                display: AbstractButton.IconOnly
            }
        }

        // ------------- Show Links ---------------
        ListView {
            id: linkListView
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: lnModel
            spacing: 5
            clip: true

            delegate: Item {
                id: cellDelegate
                width: linkListView.width
                height: 40
                property bool isEditing: false

                Rectangle {
                    anchors.fill: parent
                    color: "#3c3c3c"
                    radius: 4

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 5
                        spacing: 10

                        CheckBox {
                            id: linkChecker
                            onCheckedChanged:{
                                console.log("[LinkModel] index = ", index, " checked = ", checked)
                                lnModel.checkData(index, checked)
                                if(lnModel.anyCheck())
                                    addLinkBtn.icon.source = "qrc:/ResearchManager/ResearchManager/images/delete.svg";
                                else
                                    addLinkBtn.icon.source = "qrc:/ResearchManager/ResearchManager/images/send.svg"
                            }
                        }



                        Text {
                            text: model.website
                            font.pixelSize: 14
                            Layout.fillWidth: true
                            textFormat: Text.RichText
                            color: "#d6ffffff"
                            font.underline: true
                            elide: Text.ElideRight

                            onLinkActivated: function(link) {
                                console.log(link + " link activated");
                                Qt.openUrlExternally(link);
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                acceptedButtons: Qt.LeftButton | Qt.RightButton
                                onClicked: (mouse) => {
                                    if (mouse.button === Qt.LeftButton && model.url) {
                                        Qt.openUrlExternally(model.url);
                                    }
                                    else if (mouse.button === Qt.RightButton) {
                                       console.log("[LinkView] right clicked pressed")
                                       cellDelegate.isEditing = true
                                    }
                                }

                            }
                        }

                    }

                    // edit link cell
                    TextField {
                        id: cellEditor
                        anchors.fill: parent
                        anchors.margins: 5
                        text: model.website
                        visible: cellDelegate.isEditing
                        verticalAlignment: Text.AlignVCenter

                        onEditingFinished: {
                            // wsModel.setData(wsModel.index(row, column), text, Qt.EditRole)
                            lnModel.updateWebsiteName(index, text)
                            cellDelegate.isEditing = false
                        }

                        onActiveFocusChanged: {
                            if (!activeFocus) {
                                cellDelegate.isEditing = false
                            }
                        }

                        Component.onCompleted: {
                            if (visible) {
                                forceActiveFocus()
                                selectAll()
                            }
                        }
                    }

                }
            }
        }


        // ----------Connections ------

        Connections {
            target: addLinkBtn
            function onClicked() {
                if (lnModel.anyCheck()) {
                    console.log("delete link item " + linkViewer.taskIndex.toString())
                    lnModel.deleteLinks()

                } else if(textLinkInput.text !== "") {
                    console.log("link button pressed " + textLinkInput.text)
                    lnModel.addLink(textLinkInput.text)
                    textLinkInput.text = ""
                }
            }
        }


    }
}

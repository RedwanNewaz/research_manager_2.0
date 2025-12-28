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
                width: linkListView.width
                height: 40

                Rectangle {
                    anchors.fill: parent
                    color: "#3c3c3c"
                    radius: 4

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 5
                        spacing: 10

                        CheckBox {
                            checked: model.checked
                            onCheckedChanged: linkViewer.taskIndex = linkViewer.updateCheckedBox(lnModel, index, checked)
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
                                onClicked: {
                                    if (model.url) {
                                        Qt.openUrlExternally(model.url);
                                    }
                                }

                            }
                        }
                    }
                }
            }
        }

        // ----------Connections ------
        property int taskIndex: -1

        Connections {
            target: addLinkBtn
            function onClicked() {
                if (linkViewer.taskIndex >= 0) {
                    console.log("delete link item " + linkViewer.taskIndex.toString())
                    lnModel.deleteLink(linkViewer.taskIndex)
                    linkViewer.taskIndex = -1
                    addLinkBtn.icon.source = "qrc:/ResearchManager/ResearchManager/images/send.svg"
                } else if(textLinkInput.text !== "") {
                    console.log("link button pressed " + textLinkInput.text)
                    lnModel.addLink(textLinkInput.text)
                    textLinkInput.text = ""
                }
            }
        }

        function updateCheckedBox(model, index, checked) {
            if(!checked) {
                addLinkBtn.icon.source = "qrc:/ResearchManager/ResearchManager/images/send.svg"
                return -1
            }

            for (var i = 0; i < model.rowCount(); ++i)
                if (i !== index)
                    model.setData(model.index(i, 0), false, Qt.UserRole)

            addLinkBtn.icon.source = "qrc:/ResearchManager/ResearchManager/images/delete.svg"
            return index
        }
    }
}

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Effects

Popup {
    id: tagView
    width: Screen.width / 2
    height: Screen.height / 2
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape

    // Center the popup on screen
    anchors.centerIn: Overlay.overlay

    background: Rectangle {
        color: "transparent"
    }

    // Drop shadow effect
    Rectangle {
        anchors.fill: tagContainer
        anchors.margins: -10
        anchors.leftMargin: -30
        color: "transparent"
        radius: 16

        layer.enabled: true
        layer.effect: MultiEffect {
            shadowEnabled: true
            shadowColor: "#40000000"
            shadowBlur: 0.4
            shadowHorizontalOffset: 0
            shadowVerticalOffset: 8
        }
    }

    // Main container with gradient
    Rectangle {
        id: tagContainer
        anchors.fill: parent
        radius: 16

        gradient: Gradient {
            GradientStop { position: 0.0; color: "#ffffff" }
            GradientStop { position: 1.0; color: "#f8f9fa" }
        }

        // Decorative accent at top
        Rectangle {
            width: parent.width
            height: 6
            radius: 16
            anchors.top: parent.top
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop { position: 0.0; color: "#667eea" }
                GradientStop { position: 0.5; color: "#764ba2" }
                GradientStop { position: 1.0; color: "#f093fb" }
            }
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 30
            anchors.topMargin: 35
            spacing: 20

            // Title with icon and buttons
            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                Rectangle {
                    width: 40
                    height: 40
                    radius: 10
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "#667eea" }
                        GradientStop { position: 1.0; color: "#764ba2" }
                    }

                    Text {
                        anchors.centerIn: parent
                        text: "📝"
                        font.pixelSize: 22
                    }
                }

                Label {
                    text: colModel.currentName
                    font.pixelSize: 28
                    font.bold: true
                    font.weight: Font.DemiBold
                    color: "#2d3748"
                    Layout.fillWidth: true
                }

                // Cancel Button
                Rectangle {
                    id: cancelButton
                    Layout.preferredWidth: 100
                    Layout.preferredHeight: 40
                    color: cancelMouseArea.pressed ? "#e2e8f0" : (cancelMouseArea.containsMouse ? "#edf2f7" : "#f7fafc")
                    border.color: "#cbd5e0"
                    border.width: 2
                    radius: 10

                    Behavior on color {
                        ColorAnimation { duration: 150 }
                    }

                    Text {
                        anchors.centerIn: parent
                        text: "Cancel"
                        font.pixelSize: 14
                        font.weight: Font.Medium
                        color: "#4a5568"
                    }

                    MouseArea {
                        id: cancelMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor

                        onClicked: {
                            console.log("Cancel clicked")
                            tagView.close()
                        }
                    }
                }

                // Save Button
                Rectangle {
                    id: saveButton
                    Layout.preferredWidth: 110
                    Layout.preferredHeight: 40
                    radius: 10
                    property bool tagUpdated: false
                    scale: tagUpdated ? 0.95 : 1.0

                    Behavior on scale {
                        NumberAnimation { duration: 100 }
                    }

                    gradient: Gradient {
                        GradientStop {
                            position: 0.0
                            color: "#5568d3"
                        }
                        GradientStop {
                            position: 1.0
                            color: "#6453a1"
                        }
                    }

                    layer.enabled: true
                    layer.effect: MultiEffect {
                        shadowEnabled: true
                        shadowColor: "#30667eea"
                        shadowBlur: 0.3
                        shadowHorizontalOffset: 0
                        shadowVerticalOffset: 4
                    }

                    TextInput {
                        id: tagTxt
                        anchors.centerIn: parent
                        text: colModel.currentTag
                        font.pixelSize: 14
                        font.weight: Font.Medium
                        font.bold: true
                        color: "white"
                        onAccepted: {
                            colModel.currentTag = tagTxt.text
                            tagUpdated = true
                            tagView.close()
                        }
                    }


                }
            }

            // Task Title ComboBox
            ComboBox {
                id: taskTitleComboBox
                Layout.fillWidth: true
                Layout.preferredHeight: 52

                model: colModel.msgComboList
                currentIndex: 0

                font.pixelSize: 16
                font.weight: Font.Bold

                onCurrentIndexChanged: {
                    colModel.setTaskDescription(currentIndex)
                }

                background: Rectangle {
                    color: taskTitleComboBox.pressed ? "#e2e8f0" : (taskTitleComboBox.hovered ? "#f7fafc" : "#f8f9fa")
                    radius: 8
                    border.color: taskTitleComboBox.activeFocus ? "#667eea" : "#e2e8f0"
                    border.width: 2

                    Behavior on border.color {
                        ColorAnimation { duration: 200 }
                    }

                    Behavior on color {
                        ColorAnimation { duration: 150 }
                    }
                }

                contentItem: Text {
                    leftPadding: 12
                    rightPadding: taskTitleComboBox.indicator.width + taskTitleComboBox.spacing

                    text: taskTitleComboBox.displayText
                    font: taskTitleComboBox.font
                    color: "#2d3748"
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }

                indicator: Text {
                    x: taskTitleComboBox.width - width - 12
                    y: taskTitleComboBox.topPadding + (taskTitleComboBox.availableHeight - height) / 2
                    text: "▼"
                    font.pixelSize: 12
                    color: "#667eea"
                }

                popup: Popup {
                    y: taskTitleComboBox.height + 4
                    width: taskTitleComboBox.width
                    implicitHeight: contentItem.implicitHeight
                    padding: 4

                    contentItem: ListView {
                        clip: true
                        implicitHeight: contentHeight
                        model: taskTitleComboBox.popup.visible ? taskTitleComboBox.delegateModel : null
                        currentIndex: taskTitleComboBox.highlightedIndex

                        ScrollIndicator.vertical: ScrollIndicator { }
                    }

                    background: Rectangle {
                        color: "#ffffff"
                        border.color: "#e2e8f0"
                        border.width: 2
                        radius: 8

                        layer.enabled: true
                        layer.effect: MultiEffect {
                            shadowEnabled: true
                            shadowColor: "#20000000"
                            shadowBlur: 0.3
                            shadowHorizontalOffset: 0
                            shadowVerticalOffset: 4
                        }
                    }
                }

                delegate: ItemDelegate {
                    width: taskTitleComboBox.width - 8
                    contentItem: Text {
                        text: modelData
                        color: "#2d3748"
                        font: taskTitleComboBox.font
                        elide: Text.ElideRight
                        verticalAlignment: Text.AlignVCenter
                        leftPadding: 12
                    }

                    background: Rectangle {
                        color: highlighted ? "#667eea" : (hovered ? "#edf2f7" : "transparent")
                        radius: 6

                        Behavior on color {
                            ColorAnimation { duration: 100 }
                        }
                    }

                    highlighted: taskTitleComboBox.highlightedIndex === index
                }
            }
            // Text Area with modern styling
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "#ffffff"
                radius: 12
                border.color: "#e2e8f0"
                border.width: 2

                layer.enabled: true
                layer.effect: MultiEffect {
                    shadowEnabled: true
                    shadowColor: "#10000000"
                    shadowBlur: 0.2
                    shadowHorizontalOffset: 0
                    shadowVerticalOffset: 2
                }

                ScrollView {
                    anchors.fill: parent
                    anchors.margins: 2
                    clip: true

                    TextArea {
                        id: tagTextArea
                        width: parent.width
                        placeholderText: "Start typing your tag..."
                        font.pixelSize: 16
                        font.family: "Segoe UI"
                        color: "#2d3748"
                        wrapMode: TextArea.Wrap
                        selectByMouse: true
                        padding: 15
                        text: colModel.msgDescription
                        placeholderTextColor: "#a0aec0"

                        background: Rectangle {
                            color: "transparent"
                        }
                    }
                }

                // Focus indicator
                Rectangle {
                    anchors.fill: parent
                    color: "transparent"
                    radius: 12
                    border.color: "#667eea"
                    border.width: tagTextArea.activeFocus ? 2 : 0
                    opacity: tagTextArea.activeFocus ? 1 : 0

                    Behavior on opacity {
                        NumberAnimation { duration: 200 }
                    }
                }
            }

            // Character count
            Label {
                text: tagTextArea.text.length + " characters"
                font.pixelSize: 12
                color: "#718096"
                Layout.alignment: Qt.AlignRight
            }
        }
    }
}

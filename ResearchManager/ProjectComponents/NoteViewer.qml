import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Effects

Popup {
    id: noteView
    width: Screen.width / 2
    height: Screen.height / 2
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape

    background: Rectangle {
        color: "transparent"
    }

    // Drop shadow effect
    Rectangle {
        anchors.fill: noteContainer
        anchors.margins: -10
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
        id: noteContainer
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

            // Title with icon
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
                    text: "Edit Note"
                    font.pixelSize: 28
                    font.bold: true
                    font.weight: Font.DemiBold
                    color: "#2d3748"
                    Layout.fillWidth: true
                }
            }

            // Editable Task Title
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 52
                color: "#f8f9fa"
                radius: 8
                border.color: taskTitleInput.activeFocus ? "#667eea" : "#e2e8f0"
                border.width: 2

                Behavior on border.color {
                    ColorAnimation { duration: 200 }
                }

                TextInput {
                    id: taskTitleInput
                    anchors.fill: parent
                    anchors.margins: 12

                    text:  task.taskTitle

                    font.pixelSize: 12
                    font.styleName: "Regular"
                    color: "#2d3748"
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignLeft
                    selectByMouse: true
                    wrapMode: "WordWrap"

                    onActiveFocusChanged: {
                        if (activeFocus) {
                            selectAll()
                        }
                    }
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
                        id: noteTextArea
                        width: parent.width  // ADD THIS - explicitly set width
                        placeholderText: "Start typing your note..."
                        font.pixelSize: 16
                        font.family: "Segoe UI"
                        color: "#2d3748"
                        wrapMode: TextArea.Wrap
                        selectByMouse: true
                        padding: 15
                        text: task.taskDescription
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
                    border.width: noteTextArea.activeFocus ? 2 : 0
                    opacity: noteTextArea.activeFocus ? 1 : 0

                    Behavior on opacity {
                        NumberAnimation { duration: 200 }
                    }
                }
            }

            // Character count
            Label {
                text: noteTextArea.text.length + " characters"
                font.pixelSize: 12
                color: "#718096"
                Layout.alignment: Qt.AlignRight
            }

            // Button Row
            RowLayout {
                Layout.fillWidth: true
                spacing: 15

                Item {
                    Layout.fillWidth: true
                }

                // Cancel Button using Rectangle + MouseArea
                Rectangle {
                    id: cancelButton
                    Layout.preferredWidth: 120
                    Layout.preferredHeight: 45
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
                        font.pixelSize: 15
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
                            noteView.close()
                            noteTextArea.text = ""
                        }
                    }
                }

                // Save Button using Rectangle + MouseArea
                Rectangle {
                    id: saveButton
                    Layout.preferredWidth: 130
                    Layout.preferredHeight: 45
                    radius: 10
                    scale: saveMouseArea.pressed ? 0.95 : 1.0

                    Behavior on scale {
                        NumberAnimation { duration: 100 }
                    }

                    gradient: Gradient {
                        GradientStop {
                            position: 0.0
                            color: saveMouseArea.pressed ? "#5568d3" : (saveMouseArea.containsMouse ? "#7c8eef" : "#667eea")
                        }
                        GradientStop {
                            position: 1.0
                            color: saveMouseArea.pressed ? "#6453a1" : (saveMouseArea.containsMouse ? "#8a6bb8" : "#764ba2")
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

                    Text {
                        anchors.centerIn: parent
                        text: "Save Note"
                        font.pixelSize: 15
                        font.weight: Font.Medium
                        color: "white"
                    }

                    MouseArea {
                        id: saveMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor

                        onClicked: {
                            console.log("Save clicked for task index:", noteView.taskIndex)
                            console.log("Task title:", taskTitleInput.text)
                            console.log("Note content:", noteTextArea.text)

                            // Save the edited task title
                            if (taskTitleInput.text.trim() !== "") {
                                task.editTask(task.taskIndex, taskTitleInput.text, noteTextArea.text)
                            }

                            noteView.close()
                        }
                    }
                }
            }
        }
    }
}

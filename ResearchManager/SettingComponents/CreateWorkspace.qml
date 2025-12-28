import QtQuick 2.15
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Effects

Rectangle {
    id: root
    anchors.fill: parent
    color: "transparent"

    property var placeholders: ({
        "Name": "e.g., GoogleDrive2026",
        "Database": "e.g., G:/My Drive/UNO/2026/database.db",
        "Year": "e.g., 2026",
        "Workspace": "e.g., G:/My Drive/UNO/2026/ResearchWorkspace",
        "Icon": "e.g., icons8-google-drive-240.svg"
    })

    Flickable {
        anchors.fill: parent
        contentHeight: contentColumn.height + 40
        clip: true

        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AsNeeded

            contentItem: Rectangle {
                implicitWidth: 8
                radius: 4
                color: parent.pressed ? "#6366f1" : (parent.hovered ? "#4b5563" : "#374151")

                Behavior on color {
                    ColorAnimation { duration: 200 }
                }
            }

            background: Rectangle {
                color: "#1f2937"
                radius: 4
            }
        }

        Column {
            id: contentColumn
            width: parent.width
            spacing: 25
            topPadding: 20
            bottomPadding: 20

            // Header
            Column {
                width: parent.width - 40
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 10

                Text {
                    text: "✨ Create New Workspace"
                    color: "#f3f4f6"
                    font.pixelSize: 26
                    font.weight: Font.Bold
                }

                Rectangle {
                    width: parent.width
                    height: 2
                    gradient: Gradient {
                        orientation: Gradient.Horizontal
                        GradientStop { position: 0.0; color: "#6366f1" }
                        GradientStop { position: 0.5; color: "#8b5cf6" }
                        GradientStop { position: 1.0; color: "transparent" }
                    }
                }

                Text {
                    text: "Fill in the details below to create a new workspace"
                    color: "#9ca3af"
                    font.pixelSize: 13
                }
            }

            // Form Fields
            Repeater {
                model: settingsModel

                delegate: Rectangle {
                    width: parent.width - 40
                    height: 110
                    anchors.horizontalCenter: parent.horizontalCenter
                    color: fieldMouseArea.containsMouse ? "#1f2937" : "#111827"
                    border.color: textField.activeFocus ? "#6366f1" : "#374151"
                    border.width: textField.activeFocus ? 2 : 1
                    radius: 12

                    Behavior on color {
                        ColorAnimation { duration: 200 }
                    }

                    Behavior on border.color {
                        ColorAnimation { duration: 200 }
                    }

                    Behavior on border.width {
                        NumberAnimation { duration: 200 }
                    }

                    // Subtle glow when focused
                    layer.enabled: textField.activeFocus
                    layer.effect: MultiEffect {
                        shadowEnabled: true
                        shadowColor: "#6366f1"
                        shadowBlur: 0.3
                        shadowOpacity: 0.4
                    }

                    MouseArea {
                        id: fieldMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        propagateComposedEvents: true
                        onClicked: textField.forceActiveFocus()
                    }

                    Column {
                        anchors.fill: parent
                        anchors.margins: 20
                        spacing: 10

                        Row {
                            width: parent.width
                            spacing: 8

                            Text {
                                text: getIconForField(model.name)
                                font.pixelSize: 18
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            Text {
                                text: model.name
                                color: "#f3f4f6"
                                font.pixelSize: 14
                                font.weight: Font.Bold
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            Rectangle {
                                visible: isRequired(model.name)
                                width: 8
                                height: 8
                                radius: 4
                                color: "#ef4444"
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }

                        TextField {
                            id: textField
                            width: parent.width
                            text: model.value
                            placeholderText: root.placeholders[model.name] || "Enter " + model.name.toLowerCase()
                            color: "#f3f4f6"
                            placeholderTextColor: "#4b5563"
                            font.pixelSize: 13
                            selectByMouse: true

                            background: Rectangle {
                                color: "#0f0f0f"
                                border.color: textField.activeFocus ? "#8b5cf6" : "#2a2a2a"
                                border.width: 1
                                radius: 8

                                Behavior on border.color {
                                    ColorAnimation { duration: 200 }
                                }
                            }

                            leftPadding: 12
                            rightPadding: 12
                            topPadding: 10
                            bottomPadding: 10

                            onTextChanged: {
                                settingsModel.setProperty(index, "value", text)
                            }

                            // Add visual feedback on focus
                            onActiveFocusChanged: {
                                if (activeFocus) {
                                    selectAll()
                                }
                            }

                            // Handle special keys
                            Keys.onReturnPressed: {
                                // Move to next field
                                if (index < settingsModel.count - 1) {
                                    var nextItem = contentColumn.children[index + 2]
                                    if (nextItem && nextItem.children[1]) {
                                        nextItem.children[1].children[2].forceActiveFocus()
                                    }
                                }
                            }

                            Keys.onTabPressed: {
                                // Move to next field
                                event.accepted = true
                                if (index < settingsModel.count - 1) {
                                    var nextItem = contentColumn.children[index + 2]
                                    if (nextItem && nextItem.children[1]) {
                                        nextItem.children[1].children[2].forceActiveFocus()
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // Info box
            Rectangle {
                width: parent.width - 40
                height: infoColumn.height + 30
                anchors.horizontalCenter: parent.horizontalCenter
                color: "#1e3a8a"
                opacity: 0.3
                radius: 10
                border.color: "#3b82f6"
                border.width: 1

                Column {
                    id: infoColumn
                    anchors.centerIn: parent
                    width: parent.width - 30
                    spacing: 8

                    Text {
                        text: "💡 Tips"
                        color: "#93c5fd"
                        font.pixelSize: 13
                        font.bold: true
                    }

                    Text {
                        text: "• Press Tab or Enter to move to the next field\n• Required fields are marked with a red dot\n• Use Browse buttons (if available) to select paths\n• All fields can be edited later from workspace manager"
                        color: "#bfdbfe"
                        font.pixelSize: 11
                        wrapMode: Text.WordWrap
                        width: parent.width
                        lineHeight: 1.4
                    }
                }
            }
        }
    }

    function getIconForField(fieldName) {
        var icons = {
            "Name": "📝",
            "Database": "🗄️",
            "Year": "📅",
            "Workspace": "📁",
            "Icon": "🎨"
        }
        return icons[fieldName] || "•"
    }

    function isRequired(fieldName) {
        var required = ["Name", "Database", "Year", "Workspace"]
        return required.indexOf(fieldName) !== -1
    }
}

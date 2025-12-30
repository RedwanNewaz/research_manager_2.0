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
        "Icon": "Start typing: Local, One Drive, Google Drive, Dropbox"
    })

    property var iconOptions: ({
        "Local": "local-folder.svg",
        "One Drive": "icons8-google-drive-240.svg",
        "Google Drive": "icons8-google-drive-64.png",
        "Dropbox": "icons8-dropbox-64.png"
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
                    height: model.name === "Icon" && suggestionList.visible ? 220 : 110
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

                    Behavior on height {
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
                        anchors.bottomMargin: model.name === "Icon" && suggestionList.visible ? 110 : 0
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

                                // Show autocomplete for Icon field
                                if (model.name === "Icon") {
                                    updateSuggestions(text)
                                }
                            }

                            onActiveFocusChanged: {
                                if (activeFocus) {
                                    selectAll()
                                    if (model.name === "Icon") {
                                        updateSuggestions(text)
                                    }
                                } else {
                                    if (model.name === "Icon") {
                                        suggestionList.visible = false
                                    }
                                }
                            }

                            Keys.onReturnPressed: {
                                if (model.name === "Icon" && suggestionList.visible && suggestionList.currentIndex >= 0) {
                                    selectSuggestion(suggestionList.currentIndex)
                                    event.accepted = true
                                } else {
                                    // Move to next field
                                    if (index < settingsModel.count - 1) {
                                        var nextItem = contentColumn.children[index + 2]
                                        if (nextItem && nextItem.children[1]) {
                                            nextItem.children[1].children[2].forceActiveFocus()
                                        }
                                    }
                                }
                            }

                            Keys.onTabPressed: {
                                event.accepted = true
                                if (model.name === "Icon" && suggestionList.visible && suggestionList.currentIndex >= 0) {
                                    selectSuggestion(suggestionList.currentIndex)
                                }

                                // Move to next field
                                if (index < settingsModel.count - 1) {
                                    var nextItem = contentColumn.children[index + 2]
                                    if (nextItem && nextItem.children[1]) {
                                        nextItem.children[1].children[2].forceActiveFocus()
                                    }
                                }
                            }

                            Keys.onDownPressed: {
                                if (model.name === "Icon" && suggestionList.visible) {
                                    suggestionList.incrementCurrentIndex()
                                    event.accepted = true
                                }
                            }

                            Keys.onUpPressed: {
                                if (model.name === "Icon" && suggestionList.visible) {
                                    suggestionList.decrementCurrentIndex()
                                    event.accepted = true
                                }
                            }

                            function updateSuggestions(input) {
                                var suggestions = []
                                var keys = Object.keys(root.iconOptions)

                                if (input.length === 0) {
                                    suggestions = keys
                                } else {
                                    var lowerInput = input.toLowerCase()
                                    for (var i = 0; i < keys.length; i++) {
                                        if (keys[i].toLowerCase().indexOf(lowerInput) !== -1) {
                                            suggestions.push(keys[i])
                                        }
                                    }
                                }

                                suggestionModel.clear()
                                for (var j = 0; j < suggestions.length; j++) {
                                    suggestionModel.append({
                                        "name": suggestions[j],
                                        "path": root.iconOptions[suggestions[j]]
                                    })
                                }

                                suggestionList.visible = suggestions.length > 0 && textField.activeFocus
                                if (suggestionList.visible) {
                                    suggestionList.currentIndex = 0
                                }
                            }

                            function selectSuggestion(idx) {
                                if (idx >= 0 && idx < suggestionModel.count) {
                                    var selectedName = suggestionModel.get(idx).name
                                    var selectedPath = suggestionModel.get(idx).path
                                    textField.text = selectedPath
                                    settingsModel.setProperty(index, "value", selectedPath)
                                    suggestionList.visible = false
                                }
                            }
                        }

                        // Autocomplete suggestions list (only for Icon field)
                        Rectangle {
                            id: suggestionList
                            width: parent.width
                            height: 100
                            visible: false
                            color: "#111827"
                            border.color: "#374151"
                            border.width: 1
                            radius: 8
                            clip: true

                            property alias currentIndex: listView.currentIndex

                            ListView {
                                id: listView
                                anchors.fill: parent
                                anchors.margins: 1
                                model: ListModel { id: suggestionModel }
                                clip: true

                                delegate: ItemDelegate {
                                    width: listView.width
                                    height: 35

                                    background: Rectangle {
                                        color: {
                                            if (index === listView.currentIndex) return "#374151"
                                            if (hovered) return "#1f2937"
                                            return "transparent"
                                        }

                                        Behavior on color {
                                            ColorAnimation { duration: 150 }
                                        }
                                    }

                                    contentItem: Row {
                                        spacing: 10
                                        leftPadding: 12

                                        Text {
                                            text: "🎨"
                                            font.pixelSize: 14
                                            anchors.verticalCenter: parent.verticalCenter
                                        }

                                        Column {
                                            anchors.verticalCenter: parent.verticalCenter
                                            spacing: 2

                                            Text {
                                                text: model.name
                                                color: "#f3f4f6"
                                                font.pixelSize: 13
                                                font.weight: Font.Medium
                                            }

                                            Text {
                                                text: model.path
                                                color: "#6b7280"
                                                font.pixelSize: 10
                                            }
                                        }
                                    }

                                    onClicked: {
                                        textField.selectSuggestion(index)
                                    }

                                    hoverEnabled: true
                                }

                                ScrollBar.vertical: ScrollBar {
                                    policy: ScrollBar.AsNeeded
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
                        text: "• Press Tab or Enter to move to the next field\n• Required fields are marked with a red dot\n• For Icon field, start typing to see autocomplete suggestions\n• Use arrow keys to navigate suggestions, Enter to select\n• All fields can be edited later from workspace manager"
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

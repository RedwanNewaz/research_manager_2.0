import QtQuick
import QtQuick.Controls
import Qt.labs.folderlistmodel 2.15
import QtQuick.Window 2.15
import QtQuick.Dialogs
import QtQuick.Effects
import "SettingComponents"

Item {
    id: root
    width: Screen.width
    height: Screen.height - 70

    // Signal to open file dialog from anywhere
    signal openFileDialog()

    onOpenFileDialog: {
        configDbFileDialog.open()
    }

    // FileDialog for selecting config database
    FileDialog {
        id: configDbFileDialog
        title: "Select Config Database File"
        nameFilters: ["Database Files (*.db)", "All Files (*)"]
        onAccepted: {
            var path = selectedFile.toLocalFile()
            settingsManager.setConfigDatabasePath(path)
        }
    }

    // Animated gradient background
    Rectangle {
        anchors.fill: parent

        gradient: Gradient {
            GradientStop { position: 0.0; color: "#0f0f0f" }
            GradientStop { position: 1.0; color: "#1a1a1a" }
        }

        // Subtle animated overlay
        Rectangle {
            anchors.fill: parent
            opacity: 0.03

            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop { position: 0.0; color: "#6366f1" }
                GradientStop { position: 0.5; color: "#8b5cf6" }
                GradientStop { position: 1.0; color: "#ec4899" }
            }

            SequentialAnimation on opacity {
                loops: Animation.Infinite
                NumberAnimation { to: 0.06; duration: 3000; easing.type: Easing.InOutSine }
                NumberAnimation { to: 0.03; duration: 3000; easing.type: Easing.InOutSine }
            }
        }

        // Navigation Buttons - Centered at top with glassmorphism
        Rectangle {
            id: navBar
            width: topNavigation.width + 40
            height: topNavigation.height + 20
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.topMargin: 30

            color: "#1a1a1a"
            opacity: 0.8
            radius: 16
            border.color: "#ffffff"
            border.width: 1

            // Subtle glow effect
            layer.enabled: true
            layer.effect: MultiEffect {
                shadowEnabled: true
                shadowColor: "#60a5f7"
                shadowBlur: 0.4
                shadowOpacity: 0.3
            }

            Row {
                id: topNavigation
                spacing: 12
                anchors.centerIn: parent

                Repeater {
                    model: [
                        {text: "Manage Workspace", icon: "📊", obj: "workspaceManager", page: "SettingComponents/WorkspaceManager.qml"},
                        {text: "Create Workspace", icon: "✨", obj: "createWorkspace", component: createWorkspaceComponent},
                        {text: "Database Settings", icon: "🗄️", obj: "databaseSettings", component: databaseSettingsComponent},
                        {text: "Template Manager", icon: "📝", obj: "templateManager", page: "SettingComponents/TemplateManager.qml"}
                    ]

                    delegate: Rectangle {
                        id: btnRect
                        width: 180
                        height: 56
                        radius: 12

                        property bool isActive: stackView.depth > 0 && stackView.currentItem.objectName === modelData.obj
                        property bool hovered: btnMouseArea.containsMouse

                        color: isActive ? "#6366f1" : (hovered ? "#2a2a2a" : "#222222")
                        border.color: isActive ? "#818cf8" : (hovered ? "#404040" : "#2a2a2a")
                        border.width: 2

                        scale: hovered ? 1.05 : 1.0

                        Behavior on color { ColorAnimation { duration: 200 } }
                        Behavior on border.color { ColorAnimation { duration: 200 } }
                        Behavior on scale {
                            NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
                        }

                        // Gradient overlay for active state
                        Rectangle {
                            anchors.fill: parent
                            radius: parent.radius
                            opacity: isActive ? 0.2 : 0
                            gradient: Gradient {
                                GradientStop { position: 0.0; color: "#ffffff" }
                                GradientStop { position: 1.0; color: "transparent" }
                            }
                            Behavior on opacity { NumberAnimation { duration: 200 } }
                        }

                        Row {
                            anchors.centerIn: parent
                            spacing: 8

                            Text {
                                text: modelData.icon
                                font.pixelSize: 20
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            Text {
                                text: modelData.text
                                color: btnRect.isActive ? "#ffffff" : (btnRect.hovered ? "#e5e7eb" : "#9ca3af")
                                font.pixelSize: 13
                                font.weight: btnRect.isActive ? Font.Bold : Font.Normal
                                anchors.verticalCenter: parent.verticalCenter

                                Behavior on color { ColorAnimation { duration: 200 } }
                            }
                        }

                        MouseArea {
                            id: btnMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor

                            onClicked: {
                                if (modelData.page) {
                                    stackView.replace(modelData.page)
                                } else if (modelData.component) {
                                    stackView.replace(modelData.component)
                                }
                            }
                        }

                        // Ripple effect on click
                        Rectangle {
                            id: ripple
                            width: 0
                            height: 0
                            radius: width / 2
                            color: "#ffffff"
                            opacity: 0
                            anchors.centerIn: parent

                            ParallelAnimation {
                                id: rippleAnim
                                NumberAnimation {
                                    target: ripple
                                    property: "width"
                                    to: btnRect.width * 2
                                    duration: 400
                                    easing.type: Easing.OutQuad
                                }
                                NumberAnimation {
                                    target: ripple
                                    property: "height"
                                    to: btnRect.height * 2
                                    duration: 400
                                    easing.type: Easing.OutQuad
                                }
                                NumberAnimation {
                                    target: ripple
                                    property: "opacity"
                                    from: 0.3
                                    to: 0
                                    duration: 400
                                    easing.type: Easing.OutQuad
                                }
                            }

                            Connections {
                                target: btnMouseArea
                                function onClicked() {
                                    ripple.width = 0
                                    ripple.height = 0
                                    rippleAnim.start()
                                }
                            }
                        }
                    }
                }
            }
        }

        // Main Content Area - Centered with card style
        Rectangle {
            id: contentCard
            width: Math.min(parent.width - 100, 1400)
            height: parent.height - navBar.height - 100
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: navBar.bottom
            anchors.topMargin: 30

            color: "#1a1a1a"
            opacity: 0.95
            radius: 20
            border.color: "#2a2a2a"
            border.width: 1

            layer.enabled: true
            layer.effect: MultiEffect {
                shadowEnabled: true
                shadowColor: "#000000"
                shadowBlur: 1.0
                shadowOpacity: 0.5
            }

            StackView {
                id: stackView
                anchors.fill: parent
                anchors.margins: 20

                initialItem: "SettingComponents/WorkspaceManager.qml"

                replaceEnter: Transition {
                    NumberAnimation {
                        property: "opacity"
                        from: 0
                        to: 1
                        duration: 300
                        easing.type: Easing.OutCubic
                    }
                    NumberAnimation {
                        property: "x"
                        from: stackView.width * 0.1
                        to: 0
                        duration: 300
                        easing.type: Easing.OutCubic
                    }
                }

                replaceExit: Transition {
                    NumberAnimation {
                        property: "opacity"
                        from: 1
                        to: 0
                        duration: 200
                        easing.type: Easing.InCubic
                    }
                }
            }
        }
    }

    // Component for Create Workspace
    Component {
        id: createWorkspaceComponent

        Item {
            objectName: "createWorkspace"

            ListModel {
                id: settingsModel
                ListElement { name: "Name"; value: "" }
                ListElement { name: "Database"; value: "" }
                ListElement { name: "Year"; value: "" }
                ListElement { name: "Workspace"; value: "" }
                ListElement { name: "Icon"; value: "" }
            }

            CreateWorkspace {
                id: createWorkspace
                anchors.fill: parent
            }

            Row {
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottomMargin: 20
                spacing: 15

                Rectangle {
                    width: 150
                    height: 48
                    radius: 12

                    gradient: Gradient {
                        GradientStop { position: 0.0; color: updateMouseArea.containsMouse ? "#7c3aed" : "#6366f1" }
                        GradientStop { position: 1.0; color: updateMouseArea.containsMouse ? "#6366f1" : "#4f46e5" }
                    }

                    scale: updateMouseArea.containsMouse ? 1.05 : 1.0

                    Behavior on scale {
                        NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
                    }

                    Text {
                        anchors.centerIn: parent
                        text: "✓ Update"
                        color: "white"
                        font.pixelSize: 14
                        font.bold: true
                    }

                    MouseArea {
                        id: updateMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.onUpdateButtonClicked(settingsModel, true)
                    }
                }

                Rectangle {
                    width: 150
                    height: 48
                    radius: 12
                    color: cancelMouseArea.containsMouse ? "#374151" : "#1f2937"
                    border.color: "#4b5563"
                    border.width: 2

                    scale: cancelMouseArea.containsMouse ? 1.05 : 1.0

                    Behavior on color { ColorAnimation { duration: 200 } }
                    Behavior on scale {
                        NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
                    }

                    Text {
                        anchors.centerIn: parent
                        text: "✕ Cancel"
                        color: "#e5e7eb"
                        font.pixelSize: 14
                        font.bold: true
                    }

                    MouseArea {
                        id: cancelMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.onUpdateButtonClicked(settingsModel, false)
                    }
                }
            }
        }
    }

    // Component for Database Settings
    Component {
        id: databaseSettingsComponent

        Rectangle {
            id: dbSettingsRect
            objectName: "databaseSettings"
            color: "transparent"

            Connections {
                target: settingsManager
                function onConfigDatabasePathChanged() {
                    dbPathEdit.text = settingsManager.configDatabasePath
                    currentPathText.text = "Current Path: " + settingsManager.configDatabasePath
                }
            }

            Column {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 25

                Text {
                    text: "🗄️ Database Configuration"
                    color: "#f3f4f6"
                    font.pixelSize: 28
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
                    text: "Config Database Path"
                    color: "#d1d5db"
                    font.pixelSize: 15
                    font.weight: Font.Medium
                }

                Rectangle {
                    width: parent.width
                    height: 60
                    color: "#0f0f0f"
                    border.color: "#374151"
                    border.width: 2
                    radius: 10

                    TextEdit {
                        id: dbPathEdit
                        anchors.fill: parent
                        anchors.margins: 15
                        color: "#9ca3af"
                        font.pixelSize: 13
                        text: settingsManager.configDatabasePath
                        readOnly: true
                        verticalAlignment: Text.AlignVCenter
                        wrapMode: Text.WordWrap
                    }
                }

                Rectangle {
                    width: 160
                    height: 50
                    radius: 12

                    gradient: Gradient {
                        GradientStop { position: 0.0; color: browseMouseArea.containsMouse ? "#7c3aed" : "#6366f1" }
                        GradientStop { position: 1.0; color: browseMouseArea.containsMouse ? "#6366f1" : "#4f46e5" }
                    }

                    scale: browseMouseArea.containsMouse ? 1.05 : 1.0

                    Behavior on scale {
                        NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
                    }

                    Text {
                        anchors.centerIn: parent
                        text: "📁 Browse..."
                        color: "white"
                        font.pixelSize: 14
                        font.bold: true
                    }

                    MouseArea {
                        id: browseMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            console.log("Browse button clicked - emitting signal")
                            root.openFileDialog()
                        }
                    }
                }

                Text {
                    id: currentPathText
                    text: "Current: " + settingsManager.configDatabasePath
                    color: "#6b7280"
                    font.pixelSize: 12
                    wrapMode: Text.WordWrap
                    width: parent.width
                }

                Item { height: 20 }

                Rectangle {
                    width: 120
                    height: 45
                    radius: 10
                    color: closeMouseArea.containsMouse ? "#374151" : "#1f2937"
                    border.color: "#4b5563"
                    border.width: 2

                    scale: closeMouseArea.containsMouse ? 1.05 : 1.0

                    Behavior on color { ColorAnimation { duration: 200 } }
                    Behavior on scale {
                        NumberAnimation { duration: 200; easing.type: Easing.OutCubic }
                    }

                    Text {
                        anchors.centerIn: parent
                        text: "← Close"
                        color: "#e5e7eb"
                        font.pixelSize: 14
                        font.bold: true
                    }

                    MouseArea {
                        id: closeMouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: stackView.replace("SettingComponents/WorkspaceManager.qml")
                    }
                }
            }
        }
    }

    // Update function
    function onUpdateButtonClicked(settingsModel, update) {
        if (!update) return;

        let dataResults = {};
        for (let i = 0; i < settingsModel.count; i++) {
            let item = settingsModel.get(i);
            dataResults[item.name] = item.value;
            console.log("create workspace pressed", item.value);
        }

        if (wsModel.createWorkspace(dataResults)) {
            console.log("Workspace created successfully. Clearing fields...");
            for (let j = 0; j < settingsModel.count; j++) {
                settingsModel.setProperty(j, "value", "");
            }
        } else if (wsModel.updateWorkspace(dataResults)) {
            console.log("Workspace updated successfully. Clearing fields...");
            for (let j = 0; j < settingsModel.count; j++) {
                settingsModel.setProperty(j, "value", "");
            }
        } else {
            console.log("Failed to create or update workspace.");
        }
    }
}

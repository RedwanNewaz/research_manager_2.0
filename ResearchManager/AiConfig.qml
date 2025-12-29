import QtQuick
import QtQuick.Controls

import Qt.labs.folderlistmodel 2.15
import QtQuick.Layouts 1.12
import Qt5Compat.GraphicalEffects
import QtQuick.Dialogs
import QtCore

Rectangle {
    id: rootAiConfig
    width: Screen.width
    height: Screen.height - 70
    color: "#181818"

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

    // Load data from backend on component completion
    Component.onCompleted: {
        geminiAPIText.text = aiConfig.geminiApiKey || "Enter your Gemini API key here"
        geminiPdfRenameText.text = aiConfig.pdfRenamePrompt
        geminiCalendarText.text = aiConfig.calendarPrompt
        geminiTaskText.text = aiConfig.taskPrompt
    }

    // Update response area when test completes
    Connections {
        target: aiConfig
        function onTestResponseChanged() {
            geminiResponseText.text = aiConfig.testResponse
        }
        function onIsLoadingChanged() {
            testButton.enabled = !aiConfig.isLoading
            loadingIndicator.visible = aiConfig.isLoading
        }
    }

    ScrollView {
        anchors.fill: parent
        anchors.margins: 20
        contentWidth: availableWidth

        GridLayout {
            id: grid
            anchors.horizontalCenter: parent.horizontalCenter
            columns: 2
            columnSpacing: 20
            rowSpacing: 15
            width: parent.width * 0.8
            
            property int promptHeight: 80
            property int apiKeyHeight: 50
            property int responseHeight: 150

            // === API Key Section ===
            Label { 
                text: "Gemini API Key:" 
                font.pointSize: 14
                font.bold: true
                color: "white"
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            }
            Rectangle {
                Layout.fillWidth: true
                height: grid.apiKeyHeight
                color: "#282828"
                border.color: "#444"
                border.width: 1
                radius: 4
                
                TextEdit {
                    id: geminiAPIText
                    anchors.fill: parent
                    anchors.margins: 10
                    color: "white"
                    font.pointSize: 12
                    wrapMode: TextEdit.Wrap
                    verticalAlignment: Text.AlignVCenter
                    
                    onTextChanged: {
                        if (text !== aiConfig.geminiApiKey) {
                            aiConfig.geminiApiKey = text
                        }
                    }
                }
            }

            // === PDF Rename Prompt ===
            Label { 
                text: "PDF Rename Prompt:" 
                font.pointSize: 14
                font.bold: true
                color: "white"
                Layout.alignment: Qt.AlignRight | Qt.AlignTop
            }
            Rectangle {
                Layout.fillWidth: true
                height: grid.promptHeight
                color: "#282828"
                border.color: "#444"
                border.width: 1
                radius: 4
                
                ScrollView {
                    anchors.fill: parent
                    anchors.margins: 5
                    
                    TextEdit {
                        id: geminiPdfRenameText
                        width: parent.width
                        color: "white"
                        font.pointSize: 11
                        wrapMode: TextEdit.Wrap
                        
                        onTextChanged: {
                            if (text !== aiConfig.pdfRenamePrompt) {
                                aiConfig.pdfRenamePrompt = text
                            }
                        }
                    }
                }
            }

            // === Calendar Parser Prompt ===
            Label { 
                text: "Calendar Parser Prompt:" 
                font.pointSize: 14
                font.bold: true
                color: "white"
                Layout.alignment: Qt.AlignRight | Qt.AlignTop
            }
            Rectangle {
                Layout.fillWidth: true
                height: grid.promptHeight
                color: "#282828"
                border.color: "#444"
                border.width: 1
                radius: 4
                
                ScrollView {
                    anchors.fill: parent
                    anchors.margins: 5
                    
                    TextEdit {
                        id: geminiCalendarText
                        width: parent.width
                        color: "white"
                        font.pointSize: 11
                        wrapMode: TextEdit.Wrap
                        
                        onTextChanged: {
                            if (text !== aiConfig.calendarPrompt) {
                                aiConfig.calendarPrompt = text
                            }
                        }
                    }
                }
            }

            // === Task Parser Prompt ===
            Label { 
                text: "Task Parser Prompt:" 
                font.pointSize: 14
                font.bold: true
                color: "white"
                Layout.alignment: Qt.AlignRight | Qt.AlignTop
            }
            Rectangle {
                Layout.fillWidth: true
                height: grid.promptHeight
                color: "#282828"
                border.color: "#444"
                border.width: 1
                radius: 4
                
                ScrollView {
                    anchors.fill: parent
                    anchors.margins: 5
                    
                    TextEdit {
                        id: geminiTaskText
                        width: parent.width
                        color: "white"
                        font.pointSize: 11
                        wrapMode: TextEdit.Wrap
                        
                        onTextChanged: {
                            if (text !== aiConfig.taskPrompt) {
                                aiConfig.taskPrompt = text
                            }
                        }
                    }
                }
            }

            // === Save Button ===
            Item { Layout.fillWidth: true } // Spacer
            Button {
                id: saveButton
                text: "Save Configuration"
                Layout.preferredWidth: 150
                Layout.preferredHeight: 40
                
                background: Rectangle {
                    color: saveButton.pressed ? "#1a6b1a" : (saveButton.hovered ? "#2d8a2d" : "#228B22")
                    radius: 4
                }
                
                contentItem: Text {
                    text: saveButton.text
                    color: "white"
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                
                onClicked: {
                    aiConfig.saveConfig()
                }
            }

            // === Test Section Header ===
            Rectangle {
                Layout.columnSpan: 2
                Layout.fillWidth: true
                height: 2
                color: "#444"
                Layout.topMargin: 20
                Layout.bottomMargin: 10
            }

            Label { 
                text: "Test Gemini API:" 
                font.pointSize: 14
                font.bold: true
                color: "white"
                Layout.alignment: Qt.AlignRight | Qt.AlignTop
            }
            Rectangle {
                Layout.fillWidth: true
                height: grid.promptHeight
                color: "#282828"
                border.color: "#444"
                border.width: 1
                radius: 4
                
                ScrollView {
                    anchors.fill: parent
                    anchors.margins: 5
                    
                    TextEdit {
                        id: geminiTestPrompt
                        width: parent.width
                        color: "white"
                        font.pointSize: 11
                        wrapMode: TextEdit.Wrap
                        text: "Hello! Can you confirm that you're working? Please respond with a brief greeting."
                    }
                }
            }

            // === Test Button with Loading Indicator ===
            Item { Layout.fillWidth: true } // Spacer
            Row {
                spacing: 10
                Layout.alignment: Qt.AlignLeft
                
                Button {
                    id: testButton
                    text: "Send Test Prompt"
                    width: 180
                    height: 40
                    
                    background: Rectangle {
                        color: testButton.enabled ? (testButton.pressed ? "#1a4a7a" : (testButton.hovered ? "#2a6aaa" : "#2196F3")) : "#666"
                        radius: 4
                    }
                    
                    contentItem: Row {
                        anchors.centerIn: parent
                        spacing: 8
                        
                        Image {
                            source: "images/ai-svgrepo-com.svg"
                            width: 20
                            height: 20
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        
                        Text {
                            text: testButton.text
                            color: "white"
                            font.bold: true
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                    
                    onClicked: {
                        geminiResponseText.text = "Sending request..."
                        aiConfig.testGemini(geminiTestPrompt.text)
                    }
                }
                
                BusyIndicator {
                    id: loadingIndicator
                    running: visible
                    visible: false
                    width: 40
                    height: 40
                }
            }

            // === Response Section ===
            Label { 
                text: "Response:" 
                font.pointSize: 14
                font.bold: true
                color: "white"
                Layout.alignment: Qt.AlignRight | Qt.AlignTop
            }
            Rectangle {
                Layout.fillWidth: true
                height: grid.responseHeight
                color: "#1e1e1e"
                border.color: "#444"
                border.width: 1
                radius: 4
                
                ScrollView {
                    anchors.fill: parent
                    anchors.margins: 10
                    
                    TextEdit {
                        id: geminiResponseText
                        width: parent.width
                        color: "#90EE90"
                        font.pointSize: 11
                        font.family: "Consolas"
                        wrapMode: TextEdit.Wrap
                        readOnly: true
                        text: "Response will appear here..."
                        selectByMouse: true
                    }
                }
            }
        }
    }
}

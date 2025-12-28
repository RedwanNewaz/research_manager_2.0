import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

ColumnLayout {
    anchors.fill: parent
    
    // AI Rename result notification
    Connections {
        target: aiConfig
        function onPdfRenameCompleted(success, oldPath, newPath) {
            if (success) {
                renameSuccessDialog.text = "File renamed successfully!\n\nNew name: " + newPath.split('/').pop()
                renameSuccessDialog.open()
                // Refresh the file list
                flModel.refresh()
            }
        }
        function onPdfRenameError(error) {
            renameErrorDialog.text = "Failed to rename PDF:\n\n" + error
            renameErrorDialog.open()
        }
    }
    
    // Success dialog
    MessageDialog {
        id: renameSuccessDialog
        title: "AI Rename Complete"
        buttons: MessageDialog.Ok
    }
    
    // Error dialog
    MessageDialog {
        id: renameErrorDialog
        title: "AI Rename Error"
        buttons: MessageDialog.Ok
    }

    Rectangle {
        Layout.fillWidth: true
        Layout.fillHeight: true
        color: "#181818"

        SplitView {
            anchors.fill: parent
            orientation: Qt.Horizontal

            // Left side - Folders only
            Rectangle {
                SplitView.minimumWidth: 200
                SplitView.preferredWidth: parent.width * 0.4
                color: "#1e1e1e"

                TreeView {
                    id: folderTree
                    anchors.fill: parent
                    anchors.margins: 5
                    model: fsModel
                    rootIndex: fsWrapper.rootIndex
                    clip: true

                    visible: fsWrapper.rootIndex && fsWrapper.rootIndex.valid

                    delegate: Item {
                        id: folderDelegate

                        required property int row
                        required property int column
                        required property bool expanded
                        required property int depth
                        required property bool isTreeNode
                        required property bool hasChildren

                        readonly property var modelIndex: {
                            try {
                                return folderTree.index(row, column)
                            } catch (e) {
                                return null
                            }
                        }

                        readonly property bool isDirectory: modelIndex && modelIndex.valid ? fsWrapper.isDir(modelIndex) : false
                        readonly property string itemPath: modelIndex && modelIndex.valid ? fsWrapper.filePath(modelIndex) : ""
                        readonly property string itemName: modelIndex && modelIndex.valid ? fsWrapper.fileName(modelIndex) : ""

                        // Only show folders
                        visible: isDirectory
                        implicitHeight: isDirectory ? 35 : 0
                        implicitWidth: isDirectory ? folderTree.width : 0

                        Rectangle {
                            anchors.fill: parent
                            color: folderMA.containsMouse ? "#2a2a2a" : "transparent"

                            Row {
                                anchors.fill: parent
                                anchors.leftMargin: depth * 16 + 5
                                anchors.verticalCenter: parent.verticalCenter
                                spacing: 6

                                ToolButton {
                                    visible: folderDelegate.hasChildren
                                    text: expanded ? "▼" : "▶"
                                    width: 16
                                    height: 16
                                    anchors.verticalCenter: parent.verticalCenter
                                    onClicked: folderTree.toggleExpanded(row)
                                    background: Rectangle { color: "transparent" }
                                    contentItem: Text {
                                        text: parent.text
                                        color: "white"
                                        font.pixelSize: 10
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                }

                                Image {
                                    width: 18
                                    height: 18
                                    anchors.verticalCenter: parent.verticalCenter
                                    source: folderDelegate.itemPath ? "image://fileicon/" + folderDelegate.itemPath : ""
                                    cache: false
                                    asynchronous: true
                                }

                                Text {
                                    text: folderDelegate.itemName
                                    color: folderMA.containsMouse ? "#ffffff" : "#e0e0e0"
                                    font.pixelSize: 14
                                    elide: Text.ElideRight
                                    anchors.verticalCenter: parent.verticalCenter
                                    width: folderTree.width - (depth * 16) - 100
                                }
                            }

                            MouseArea {
                                id: folderMA
                                anchors.fill: parent
                                hoverEnabled: true
                                acceptedButtons: Qt.LeftButton | Qt.RightButton

                                onClicked: (mouse) => {
                                    if (mouse.button === Qt.LeftButton) {
                                        // Update file list to show files in this folder
                                        flModel.currentFolderPath = folderDelegate.itemPath
                                        project.setLinkText(folderDelegate.itemPath)
                                    } else if (mouse.button === Qt.RightButton) {
                                        contextMenu.currentFilePath = folderDelegate.itemPath
                                        contextMenu.currentIsDir = true
                                        contextMenu.currentRow = row
                                        contextMenu.popup()
                                    }
                                }

                                onDoubleClicked: {
                                    folderTree.toggleExpanded(row)
                                }
                            }
                        }
                    }
                }

                Column {
                    anchors.centerIn: parent
                    spacing: 10
                    visible: !folderTree.visible

                    Text {
                        text: "No directory selected"
                        color: "#666"
                        font.pixelSize: 16
                    }
                }
            }

            // Right side - Files only
            Rectangle {
                SplitView.minimumWidth: 200
                SplitView.fillWidth: true
                color: "#181818"

                Column {
                    anchors.fill: parent

                    // Header showing current folder
                    Rectangle {
                        width: parent.width
                        height: 35
                        color: "#252525"

                        Row {
                            anchors.fill: parent
                            anchors.leftMargin: 10
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 5

                            Text {
                                text: "📁"
                                font.pixelSize: 14
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            Text {
                                text: fileList.currentFolderPath ? fileList.currentFolderPath.split('/').pop() : "Select a folder"
                                color: "#e0e0e0"
                                font.pixelSize: 14
                                elide: Text.ElideMiddle
                                anchors.verticalCenter: parent.verticalCenter
                                width: parent.parent.width - 50
                            }
                        }
                    }

                    // File list
                    ListView {
                        id: fileList
                        width: parent.width
                        height: parent.height - 35
                        clip: true

                        property var currentFolderIndex: null
                        property string currentFolderPath: ""

                        model: flModel

                        // Update file list when folder changes


                        delegate: Rectangle {
                            width: fileList.width
                            height: 40
                            color: fileMA.containsMouse ? "#2a2a2a" : "transparent"

                            Row {
                                anchors.fill: parent
                                anchors.leftMargin: 15
                                anchors.verticalCenter: parent.verticalCenter
                                spacing: 10

                                Image {
                                    width: 20
                                    height: 20
                                    anchors.verticalCenter: parent.verticalCenter
                                    source: model.filePath ? "image://fileicon/" + model.filePath : ""
                                    cache: false
                                    asynchronous: true
                                }

                                Text {
                                    text: model.fileName
                                    color: fileMA.containsMouse ? "#ffffff" : "#e0e0e0"
                                    font.pixelSize: 14
                                    elide: Text.ElideRight
                                    anchors.verticalCenter: parent.verticalCenter
                                    width: fileList.width - 80
                                }
                            }

                            MouseArea {
                                id: fileMA
                                anchors.fill: parent
                                hoverEnabled: true
                                acceptedButtons: Qt.LeftButton | Qt.RightButton

                                onDoubleClicked: {
                                    if (model.filePath) {
                                        // Qt.openUrlExternally("file:///" + model.filePath)
                                        fsWrapper.openFile(model.filePath)
                                    }
                                }

                                onClicked: (mouse) => {
                                    if (mouse.button === Qt.RightButton) {
                                        contextMenu.currentFilePath = model.filePath
                                        contextMenu.currentIsDir = false
                                        contextMenu.currentRow = -1
                                        contextMenu.popup()
                                    }
                                }
                            }
                        }

                        // Empty state
                        // Text {
                        //     anchors.centerIn: parent
                        //     text: fileList.currentFolderPath ? "No files in this folder" : "Select a folder to view files"
                        //     color: "#666"
                        //     font.pixelSize: 14
                        //     visible: fileListModel.count === 0
                        // }

                        ScrollBar.vertical: ScrollBar {
                            active: true
                        }
                    }
                }
            }
        }

        // Context Menu
        Menu {
            id: contextMenu
            property string currentFilePath: ""
            property bool currentIsDir: false
            property int currentRow: -1

            MenuItem {
                text: contextMenu.currentIsDir ? "Open Folder" : "Open File"
                onTriggered: {
                    if (contextMenu.currentIsDir) {
                        fsWrapper.showInFileManager(contextMenu.currentFilePath)
                    } else if (contextMenu.currentFilePath) {
                        fsWrapper.openFile(contextMenu.currentFilePath)
                    }
                }
            }

            MenuItem {
                text: {
                    if (Qt.platform.os === "osx") return "Reveal in Finder"
                    if (Qt.platform.os === "windows") return "Show in Explorer"
                    return "Show in File Manager"  // Linux and others
                }
                onTriggered: {
                    if (contextMenu.currentFilePath) {
                        fsWrapper.showInFileManager(contextMenu.currentFilePath)
                    }
                }
            }

            MenuItem {
                text: "Copy Path"
                onTriggered: {
                    if (contextMenu.currentFilePath) {
                        fsWrapper.copyToClipboard(contextMenu.currentFilePath)
                    }
                }
            }
            
            MenuSeparator {
                visible: !contextMenu.currentIsDir && contextMenu.currentFilePath.toLowerCase().endsWith(".pdf")
            }
            
            MenuItem {
                visible: !contextMenu.currentIsDir && contextMenu.currentFilePath.toLowerCase().endsWith(".pdf")
                text: "🤖 AI Rename PDF"
                enabled: !aiConfig.isLoading
                onTriggered: {
                    if (contextMenu.currentFilePath) {
                        aiConfig.renamePdfWithAi(contextMenu.currentFilePath)
                    }
                }
            }

            MenuSeparator {
                visible: contextMenu.currentIsDir
            }

            MenuItem {
                visible: contextMenu.currentIsDir
                text: "Refresh Folder"
                onTriggered: refresh(fileList)
            }
        }

        // Loading overlay
        Rectangle {
            anchors.fill: parent
            color: "#181818"
            visible: !folderTree.visible

            Column {
                anchors.centerIn: parent
                spacing: 10

                BusyIndicator {
                    anchors.horizontalCenter: parent.horizontalCenter
                    running: fsWrapper.currentRootPath !== ""
                    visible: running
                }

                Text {
                    text: fsWrapper.currentRootPath !== "" ? "Loading directory..." : "No directory selected"
                    color: "#666"
                    font.pixelSize: 18
                }
            }
        }
    }

    function refresh(fileList){
        if (fileList.currentFolderIndex) {
            let temp = fileList.currentFolderIndex
            fileList.currentFolderIndex = null
            fileList.currentFolderIndex = temp
        }
    }
}

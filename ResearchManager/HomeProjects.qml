import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import Qt5Compat.GraphicalEffects

ScrollView {
        id: mainScrollView
        anchors.fill: parent
        clip: true

        // Horizontal layout for side-by-side groups
        ColumnLayout {
            id: groupsRow
            spacing: 20

            // Repeater creates vertical groups
            Repeater {
                id: groupRepeater
                model: homepageModel

                delegate: Rectangle {
                    id: groupDelegate
                    Layout.preferredWidth: homeProjects.width
                    Layout.preferredHeight: {
                        var itemCount = category ? category.length : 0;
                        var columns = Math.floor(homeProjects.width / 150); // Based on cellWidth
                        var rows = Math.ceil(itemCount / Math.max(1, columns));
                        // Calculate total height: header + separator + grid + margins
                        var headerHeight = 18 + 8; // Label height + spacing
                        var separatorHeight = 1 + 8; // Separator + spacing
                        var gridHeight = rows * 150; // rows * cellHeight
                        var margins = 24; // top + bottom margins (12 * 2)
                        return headerHeight + separatorHeight + gridHeight + margins;
                    }
                    // Remove Layout.fillHeight: true
                    color: "#153584e4"
                    radius: 8
                    border.color: "#ddd"
                    border.width: 1
                    property string randomColor: getRandomColor()

                    // Keep the bottom margin for spacing between groups
                    Layout.bottomMargin: 20


                    // Column for group content
                    ColumnLayout {
                        id: groupColumn
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 8

                        // Group title
                        Label {
                            text: name
                            font.bold: true
                            font.pixelSize: 18
                            color: "white"
                            Layout.fillWidth: true
                        }

                        // Separator line
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 1
                            color: "#ddd"
                        }

                        // GridView for folders
                        GridView {
                            id: folderGrid
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            // Responsive cell sizing
                            cellWidth: 150
                            cellHeight: 150
                            interactive: false  // Disable independent scrolling
                            model: getCategoryModel(category)



                            delegate: Rectangle {
                                width: folderGrid.cellWidth - 8
                                height: folderGrid.cellHeight - 8
                                radius: 4
                                color: "#282828"
                                border.color: "#e0e0e0"
                                border.width: 1



                                Column {
                                    anchors.centerIn: parent
                                    spacing: 8

                                    Image {
                                        source: "images/folder_generic.png"
                                        width: 56
                                        height: 56
                                        anchors.horizontalCenter: parent.horizontalCenter

                                        ColorOverlay {
                                            anchors.fill: parent
                                            source: parent
                                            color: groupDelegate.randomColor
                                            Behavior on color { ColorAnimation { duration: 200 } }
                                        }
                                    }

                                    Text {
                                        text: folderName
                                        width: 100
                                        color: "#3584e4"
                                        font.pixelSize: 16
                                        font.bold: true
                                        wrapMode: Text.Wrap
                                        horizontalAlignment: Text.AlignHCenter
                                        maximumLineCount: 2
                                        elide: Text.ElideRight
                                        anchors.horizontalCenter: parent.horizontalCenter
                                    }
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                                    onPressed: (mouse) => {
                                        mainScreen.projectName = folderName;
                                        if (mouse.button === Qt.RightButton) {
                                            // tableView.tableRowIndex = index
                                            delMenu.popup(mouse.scenePosition)
                                        }
                                        else{
                                            navigateProject(folderName)
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        MessageDialog {
            id: deleteDialog
            title: "Confirm Delete"
            text: "Are you sure you want to delete this project?"
            buttons: MessageDialog.Yes | MessageDialog.No
            onAccepted: {
                console.log("delete project ", mainScreen.projectName)
                homepageModel.deleteProject(mainScreen.projectName)
                tpModel.deleteProject(mainScreen.projectName)
            }
        }

        /* ---------------- Context Menu ---------------- */
        Menu {
            id: delMenu

            MenuItem {
                text: "Delete"
                onTriggered: {
                    deleteDialog.open()
                }
            }
        }
         /* ---------------- Helpers ---------------- */
        function getCategoryModel(data) {
            let model = Qt.createQmlObject('import QtQml 2.0; ListModel {}', parent);

            for (let i = 0; i < data.length; i++) {
                model.append({ folderName: data[i] });
                // console.log(data[i]);
            }

            return model;
        }


        // Function to generate random color
        function getRandomColor() {
            var colors = [
                "#E74C3C", // Red
                "#3498DB", // Blue
                "#2ECC71", // Green
                "#F39C12", // Orange
                "#9B59B6", // Purple
                "#1ABC9C", // Turquoise
                "#E91E63", // Pink
                "#FF5722", // Deep Orange
                "#00BCD4", // Cyan
                "#4CAF50", // Light Green
                "#FFC107", // Amber
                "#673AB7", // Deep Purple
                "#009688", // Teal
                "#FF9800", // Orange
                "#8BC34A"  // Lime
            ];
            return colors[Math.floor(Math.random() * colors.length)];
        }
}

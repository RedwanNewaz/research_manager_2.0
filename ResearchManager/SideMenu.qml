import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt5Compat.GraphicalEffects

Rectangle {
    id: root
    Loader {
        id: loader
        source: mainScreen.appCurrentPage // "Workspace.qml"
    }




    // Side Menu
    Rectangle {
        id: sideMenu
        width: 65
        height: parent.height
        color: "#181818"
         // color: "#282828"
        z: 1

        // Optional shadow effect
        layer.enabled: true
        layer.effect: DropShadow {
            horizontalOffset: 2
            verticalOffset: 0
            radius: 8
            samples: 16
            color: "#20000000"
        }



        Column {
            id: sideMenuColumn
            width: parent.width
            anchors.fill: parent
            topPadding: 40  // Top padding added
            spacing: 20

            // Home Button
            Item {
                width: parent.width
                height: 60

                Image {
                    id: imageHomePage
                    width: 40
                    height: 50
                    anchors.centerIn: parent
                    source: "images/home-svgrepo-com.svg"
                    sourceSize.height: 700
                    sourceSize.width: 700
                    fillMode: Image.PreserveAspectFit

                    ColorOverlay {
                        anchors.fill: parent
                        source: parent
                        color: homeMouseArea.containsMouse ? "#4A90E2" : "#808080"
                        Behavior on color { ColorAnimation { duration: 200 } }
                    }

                    MouseArea {
                        id: homeMouseArea
                        hoverEnabled: true
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            console.log("Homepage clicked")
                            mainScreen.appCurrentPage = "HomePage.qml";
                            sideMenu.color = "#181818"
                        }
                    }
                }
            }

            // Contact Button
            Item {
                width: parent.width
                height: 60

                Image {
                    id: imageContact
                    width: 40
                    height: 50
                    anchors.centerIn: parent
                    source: "images/contact-card-svgrepo-com.svg"
                    sourceSize.height: 700
                    sourceSize.width: 700
                    fillMode: Image.PreserveAspectFit

                    ColorOverlay {
                        anchors.fill: parent
                        source: parent
                        color: contactMouseArea.containsMouse ? "#4A90E2" : "#808080"
                        Behavior on color { ColorAnimation { duration: 200 } }
                    }

                    MouseArea {
                        id: contactMouseArea
                        hoverEnabled: true
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            console.log("Contact clicked")
                            mainScreen.appCurrentPage = "ContactList.qml";
                            sideMenu.color = "#181818"
                        }
                    }
                }
            }

            // AI Button
            Item {
                width: parent.width
                height: 60

                Image {
                    id: imageAi
                    width: 50
                    height: 50
                    anchors.centerIn: parent
                    source: "images/ai-svgrepo-com.svg"
                    sourceSize.height: 700
                    sourceSize.width: 700
                    fillMode: Image.PreserveAspectFit

                    ColorOverlay {
                        anchors.fill: parent
                        source: parent
                        color: aiMouseArea.containsMouse ? "#4A90E2" : "#808080"
                        Behavior on color { ColorAnimation { duration: 200 } }
                    }

                    MouseArea {
                        id: aiMouseArea
                        hoverEnabled: true
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            console.log("AiConfig clicked")
                            mainScreen.appCurrentPage = "AiConfig.qml";
                            sideMenu.color = "#181818"
                        }
                    }
                }
            }

            // Recent Button
            Item {
                width: parent.width
                height: 60

                Image {
                    id: imageRecent
                    width: 40
                    height: 50
                    anchors.centerIn: parent
                    source: "images/recent-svgrepo-com.svg"
                    sourceSize.height: 700
                    sourceSize.width: 700
                    fillMode: Image.PreserveAspectFit

                    ColorOverlay {
                        anchors.fill: parent
                        source: parent
                        color: recentMouseArea.containsMouse ? "#4A90E2" : "#808080"
                        Behavior on color { ColorAnimation { duration: 200 } }
                    }

                    MouseArea {
                        id: recentMouseArea
                        hoverEnabled: true
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            console.log("RecentProjects clicked")
                            mainScreen.appCurrentPage = "RecentProjects.qml";
                            sideMenu.color = "#282828"
                        }
                    }
                }
            }

            // Spacer to push settings to bottom
            Item {
                Layout.fillHeight: true
                width: parent.width
                height: parent.height - (60 * 5) - 40 - (20 * 4) - 60
            }

            // Settings Button
            Item {
                width: parent.width
                height: 60

                Image {
                    id: imageSettings
                    width: 40
                    height: 50
                    anchors.centerIn: parent
                    source: "images/settings-svgrepo-com.svg"
                    sourceSize.height: 700
                    sourceSize.width: 700
                    fillMode: Image.PreserveAspectFit

                    ColorOverlay {
                        anchors.fill: parent
                        source: parent
                        color: settingsMouseArea.containsMouse ? "#4A90E2" : "#808080"
                        Behavior on color { ColorAnimation { duration: 200 } }
                    }

                    MouseArea {
                        id: settingsMouseArea
                        hoverEnabled: true
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            console.log("Settings clicked")
                            mainScreen.appCurrentPage = "Settings.qml";
                            sideMenu.color = "#181818"
                        }
                    }
                }
            }
        }
    }
}

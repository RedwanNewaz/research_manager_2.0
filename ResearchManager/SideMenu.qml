import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt5Compat.GraphicalEffects

Rectangle {
    Loader {
        id: loader
        source: mainScreen.appCurrentPage // "Workspace.qml"
    }
    // color:  "#D3D3D3"
    Column{
        id: sideMenuGrid
        width: parent.width
        height: parent.height
        // anchors.left: parent.left
        // anchors.leftMargin: 0

        Image {
            id: imageHomePage
            x: 10
            width: 57
            height: 57
            // anchors.top: parent.top
            // anchors.topMargin: 44
            source: "images/home-svgrepo-com.svg"
            sourceSize.height: 700
            sourceSize.width: 700
            activeFocusOnTab: false
            clip: false
            fillMode: Image.PreserveAspectFit
            scale: 0.8

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
                    console.log("homepage clicked");
                    // loader.source = "HomePage.qml"
                    mainScreen.appCurrentPage = "HomePage.qml";
                }
            }
        }

        Image {
            id: imageContact
            width: 50
            height: 55
            anchors.right: parent.left
            anchors.top: imageHomePage.bottom
            anchors.rightMargin: -60
            anchors.topMargin: 33
            source: "images/contact-card-svgrepo-com.svg"
            scale: 0.9
            fillMode: Image.PreserveAspectFit
            clip: false

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
                    console.log("Contact clicked");
                    // loader.source = "ContactList.qml"
                    mainScreen.appCurrentPage = "ContactList.qml";
                }
            }
        }

        Image {
            id: imageAi
            x: 13
            width: 50
            height: 55
            anchors.top: imageContact.bottom
            anchors.topMargin: 20
            source: "images/ai-svgrepo-com.svg"
            scale: 1.2
            fillMode: Image.PreserveAspectFit
            clip: false

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
                    console.log("AiConfig clicked");
                    // loader.source = "AiConfig.qml"
                    mainScreen.appCurrentPage = "AiConfig.qml";
                }
            }
        }

        Image {
            id: imageRecent
            x: 13
            width: 50
            height: 55
            anchors.top: imageAi.bottom
            anchors.topMargin: 29
            source: "images/recent-svgrepo-com.svg"
            scale: 0.9
            fillMode: Image.PreserveAspectFit
            clip: false

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
                    console.log("RecentProjects clicked");
                    // loader.source = "RecentProjects.qml"
                    mainScreen.appCurrentPage = "RecentProjects.qml";
                }
            }
        }

        Image {
            id: imageSettings
            width: 50
            height: 55
            anchors.left: parent.left
            // anchors.bottom: parent.bottom
            anchors.top: imageRecent.bottom
            anchors.leftMargin: 13
            anchors.topMargin: 40
            source: "images/settings-svgrepo-com.svg"
            scale: 0.85
            fillMode: Image.PreserveAspectFit
            clip: false

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
                    console.log("Settings clicked");
                    // loader.source = "Settings.qml"
                    mainScreen.appCurrentPage = "Settings.qml";

                }
            }

        }
    }
}

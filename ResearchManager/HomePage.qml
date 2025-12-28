import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts

Rectangle {
    id: rectangle
    width: Screen.width
    height: Screen.height -70
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

    function navigateProject(folderName)
    {
        if(!homepageModel.isValidProject(folderName))
        {
            console.log("[HomePage] not a valid foldername = ", folderName)
            return;
        }

        mainScreen.appCurrentPage = "RecentProjects.qml";
        console.log("[WsName]: " + tpModel.WsPath)
        project.projectName = folderName
        project.setRootDir(tpModel.WsPath + "/" + folderName)
        project.setLinkText(tpModel.WsPath + "/" + folderName)
    }

    ColumnLayout {
        id: rootLayout
        anchors.fill: parent
        anchors.margins: 95
        spacing: 20

        Row {
            id: row
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            Button {
                id: buttonLeft
                anchors.verticalCenter: parent.verticalCenter
                icon.color: "#3584e4"
                flat: false
                highlighted: false
                icon.height: 50
                icon.width: 50
                display: AbstractButton.IconOnly
                icon.source: "images/go_previous.png"
                // This makes the background transparent
                background: Rectangle {
                    color: "transparent"
                    border.color: "transparent"
                }
                onClicked: {
                    tpModel.WsIndex = tpModel.WsIndex - 1
                }
            }
            Label {
                id: label
                color: "#3584e4"
                text: tpModel.WsName ? qsTr(tpModel.WsName) : ""
                anchors.verticalCenter: parent.verticalCenter
                horizontalAlignment: Text.AlignLeft
                font.styleName: "Bold Italic"
                font.family: "Arial"
                font.pointSize: 30
            }
            Image {
                id: image
                width: 100
                height: 80
                source: tpModel.WsIcon ? tpModel.WsIcon : ""
                fillMode: Image.PreserveAspectFit
            }

            Button {
                id: buttonRight
                opacity: 1
                anchors.verticalCenter: parent.verticalCenter
                clip: false
                icon.color: "#3584e4"
                flat: false
                highlighted: false
                icon.height: 50
                icon.width: 50
                display: AbstractButton.IconOnly
                icon.source: "images/go_next.png"
                // This makes the background transparent
                background: Rectangle {
                    color: "transparent"
                    border.color: "transparent"
                }
                onClicked: {

                    tpModel.WsIndex = tpModel.WsIndex + 1
                }
            }
        }

        // ----------- Label -------------
        Label {
            text: "Research Manager"
            color: "#3584e4"
            font.pointSize: 50
            font.bold: true
            font.italic: true
            horizontalAlignment: Text.AlignHCenter
            Layout.alignment: Qt.AlignHCenter
        }

        // ----------- Project Name Row -------------
        RowLayout {
            id: createProject
            Layout.fillWidth: true
            Layout.preferredHeight: 200
            spacing: 20

            ComboBox {
                id: combo
                model: tpModel
                textRole: "name"        // what user sees
                valueRole: "id"         // internal value
                font.pixelSize: 20
                font.wordSpacing: 0.1
                font.weight: Font.Bold
                font.family: "Arial"
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                Layout.preferredWidth: 150
                Layout.preferredHeight: 50

                // This makes the background transparent
                background: Rectangle {
                    radius: 18
                    // color: "transparent"
                    // border.color: "transparent"
                }


                // onActivated: (index) => {
                //     console.log("Selected:",
                //                 currentText,
                //                 currentValue);
                //     tpModel.selectTemplate(currentText);
                // }
            }

            // Text Input
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 120

                TextInput {
                    id: createProjectText
                    anchors.fill: parent
                    font.pixelSize: 26
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.styleName: "Italic"
                    property string placeholderText: "Project Name here..."
                    color: "white"
                    Text {
                        text: createProjectText.placeholderText
                        font.styleName: "Italic"
                        font.pointSize: 26
                        color: "#aaa"
                        anchors.centerIn: parent
                        visible: !createProjectText.text && !createProjectText.activeFocus
                    }
                }
            }

            // Create Button
            RoundButton {
                id: createButton
                text: "Create"
                font.pixelSize: 24
                Layout.preferredWidth: 150
                Layout.preferredHeight: 100
                onClicked: {
                    if(createProjectText.text !== "")
                    {
                        tpModel.createProject(createProjectText.text, combo.currentText)
                        mainScreen.appCurrentPage = "RecentProjects.qml";
                        project.projectName = createProjectText.text;
                    }
                }
            }
        }

        // ----------- Scrollable Area -------------
        Rectangle {
            id: rectArea
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#153584e4"  // Fixed format


            HomeProjects {
                id: homeProjects
                anchors.fill: parent
            }
        }
    }

    Rectangle {
        id: searchRectangle
        x: 1543
        y: 43
        width: 282
        height: 74
        color: "transparent"
        border.color: "#312f2f"
        border.width: 2
        anchors.right: rootLayout.right
        anchors.rightMargin: 0


        TextInput {
            id: searchProject
            y: 11
            width: 241
            height: 52
            color: "#f7f2f2"

            // text: qsTr("Text Input")
            font.pixelSize: 18
            verticalAlignment: Text.AlignVCenter
            anchors.horizontalCenterOffset: 0
            anchors.horizontalCenter: parent.horizontalCenter
            font.styleName: "Italic"

            horizontalAlignment: Text.AlignHCenter
            property string placeholderText: "Search here..."
            selectByMouse: true
            
            // Trigger search on text change
            onTextChanged: {
                if (text.length > 0) {
                    homepageModel.searchProjects(text)
                    suggestionsPopup.open()
                } else {
                    homepageModel.clearSearch()
                    suggestionsPopup.close()
                }
            }
            
            Text {
                text: searchProject.placeholderText
                font.styleName: "Italic"
                font.pointSize: 16
                color: "#d6ffffff"
                visible: !searchProject.text && !searchProject.activeFocus
                anchors.centerIn: parent
            }
            onAccepted: {
                console.log("suggestion accepted")
                suggestionsPopup.close()
                navigateProject(searchProject.text)
            }
        }

        Image {
            id: searchIcon
            x: -61
            width: 58
            height: 48
            anchors.verticalCenter: searchProject.verticalCenter
            source: "images/search.png"
            anchors.verticalCenterOffset: 0
            fillMode: Image.PreserveAspectFit
        }


        
        // Autocomplete suggestions popup
        Popup {
            id: suggestionsPopup
            x: 0
            y: searchRectangle.height
            width: searchRectangle.width
            height: Math.min(suggestionsList.contentHeight + 10, 300)
            padding: 5
            
            background: Rectangle {
                color: "#424242"
                border.color: "#3584e4"
                border.width: 2
                radius: 5
            }
            
            ListView {
                id: suggestionsList
                anchors.fill: parent
                clip: true
                model: homepageModel.searchSuggestions
                
                delegate: ItemDelegate {
                    width: suggestionsList.width
                    height: 40
                    
                    background: Rectangle {
                        color: parent.hovered ? "#3584e4" : "transparent"
                        radius: 3
                    }
                    
                    Text {
                        text: modelData
                        color: "#ffffff"
                        font.pixelSize: 16
                        anchors.left: parent.left
                        anchors.leftMargin: 10
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    
                    onClicked: {
                        searchProject.text = modelData
                        suggestionsPopup.close()
                        // Optionally navigate to the selected project
                        // project.projectName = modelData
                        // mainScreen.appCurrentPage = "RecentProjects.qml"
                        // navigateProject(modelData)
                    }
                }
                
                ScrollBar.vertical: ScrollBar {
                    policy: ScrollBar.AsNeeded
                }
            }
        }
    }

}

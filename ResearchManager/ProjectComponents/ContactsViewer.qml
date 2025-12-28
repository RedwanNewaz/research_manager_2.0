import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import Qt5Compat.GraphicalEffects



Rectangle{
    id: contactsList
    // color: 'navy'
    color: parent.color
    anchors.top: notificationRect.bottom
    width: deadlineViewer.width
    height: deadlineViewer.height
    property string name: ""
    property string affiliation: ""
    property string website: ""
    property string phone: ""
    property string email: ""
    property string zoom: ""
    property string photo: ""

    
    StackView {
        id: stackContactView
        anchors.fill: parent
        initialItem: Qt.resolvedUrl("ContactSettings.qml") // Start with Page1
        
    }

    // opacity: 0.5
    GridView {
        id: contactViews
        height: 3 * parent.height / 4
        width: parent.width
        cellWidth: 150
        cellHeight: 150
        model: pcModel

        delegate: Item {
            width: 120
            height: 120

            Rectangle {
                id: imageContainer
                width: 100
                height: 100
                color: "transparent"
                anchors.centerIn: parent

                Image {
                    id: sourceImage
                    source: photo
                    anchors.fill: parent
                    fillMode: Image.PreserveAspectCrop
                    visible: false // Hide the original image
                }

                Rectangle {
                    id: mask
                    anchors.fill: parent
                    radius: width / 2
                    visible: false // Hide the mask
                }

                OpacityMask {
                    anchors.fill: sourceImage
                    source: sourceImage
                    maskSource: mask
                }
                Menu {
                    id: contactMenu
                    width: parent.width
                    // Customize the background to be transparent
                    background: Rectangle {
                        opacity: 0.5
                    }
                    MenuItem {
                        text: "Delete"
                        onTriggered: pcModel.delete_item(name)
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                    
                    ToolTip {
                        text: name
                        visible: parent.containsMouse
                        delay: 500
                    }
                    onClicked: function(mouse) {
                        if(mouse.button === Qt.RightButton)
                            contactMenu.open()  
                    }
                    onDoubleClicked: {
                        console.log("onDoubleClicked show contact details")
                        contactsList.name = name
                        contactsList.affiliation = affiliation
                        contactsList.website = website
                        contactsList.phone = phone
                        contactsList.email = email
                        contactsList.zoom = zoom
                        contactsList.photo = photo
                        stackContactView.push(Qt.resolvedUrl("ContactSettings.qml"))
                    } 
                }
            }
        }
    }

    // ###########################ADD Contacts #####################################
    Rectangle {
        height: parent.height / 4
        width: parent.width
        color: parent.color
        anchors.top: contactViews.bottom
        RoundButton {
            id: buttonAddContact
            x: parent.width - buttonAddContact.width
            // y: 46
            width: 50
            height: 50
            text: "+"
            display: AbstractButton.IconOnly
            icon.source: "qrc:/icon/Resources/contact.svg"
            onClicked: addContact(contactTextInput.text, pcModel)
        }
        TextEdit {
            id: contactTextInput
            anchors.right: buttonAddContact.left
            width: parent.width - buttonAddContact.width
            height: buttonAddContact.height
            color: "#d6ffffff"
            // text: qsTr("TypeHere")
            font.pixelSize: 16
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.styleName: "Italic"
            property string placeholderText: "Add contact here..."
            selectByMouse: true
            Text {
                text: contactTextInput.placeholderText
                font.styleName: "Italic"
                font.pointSize: 16
                color: "#d6ffffff"
                visible: !contactTextInput.text && !contactTextInput.activeFocus
                anchors.centerIn: parent
            }
            onTextChanged: {
                // Filter suggestions based on current text
                suggestionsModel.update(contactModel, text)
            }
        }
        // add suggestion from database 
        ListView {
            id: suggestionsView
            width: contactTextInput.width
            anchors.top: contactTextInput.bottom
            height: 200
            visible: count > 0
            model: suggestionsModel
            opacity: 0.5

            delegate: ItemDelegate {
                width: parent.width
                text: model.text
                highlighted: ListView.isCurrentItem

                onClicked: {
                    contactTextInput.text = model.text
                    suggestionsModel.clear()
                }
            }
        }

        // backend suggestion model
        ListModel {
            id: suggestionsModel

            function update(model, text) {
                clear()
                // This is where you would typically query your data source
                // For this example, we'll use a simple predefined list
                var allSuggestions = model.getAllItems()
                for (var i = 0; i < allSuggestions.length; i++) {
                    if (allSuggestions[i].startsWith(text)) {
                        append({text: allSuggestions[i]})
                    }
                }
             
            }
        }

    }

    function addContact(text, model){
        console.log("adding contact",  text)
        model.addContact(text)
        contactTextInput.text = ""
    }
}
import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt5Compat.GraphicalEffects


Item{
    id: root
    
    // Signal emitted when a collaborator is selected for editing
    signal openTagViewer(string collaboratorName)
    
    property string name: ""
    property string affiliation: ""
    property string website: ""
    property string phone: ""
    property string email: ""
    property string zoom: ""
    property string photo: ""

    ColumnLayout {
        anchors.fill: parent
        spacing: 50

        // ###########################ADD Contacts #####################################
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 60
            color: "transparent"
            
            RowLayout {
                anchors.fill: parent
                spacing: 5
                
                TextEdit {
                    id: contactTextInput
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "#d6ffffff"
                    font.pixelSize: 16
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.styleName: "Italic"
                    property string placeholderText: "Add collaborator here..."
                    property bool isProgrammaticChange: false
                    selectByMouse: true
                    wrapMode: Text.WordWrap
                    
                    Keys.onTabPressed: {
                        if (suggestionsModel.count > 0) {
                            var suggestion = suggestionsModel.get(0)
                            if (suggestion && suggestion.name !== undefined && suggestion.name !== "") {
                                isProgrammaticChange = true
                                text = suggestion.name
                                selectedContact = suggestion.name
                                isProgrammaticChange = false
                            }
                            suggestionsModel.clear()
                            event.accepted = true
                        }
                    }
                    
                    property string selectedContact: ""
                    
                    Text {
                        text: contactTextInput.placeholderText
                        font.styleName: "Italic"
                        font.pointSize: 16
                        color: "#d6ffffff"
                        visible: !contactTextInput.text && !contactTextInput.activeFocus
                        anchors.centerIn: parent
                    }
                    onTextChanged: {
                        // Skip if this is a programmatic change (e.g., clearing after adding)
                        if (isProgrammaticChange) return
                        
                        // Filter suggestions based on current text - skip if clearing
                        if (text && text.length > 0) {
                            suggestionsModel.update(pcModel, text)
                            selectedContact = text
                        } else {
                            suggestionsModel.clear()
                            selectedContact = ""
                        }
                    }
                    Keys.onEnterPressed: {
                        if (text && text.length > 0) {
                            addCollaborator(text)
                        }
                        event.accepted = true
                    }
                    Keys.onReturnPressed: {
                        if (text && text.length > 0) {
                            addCollaborator(text)
                        }
                        event.accepted = true
                    }
                }
                
            }
            
            // add suggestion from database
            ListView {
                id: suggestionsView
                width: parent.width
                anchors.top: parent.bottom
                height: 200
                visible: count > 0
                model: suggestionsModel
                opacity: 0.5
                z: 1000

                delegate: ItemDelegate {
                    width: suggestionsView.width
                    text: model.name
                    highlighted: ListView.isCurrentItem

                    onClicked: {
                        contactTextInput.text = model.name
                        contactTextInput.selectedContact = model.name
                        suggestionsModel.clear()
                    }
                }
            }

            // backend suggestion model
            ListModel {
                id: suggestionsModel

                function update(model, text) {
                    clear()
                    if (!text) return
                    
                    // Get all suggestions and filter case-insensitively
                    var allContacts = model.getAllItems()
                    var lowerText = text.toLowerCase()
                    
                    for (var i = 0; i < allContacts.length; i++) {
                        var contactName = allContacts[i]
                        if (contactName.toLowerCase().startsWith(lowerText)) {
                            // Just append the name for now
                            append({
                                "name": contactName,
                                "photo": "",
                                "email": "",
                                "affiliation": "",
                                "website": "",
                                "phone": "",
                                "zoom": ""
                            })
                        }
                    }
                }
            }
        }

        GridView {
            id: contactViews
            Layout.fillWidth: true
            Layout.fillHeight: true
            cellWidth: 100
            cellHeight: 100
            model: colModel

            delegate: Item {
                width: 100
                height: 100
                
                property int itemIndex: index

                Rectangle {
                    id: imageContainer
                    width: 90
                    height: 90
                    color: "transparent"
                    anchors.centerIn: parent

                    // Function to get initials from name
                    function getInitials(fullName) {
                        if (!fullName) return "?"
                        var parts = fullName.trim().split(/\s+/)
                        if (parts.length === 1) {
                            return parts[0].substring(0, 2).toUpperCase()
                        }
                        return (parts[0].charAt(0) + parts[parts.length - 1].charAt(0)).toUpperCase()
                    }

                    // Show image if photo exists, otherwise show initials
                    Image {
                        id: sourceImage
                        source: photo || ""
                        anchors.fill: parent
                        fillMode: Image.PreserveAspectCrop
                        visible: false
                    }

                    Rectangle {
                        id: mask
                        anchors.fill: parent
                        radius: width / 2
                        visible: false
                    }

                    OpacityMask {
                        anchors.fill: sourceImage
                        source: sourceImage
                        maskSource: mask
                        visible: photo && sourceImage.status === Image.Ready
                    }

                    // Fallback initials display
                    Rectangle {
                        anchors.fill: parent
                        radius: width / 2
                        color: "#4A90E2"
                        visible: !photo || sourceImage.status !== Image.Ready
                        
                        Text {
                            anchors.centerIn: parent
                            text: imageContainer.getInitials(name)
                            font.pixelSize: 36
                            font.bold: true
                            color: "white"
                        }
                    }

                    Menu {
                        id: contactMenu
                        width: parent.width
                        background: Rectangle {
                            opacity: 0.5
                        }
                        MenuItem {
                            text: "Delete"
                            onTriggered: {
                                console.log("[CollaboratorView] Deleting collaborator at index:", itemIndex, "name:", name)
                                colModel.deleteCollaborator(itemIndex)
                            }
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
                            // Find the root Item by traversing up the hierarchy
                            msgModel.currentName = name;
                            tagViewer.open()

                        }
                    }
                }
            }
        }
    }

    function addCollaborator(contactName) {
        console.log("[CollaboratorView] Adding collaborator:", contactName)
        if (!contactName || contactName.length === 0) {
            console.log("[CollaboratorView] Empty contact name, skipping")
            return
        }
        // Add to colModel (collaborator model)
        colModel.addCollaborator(contactName)
        // Clear the input and reset state
        Qt.callLater(function() {
            contactTextInput.isProgrammaticChange = true
            contactTextInput.text = ""
            contactTextInput.selectedContact = ""
            suggestionsModel.clear()
            contactTextInput.isProgrammaticChange = false
            contactTextInput.forceActiveFocus()
        })
    }
    
    function addContact(text, model){
        console.log("adding contact",  text)
        model.addContact(text)
        contactTextInput.text = ""
    }
}

import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import Qt.labs.folderlistmodel 2.15
import QtQuick.Layouts 1.12
import Qt5Compat.GraphicalEffects

Rectangle{
    id: profileView
    width: (Screen.width) / 2
    height: (Screen.height -70) / 2
    anchors.centerIn: parent
    color: "#282828"


    property string currentName: ""
    property string currentAffiliation: ""
    property string currentWebsite: ""
    property string currentPhone: ""
    property string currentEmail: ""
    property string currentZoom: ""
    property string currentPhoto: ""

    ColumnLayout {
        anchors.fill: parent
        width: parent.width
        height:  parent.height
        anchors.margins: 0
        spacing: 10


        RowLayout {
            id: contactSelector
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 10
            height: 40
            spacing: 10

            Button {
                text: "◄ Previous"
                icon.color: "#e309f1"
                icon.source: "images/go_previous.png"
                display: AbstractButton.IconOnly
                enabled: currentContactIndex > 0 && !isCreatingNew
                onClicked: {
                    if (currentContactIndex > 0) {
                        currentContactIndex--
                        loadCurrentContact()
                    }
                }
            }

            ComboBox {
                id: contactComboBox
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                font.pointSize: 16
                Layout.fillWidth: true
                model: pcModel
                textRole: "name"
                currentIndex: currentContactIndex
                enabled: !isCreatingNew
                onCurrentIndexChanged: {
                    if (currentIndex !== currentContactIndex) {
                        currentContactIndex = currentIndex
                        loadCurrentContact()
                    }
                }
            }

            Button {
                text: "Next ►"
                display: AbstractButton.IconOnly
                icon.color: "#e309f1"
                icon.source: "images/go_next.png"
                enabled: currentContactIndex < pcModel.rowCount() - 1 && !isCreatingNew
                onClicked: {
                    if (currentContactIndex < pcModel.rowCount() - 1) {
                        currentContactIndex++
                        loadCurrentContact()
                    }
                }
            }

            Button {
                text: "+ New"
                icon.color: "#e412ddee"
                display: AbstractButton.IconOnly
                icon.source: "images/contact-card-svgrepo-com.svg"
                highlighted: true
                onClicked: {
                    createNewContact()
                }
            }
        }






        GridLayout {
            columns: 2
            columnSpacing: 400
            rowSpacing: 10
            Layout.fillWidth: true
            anchors.centerIn: parent

            Label{}
            Rectangle {
                width: 100
                height: 100
                Layout.alignment: Qt.AlignRight | Qt.AlignTop


                color: "transparent"
                // anchors.centerIn: parent

                Image {
                    id: sourceImage
                    source: currentPhoto
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
            }

            Label { text: "Name:" ; font.pointSize: 18; font.bold: true; color: "white"}
            Label { text: currentName ;font.pointSize: 16; color: "white" }

            Label { text: "Affiliation:" ; font.pointSize: 18; font.bold: true; color: "white" }
            Label { text: currentAffiliation ;font.pointSize: 16; color: "white" }

            Label { text: "Website:" ; font.pointSize: 18; font.bold: true; color: "white" }
            Label {
                text: "Website Link"
                font.pointSize: 16
                color: "blue"
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: Qt.openUrlExternally(currentWebsite)
                }
            }

            Label { text: "Phone:" ; font.pointSize: 18; font.bold: true; color: "white" }
            Label { text: currentPhone ; font.pointSize: 16; color: "white" }

            Label { text: "Email:" ; font.pointSize: 18; font.bold: true; color: "white" }
            Label {
                color: "red"
                text: currentEmail
                font.pointSize: 16
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: Qt.openUrlExternally("mailto:" + currentEmail)
                    }
                }

            Label { text: "Video:" ; font.pointSize: 18; font.bold: true; color: "white" }
            Label {
                text: currentZoom
                font.pointSize: 16
                color: "blue"
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: Qt.openUrlExternally(currentZoom)
                }
            }
        }

        Item { Layout.fillHeight: true } // Spacer
    }

    function loadFromBackend(index) {
        if (index >= 0) {
            var contact = pcModel.getContact(index)
            currentName = contact.name || ""
            currentAffiliation = contact.affiliation || ""
            currentWebsite = contact.website || ""
            currentPhone = contact.phone || ""
            currentEmail = contact.email || ""
            currentZoom = contact.zoom || ""
            currentPhoto = contact.photo || ""
        }
    }

    function loadCurrentContact() {
        if (pcModel.rowCount() > 0 && currentContactIndex >= 0 && currentContactIndex < pcModel.rowCount()) {
            profileView.loadFromBackend(currentContactIndex)
            if (!profileView.visible) {
                profileEdit.loadFromBackend(currentContactIndex)
            }
        }
    }

}

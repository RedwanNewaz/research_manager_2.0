import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12
import QtQuick.Dialogs

Item {
    id: root
    width: Screen.width
    height: Screen.height - 70

    property int currentContactIndex: 0

    Component.onCompleted: {
        pcModel.load_database()
        if (pcModel.rowCount() > 0) {
            currentContactIndex = 0
        }
    }

    function currentName() {
        if (currentContactIndex >= 0 && currentContactIndex < pcModel.rowCount()) {
            var c = pcModel.getContact(currentContactIndex)
            return c.name
        }
        return ""
    }

    function addNewContact() {
        pcModel.addOrUpdateContact({
            name: "New Contact",
            affiliation: "",
            website: "",
            phone: "",
            email: "",
            zoom: "",
            photo: ""
        })
        currentContactIndex = pcModel.rowCount() - 1
        contactComboBox.currentIndex = currentContactIndex
    }

    function deleteSelected() {
        var name = currentName()
        if (name) {
            pcModel.deleteContact(name)
            if (pcModel.rowCount() > 0) {
                currentContactIndex = Math.min(currentContactIndex, pcModel.rowCount() - 1)
                contactComboBox.currentIndex = currentContactIndex
            } else {
                currentContactIndex = -1
            }
        }
    }

    Rectangle {
        anchors.fill: parent
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



        Rectangle {
            id: contactPanel
            width: Math.min(parent.width * 0.9, 1100)
            height: Math.min(parent.height * 0.9, 750)
            anchors.centerIn: parent
            color: "#282828"


            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 12

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    Button {
                        text: ""
                        icon.source: "images/go_previous.png"
                        display: AbstractButton.IconOnly
                        enabled: currentContactIndex > 0 && pcModel.rowCount() > 0
                        onClicked: {
                            currentContactIndex--
                            contactComboBox.currentIndex = currentContactIndex
                        }
                    }

                    ComboBox {
                        id: contactComboBox
                        Layout.fillWidth: true
                        model: pcModel
                        textRole: "name"
                        currentIndex: currentContactIndex
                        onCurrentIndexChanged: {
                            if (currentIndex >= 0) {
                                currentContactIndex = currentIndex
                            }
                        }
                    }

                    Button {
                        text: ""
                        icon.color: "#787878"
                        icon.source: "images/go_next.png"
                        display: AbstractButton.IconOnly
                        enabled: currentContactIndex < pcModel.rowCount() - 1 && pcModel.rowCount() > 0
                        onClicked: {
                            currentContactIndex++
                            contactComboBox.currentIndex = currentContactIndex
                        }
                    }

                    Button {
                        text: "+ New"
                        icon.color: "#0ec2e2"
                        display: AbstractButton.IconOnly
                        icon.source: "images/contact-card-svgrepo-com.svg"
                        highlighted: true
                        onClicked: addNewContact()
                    }

                    // Button {
                    //     text: "Delete"
                    //     enabled: pcModel.rowCount() > 0
                    //     onClicked: deleteSelected()
                    // }

                    Button {
                        id: deleteTemplateBtn
                        text: "Delete"
                        Layout.preferredWidth: 120
                        Layout.preferredHeight: 36

                        background: Rectangle {
                            color: !deleteTemplateBtn.enabled ? "#555" :
                                   (deleteTemplateBtn.pressed ? "#a93226" : (deleteTemplateBtn.hovered ? "#c0392b" : "#e74c3c"))
                            radius: 3
                        }

                        contentItem: Text {
                            text: deleteTemplateBtn.text
                            color: deleteTemplateBtn.enabled ? "white" : "#888"
                            font.bold: true
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        enabled: pcModel.rowCount() > 0
                        onClicked: deleteSelected()
                    }
                }

                HorizontalHeaderView {
                    id: header
                    Layout.fillWidth: true
                    height: 36
                    syncView: tableView
                    clip: true

                    delegate: Rectangle {
                        implicitWidth: tableView.columnWidthProvider(index)
                        implicitHeight: 36
                        color: "#3a3a3a"
                        border.color: "#555"
                        Text {
                            anchors.centerIn: parent
                            text: display
                            color: "white"
                            font.bold: true
                        }
                    }
                }

                TableView {
                    id: tableView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    model: pcModel
                    clip: true
                    rowSpacing: 2
                    columnSpacing: 2
                    // current selection driven by ComboBox/currentContactIndex

                    columnWidthProvider: function(col) {
                        switch (col) {
                        case 0: return 160
                        case 1: return 180
                        case 2: return 200
                        case 3: return 140
                        case 4: return 220
                        case 5: return 160
                        case 6: return 200
                        default: return 150
                        }
                    }

                    delegate: Rectangle {
                        implicitWidth: tableView.columnWidthProvider(column)
                        implicitHeight: 54
                        color: column === 0 ? "#303030" : "#3a3a3a"
                        border.color: "#555"
                        property bool isEditing: false

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 6

                            Text {
                                visible: !isEditing || column === 0
                                Layout.fillWidth: true
                                text: display
                                color: "white"
                                wrapMode: Text.Wrap
                                elide: Text.ElideRight
                                verticalAlignment: Text.AlignVCenter
                                horizontalAlignment: Text.AlignLeft
                                font.bold: column === 0
                            }

                            Image {
                                visible: column === 6 && !isEditing && display !== ""
                                source: column === 6 ? display : ""
                                fillMode: Image.PreserveAspectFit
                                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                                Layout.fillHeight: true
                                Layout.preferredWidth: 140
                            }

                            TextField {
                                id: editor
                                visible: isEditing && column !== 6
                                Layout.fillWidth: true
                                text: display
                                verticalAlignment: Text.AlignVCenter
                                wrapMode: TextInput.Wrap
                                
                                function saveData() {
                                    if (text !== display) {
                                        console.log("Saving:", row, column, text)
                                        pcModel.setData(pcModel.index(row, column), text, Qt.EditRole)
                                    }
                                }
                                
                                onEditingFinished: {
                                    saveData()
                                    isEditing = false
                                }
                                
                                onActiveFocusChanged: {
                                    if (!activeFocus) {
                                        saveData()
                                        isEditing = false
                                    }
                                }
                                
                                Component.onCompleted: if (visible) { forceActiveFocus(); selectAll(); }
                            }

                            Button {
                                visible: isEditing && column === 6
                                text: "Browse..."
                                onClicked: {
                                    photoDialog.row = row
                                    photoDialog.open()
                                }
                            }
                        }

                        TapHandler {
                            acceptedButtons: Qt.LeftButton
                            onDoubleTapped: {
                                isEditing = true
                                currentContactIndex = row
                                contactComboBox.currentIndex = row
                            }
                        }
                    }
                }
            }
        }
    }

    FileDialog {
        id: photoDialog
        property int row: -1
        title: "Select Contact Photo"
        nameFilters: ["Image files (*.jpg *.jpeg *.png *.gif *.bmp)"]
        onAccepted: {
            if (row >= 0) {
                pcModel.setData(pcModel.index(row, 6), selectedFile.toString(), Qt.EditRole)
            }
        }
    }
}
